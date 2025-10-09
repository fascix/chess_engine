#include "uci.h"
#include "search.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile bool stop_search_flag = false; // Variable pour arrêter la recherche

// Boucle principale UCI
void uci_loop() {
  char line[4096];
  Board board;

  // Initialiser le board en position initiale
  board_from_fen(&board,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  while (1) {
    memset(line, 0, sizeof(line));
    if (!fgets(line, sizeof(line), stdin))
      break;
    // Supprimer le \n
    line[strcspn(line, "\n")] = 0;

    if (strlen(line) == 0)
      continue;

    parse_uci_command(line, &board);
  }
}

// Parser de commandes
void parse_uci_command(char *line, Board *board) {

  char *command = strtok(line, " ");

  if (strcmp(command, "uci") == 0) {
    handle_uci();
  } else if (strcmp(command, "isready") == 0) {
    handle_isready();
  } else if (strcmp(command, "position") == 0) {
    char *params = strtok(NULL, ""); // Récupérer le reste
    handle_position(board, params);
  } else if (strcmp(command, "go") == 0) {
    char *params = strtok(NULL, ""); // Récupérer le reste
    handle_go(board, params);
  } else if (strcmp(command, "stop") == 0) {
    handle_stop();
  } else if (strcmp(command, "quit") == 0) {
    handle_quit();
  }
}

// Gestionnaire commande "uci"
void handle_uci() {
  printf("id name ChessEngine v1.0\n");
  fflush(stdout);
  printf("id author Fascix\n");
  fflush(stdout);
  printf("uciok\n");
  fflush(stdout); // 🔹 essentiel pour UCI
}

// Gestionnaire commande "isready"
void handle_isready() {
  printf("readyok\n");
  fflush(stdout);
}

// Fonctions utilitaires pour position
void setup_startpos(Board *board) {
  board_from_fen(board,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  /* S'assurer que l'état éphémère est propre */
  board->to_move = WHITE;
  board->en_passant = -1;
  board->halfmove_clock = 0;
  board->move_number = 1;
}

void setup_from_fen(Board *board, char *params) {
  // Chercher "fen" et extraire la chaîne FEN
  char *fen_start = strstr(params, "fen");
  if (!fen_start)
    return;

  fen_start += 4; // Passer "fen "
  char *moves_start = strstr(fen_start, " moves");

  if (moves_start) {
    // FEN se termine avant " moves"
    size_t fen_len = moves_start - fen_start;
    char fen_string[256];
    strncpy(fen_string, fen_start, fen_len);
    fen_string[fen_len] = '\0';
    board_from_fen(board, fen_string);
  } else {
    // Pas de moves, prendre toute la fin
    board_from_fen(board, fen_start);
  }
}

// Gestionnaire commande "position"
void handle_position(Board *board, char *params) {

  if (strstr(params, "startpos")) {
    setup_startpos(board);
  } else if (strstr(params, "fen")) {
    setup_from_fen(board, params);
  }

  // Appliquer les coups si "moves" est présent
  char *moves_start = strstr(params, " moves");
  if (moves_start) {
    moves_start += 7; // Passer " moves "
    apply_uci_moves(board, moves_start);
  }
}

// Parser un coup UCI (ex: "e2e4" -> Move)
Move parse_uci_move(const char *uci_str) {
  Move move;
  memset(&move, 0, sizeof(move));

  if (!uci_str)
    return move;
  size_t len = strlen(uci_str);
  if (len < 4)
    return move;

  int file_from = uci_str[0] - 'a';
  int rank_from = uci_str[1] - '1';
  int file_to = uci_str[2] - 'a';
  int rank_to = uci_str[3] - '1';

  move.from = rank_from * 8 + file_from;
  move.to = rank_to * 8 + file_to;
  move.type = MOVE_NORMAL;
  move.promotion = EMPTY;

  /* Promotion possible si un 5ème caractère est fourni */
  if (len >= 5) {
    move.type = MOVE_PROMOTION;
    char p = uci_str[4];
    switch (p) {
    case 'q':
    case 'Q':
      move.promotion = QUEEN;
      break;
    case 'r':
    case 'R':
      move.promotion = ROOK;
      break;
    case 'b':
    case 'B':
      move.promotion = BISHOP;
      break;
    case 'n':
    case 'N':
      move.promotion = KNIGHT;
      break;
    default:
      move.promotion = QUEEN; /* fallback raisonnable */
      break;
    }
  }

  return move;
}

// Gestionnaire commande "go"
void handle_go(Board *board, char *params) {
  stop_search_flag = false; // Réinitialiser le drapeau d'arrêt

  // Extraire les paramètres (ex. profondeur, temps)
  int max_depth = 6;        // Valeur par défaut
  int time_limit_ms = 5000; // 5 secondes par défaut

  // Exemple d'extraction de la profondeur (simplifié)
  if (strstr(params, "depth")) {
    sscanf(params, "depth %d", &max_depth);
  }

  // Lancer la recherche
  SearchResult result =
      search_iterative_deepening(board, max_depth, time_limit_ms);

  // Envoyer le résultat au format UCI
  printf("info depth %d score cp %d nodes %d nps %d pv %s\n", result.depth,
         result.score, result.nodes, result.nps,
         move_to_string(&result.best_move));
  fflush(stdout);

  printf("bestmove %s\n", move_to_string(&result.best_move));
  fflush(stdout);
}

// Gestionnaire commande "stop"
void handle_stop() {
  stop_search_flag = true; // Lever le drapeau pour arrêter la recherche
  printf("# Search stopped\n");
  fflush(stdout);
}

// Gestionnaire commande "quit"
void handle_quit() { exit(0); }

// Applique un coup UCI en mettant à jour correctement l'état du board
void apply_uci_move(Board *board, const Move *move) {
  /* IMPORTANT: save the color that is moving BEFORE applying the temporary
     move. make_move_temp flips board->to_move, so reading board->to_move AFTER
     would be wrong. */
  Board backup;
  Couleur moving_color = board->to_move;

  /* Apply the move physically (make_move_temp will flip board->to_move) */
  make_move_temp(board, move, &backup);

  /* We increment the full-move number after Black has moved (i.e. when
   * moving_color == BLACK) */
  if (moving_color == BLACK) {
    board->move_number++;
  }

  PieceType piece_type = get_piece_type(&backup, move->from);

  /* Handle castling explicitly using the known moving_color */
  if (move->type == MOVE_CASTLE) {
    Square king_from = move->from;
    Square king_to = move->to;
    int is_kingside = (king_to > king_from);

    // Déterminer les positions de la tour
    Square rook_from = is_kingside ? ((moving_color == WHITE) ? H1 : H8)
                                   : ((moving_color == WHITE) ? A1 : A8);
    Square rook_to = is_kingside ? ((moving_color == WHITE) ? F1 : F8)
                                 : ((moving_color == WHITE) ? D1 : D8);

    // Déplacer roi et tour
    board->pieces[moving_color][KING] &= ~(1ULL << king_from);
    board->pieces[moving_color][KING] |= (1ULL << king_to);
    board->pieces[moving_color][ROOK] &= ~(1ULL << rook_from);
    board->pieces[moving_color][ROOK] |= (1ULL << rook_to);

    /* Rebuild occupied and all_pieces for moving_color */
    board->occupied[moving_color] = 0;
    for (PieceType pt = PAWN; pt <= KING; pt++)
      board->occupied[moving_color] |= board->pieces[moving_color][pt];
    board->all_pieces = board->occupied[WHITE] | board->occupied[BLACK];

    return;
  }

  /* En-passant capture: remove captured pawn using moving_color */
  if (move->type == MOVE_EN_PASSANT) {
    int captured_square = (moving_color == WHITE) ? move->to - 8 : move->to + 8;
    board->pieces[(moving_color == WHITE) ? BLACK : WHITE][PAWN] &=
        ~(1ULL << captured_square);
    board->occupied[(moving_color == WHITE) ? BLACK : WHITE] &=
        ~(1ULL << captured_square);
  }

  /* Normal move: the piece was already moved by make_move_temp, but we must
   * ensure captures are removed */
  Couleur opponent = (moving_color == WHITE) ? BLACK : WHITE;
  for (PieceType pt = PAWN; pt <= KING; pt++) {
    if (board->pieces[opponent][pt] & (1ULL << move->to)) {
      board->pieces[opponent][pt] &= ~(1ULL << move->to);
      break;
    }
  }

  /* Recompute occupied bitboards and all_pieces for both colors */
  board->occupied[moving_color] = 0;
  for (PieceType pt = PAWN; pt <= KING; pt++)
    board->occupied[moving_color] |= board->pieces[moving_color][pt];

  board->occupied[opponent] = 0;
  for (PieceType pt = PAWN; pt <= KING; pt++)
    board->occupied[opponent] |= board->pieces[opponent][pt];

  board->all_pieces = board->occupied[WHITE] | board->occupied[BLACK];
}

// Appliquer une séquence de coups UCI

void apply_uci_moves(Board *board, char *moves_str) {

  char moves_copy[2048];
  strncpy(moves_copy, moves_str, sizeof(moves_copy) - 1);
  moves_copy[sizeof(moves_copy) - 1] = '\0';

  char *move_str = strtok(moves_copy, " ");
  while (move_str != NULL) {
    Move uci_move = parse_uci_move(move_str);

    MoveList legal_moves;
    generate_legal_moves(board, &legal_moves);

    Move actual_move = {0};
    int found = 0;
    for (int i = 0; i < legal_moves.count; i++) {
      if (legal_moves.moves[i].from == uci_move.from &&
          legal_moves.moves[i].to == uci_move.to) {
        if (uci_move.type == MOVE_PROMOTION &&
            legal_moves.moves[i].type == MOVE_PROMOTION &&
            legal_moves.moves[i].promotion == uci_move.promotion) {
          actual_move = legal_moves.moves[i];
          found = 1;
          break;
        } else if (uci_move.type == MOVE_NORMAL) {
          actual_move = legal_moves.moves[i];
          found = 1;
          break;
        }
      }
    }

    if (found) {
      apply_uci_move(board, &actual_move);
    } else {
      /* Signaler explicitement que le coup est illégal */
      printf("info string illegal move '%s' for side %d\n", move_str,
             board->to_move);
      fflush(stdout);
    }

    move_str = strtok(NULL, " ");
  }
}
