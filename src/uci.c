#include "uci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variable globale pour mode debug
int uci_debug = 0;

// Boucle principale UCI
void uci_loop() {
  char line[4096];
  Board board;

  // Initialiser le board en position initiale
  board_from_fen(&board,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  while (fgets(line, sizeof(line), stdin)) {
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
  printf("id author Lucas Pavone\n");
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
  Move move = {0};

  if (strlen(uci_str) < 4)
    return move; // Coup invalide

  // Correction : UCI e2e4 -> from = rank*8 + file
  int file_from = uci_str[0] - 'a';
  int rank_from = uci_str[1] - '1';
  int file_to = uci_str[2] - 'a';
  int rank_to = uci_str[3] - '1';

  move.from = rank_from * 8 + file_from;
  move.to = rank_to * 8 + file_to;

  // Vérifier promotion (e7e8q)
  if (strlen(uci_str) == 5) {
    move.type = MOVE_PROMOTION;
    switch (uci_str[4]) {
    case 'q':
      move.promotion = QUEEN;
      break;
    case 'r':
      move.promotion = ROOK;
      break;
    case 'b':
      move.promotion = BISHOP;
      break;
    case 'n':
      move.promotion = KNIGHT;
      break;
    }
  } else {
    move.type = MOVE_NORMAL; // On déterminera le type exact plus tard
  }

  return move;
}

// Gestionnaire commande "go"
void handle_go(Board *board, char *params) {
  MoveList legal_moves;
  generate_legal_moves(board, &legal_moves);

  if (legal_moves.count == 0) {
    printf("bestmove (none)\n");
    fflush(stdout);
    return;
  }

  int index = rand() % legal_moves.count;
  Move best_move = legal_moves.moves[index];

  // Envoyer une ligne info minimale pour respecter UCI
  printf("info depth 1 score cp 0 nodes 1 nps 1 pv %s\n",
         move_to_string(&best_move));
  fflush(stdout);
  printf("bestmove %s\n", move_to_string(&best_move));
  fflush(stdout); // 🔹 essentiel pour UCI
}

// Gestionnaire commande "stop"
void handle_stop() {
  // TODO: Implémenter arrêt de recherche propre
  // Variable globale ou signal pour arrêter search_iterative_deepening
  printf("# Search stopped\n");
  fflush(stdout);
}

// Gestionnaire commande "quit"
void handle_quit() {
  if (uci_debug) {
    printf("# Goodbye!\n");
  }
  exit(0);
}

// Version améliorée de make_move_temp qui met à jour to_move avec logs debug
void apply_move_properly(Board *board, const Move *move) {
  // Utiliser make_move_temp existant
  Board backup; // Non utilisé, juste pour l'interface
  make_move_temp(board, move, &backup);

  // Basculer le joueur actif UNIQUEMENT ici
  board->to_move = (board->to_move == WHITE) ? BLACK : WHITE;

  // Incrémenter le numéro de coup seulement après le coup des noirs
  if (board->to_move == WHITE) {
    board->move_number++;
  }

  // 🔹 Correction: mettre à jour le bitboard pièce déplacée sans écraser les
  // autres pions (Remplace la ligne: board->pieces[piece_color][piece_type] |=
  // (1ULL << move->to);) Trouver la couleur et le type de pièce déplacée
  PieceType piece_type = get_piece_type(board, move->to);
  Couleur piece_color =
      (board->to_move == WHITE) ? BLACK : WHITE; // car on a déjà changé to_move
  board->pieces[piece_color][piece_type] &=
      ~(1ULL << move->from); // effacer départ
  board->pieces[piece_color][piece_type] |=
      (1ULL << move->to); // placer arrivée

  // Les autres mises à jour (board->occupied[piece_color], board->all_pieces)
  // restent correctes

  // 🔹 Logs de debug
  fprintf(stderr, "\n[DEBUG] Après apply_move_properly:\n");
  fprintf(stderr, "  to_move = %s\n",
          board->to_move == WHITE ? "WHITE" : "BLACK");
  fprintf(stderr, "  en_passant = %d\n", board->en_passant);
  fprintf(stderr, "  castle_rights = 0x%X\n", board->castle_rights);

  // Pièces autour de f3 (cases e2-h4)
  for (int r = 1; r <= 3; r++) {
    for (int f = 4; f <= 5; f++) {
      Square sq = r * 8 + f; // f3=fichier 5? Hm, f=5 corresponds à f-file
      char c = get_piece_color(board, sq);
      fprintf(stderr, "  Square %c%d = %c\n", 'a' + f, r + 1, c);
    }
  }

  // Vérifier pion à f3
  Square f3 = 5 + 2 * 8; // f3 -> file 5, rank 2 (0-indexed)
  int pawn_present = (board->pieces[WHITE][PAWN] & (1ULL << f3)) ? 1 : 0;
  fprintf(stderr, "  WHITE PAWN at f3 exists? %d\n", pawn_present);
  fprintf(stderr, "  WHITE PAWN bitboard = 0x%llX\n",
          board->pieces[WHITE][PAWN]);
  fflush(stderr);
}

// Appliquer une séquence de coups UCI

void apply_uci_moves(Board *board, char *moves_str) {
  char moves_copy[512];
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

    // Ajout des logs DEBUG juste avant de vérifier la légalité du coup
    if (found) {
      // Logs DEBUG détaillés pour le coup UCI en cours
      fprintf(stderr, "\n[DEBUG UCI] Testing move: %s\n", move_str);
      fprintf(stderr, "  Current to_move: %s\n",
              board->to_move == WHITE ? "WHITE" : "BLACK");
      // Bitboards autour de f3 (cases e2-h4)
      // e2: 12, f2: 13, g2: 14, h2: 15
      // e3: 20, f3: 21, g3: 22, h3: 23
      // e4: 28, f4: 29, g4: 30, h4: 31
      int squares[] = {12, 13, 14, 15, 20, 21, 22, 23, 28, 29, 30, 31};
      fprintf(stderr, "  Squares e2-h4:\n");
      for (int si = 0; si < 12; ++si) {
        int sq = squares[si];
        char file = 'a' + (sq % 8);
        char rank = '1' + (sq / 8);
        char pc = get_piece_color(board, sq);
        fprintf(stderr, "    %c%c: %c", file, rank, pc);
        if ((si + 1) % 4 == 0)
          fprintf(stderr, "\n");
        else
          fprintf(stderr, "  ");
      }
      // Bitboard des pions blancs et noirs
      fprintf(stderr, "  WHITE PAWN bitboard: 0x%016llX\n",
              board->pieces[WHITE][PAWN]);
      fprintf(stderr, "  BLACK PAWN bitboard: 0x%016llX\n",
              board->pieces[BLACK][PAWN]);
      fflush(stderr);
    }

    if (found) {
      apply_move_properly(board, &actual_move);
    }

    move_str = strtok(NULL, " ");
  }
}