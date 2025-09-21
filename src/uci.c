#include "uci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variable globale pour mode debug
int uci_debug = 0;

// Boucle principale UCI
void uci_loop() {
  char line[2048];
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
  printf("id author Lucas Pavone\n");
  printf("uciok\n");
  fflush(stdout);
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

  // e2e4 -> from=E2 (12), to=E4 (28)
  move.from = (uci_str[1] - '1') * 8 + (uci_str[0] - 'a');
  move.to = (uci_str[3] - '1') * 8 + (uci_str[2] - 'a');

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
  int depth = 5;
  int movetime = 0;

  // Parser les paramètres...
  if (params && strstr(params, "depth")) {
    char *depth_pos = strstr(params, "depth");
    sscanf(depth_pos, "depth %d", &depth);
  }

  if (params && strstr(params, "movetime")) {
    char *movetime_pos = strstr(params, "movetime");
    sscanf(movetime_pos, "movetime %d", &movetime);
  }

  // Lancer la recherche
  SearchResult result;
  if (movetime > 0) {
    result = search_iterative_deepening(board, depth, movetime);
  } else {
    result = search_best_move(board, depth);
  }

  // AJOUT: Vérification que le coup est vraiment légal
  MoveList legal_moves;
  generate_legal_moves(board, &legal_moves);

  int move_is_legal = 0;
  for (int i = 0; i < legal_moves.count; i++) {
    if (legal_moves.moves[i].from == result.best_move.from &&
        legal_moves.moves[i].to == result.best_move.to &&
        legal_moves.moves[i].type == result.best_move.type) {
      move_is_legal = 1;
      result.best_move = legal_moves.moves[i]; // Utiliser la version complète
      break;
    }
  }

  if (!move_is_legal && legal_moves.count > 0) {
    // Fallback: prendre le premier coup légal
    result.best_move = legal_moves.moves[0];
  }

  // Envoyer le meilleur coup
  printf("bestmove %s\n", move_to_string(&result.best_move));
  fflush(stdout);
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

// Version améliorée de make_move_temp qui met à jour to_move
void apply_move_properly(Board *board, const Move *move) {
  // Utiliser make_move_temp existant
  Board backup; // Non utilisé, juste pour l'interface
  make_move_temp(board, move, &backup);

  // CORRECTION: Basculer le joueur actuel
  board->to_move = (board->to_move == WHITE) ? BLACK : WHITE;

  // Optionnel: incrémenter le numéro de coup si c'est un coup des noirs
  if (board->to_move == WHITE) {
    board->move_number++;
  }
}

// Appliquer une séquence de coups UCI

void apply_uci_moves(Board *board, char *moves_str) {
  char moves_copy[512];
  strncpy(moves_copy, moves_str, sizeof(moves_copy) - 1);
  moves_copy[sizeof(moves_copy) - 1] = '\0';

  char *move_str = strtok(moves_copy, " ");
  while (move_str != NULL) {
    // Parser le coup UCI
    Move uci_move = parse_uci_move(move_str);

    // Trouver le coup correspondant dans la liste des coups légaux
    MoveList legal_moves;
    generate_legal_moves(board, &legal_moves);

    // Chercher le coup qui correspond
    Move actual_move = {0};
    int found = 0;
    for (int i = 0; i < legal_moves.count; i++) {
      if (legal_moves.moves[i].from == uci_move.from &&
          legal_moves.moves[i].to == uci_move.to) {
        // Vérifier promotion si nécessaire
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

    // Appliquer le coup s'il est trouvé
    if (found) {
      apply_move_properly(board, &actual_move);
    }

    move_str = strtok(NULL, " ");
  }
}