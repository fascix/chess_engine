#include "uci.h"
#include "search.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG_UCI(...) fprintf(stderr, "[UCI] " __VA_ARGS__)
#else
#define DEBUG_LOG_UCI(...)
#endif

volatile bool stop_search_flag = false; // Variable pour arrêter la recherche

// Structure pour les paramètres de la commande "go"
typedef struct {
  int wtime;     // Temps restant pour les blancs (ms)
  int btime;     // Temps restant pour les noirs (ms)
  int winc;      // Incrément pour les blancs (ms)
  int binc;      // Incrément pour les noirs (ms)
  int movestogo; // Nombre de coups avant le prochain contrôle de temps
  int depth;     // Profondeur maximale
  int movetime;  // Temps fixe pour ce coup (ms)
  int infinite;  // Recherche infinie
} GoParams;

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
  } else if (strcmp(command, "ucinewgame") == 0) {
    handle_ucinewgame();
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

// Gestionnaire commande "ucinewgame"
void handle_ucinewgame() {
  // Clear transposition table and reset search state for a new game
  initialize_engine();
  DEBUG_LOG_UCI("New game started, engine reset\n");
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

// Parse les paramètres de la commande "go"
void parse_go_params(char *params, GoParams *go_params) {
  // Initialiser avec des valeurs par défaut
  go_params->wtime = -1;
  go_params->btime = -1;
  go_params->winc = 0;
  go_params->binc = 0;
  go_params->movestogo = -1;
  go_params->depth = -1;
  go_params->movetime = -1;
  go_params->infinite = 0;

  DEBUG_LOG_UCI("Parsing go params: '%s'\n", params ? params : "(null)");

  if (!params)
    return;

  char *token = strtok(params, " ");
  while (token != NULL) {
    if (strcmp(token, "wtime") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->wtime = atoi(token);
    } else if (strcmp(token, "btime") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->btime = atoi(token);
    } else if (strcmp(token, "winc") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->winc = atoi(token);
    } else if (strcmp(token, "binc") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->binc = atoi(token);
    } else if (strcmp(token, "movestogo") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->movestogo = atoi(token);
    } else if (strcmp(token, "depth") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->depth = atoi(token);
    } else if (strcmp(token, "movetime") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->movetime = atoi(token);
    } else if (strcmp(token, "infinite") == 0) {
      go_params->infinite = 1;
    } else {
      token = strtok(NULL, " ");
    }
  }

  DEBUG_LOG_UCI(
      "Parsed: wtime=%d btime=%d winc=%d binc=%d depth=%d movetime=%d\n",
      go_params->wtime, go_params->btime, go_params->winc, go_params->binc,
      go_params->depth, go_params->movetime);
}

// Calcule le temps alloué pour ce coup
int calculate_time_for_move(Board *board, GoParams *params) {
  DEBUG_LOG_UCI("Calculating time for move (to_move=%s)\n",
                board->to_move == WHITE ? "WHITE" : "BLACK");

  // Si movetime est spécifié, l'utiliser directement
  if (params->movetime > 0) {
    DEBUG_LOG_UCI("Using fixed movetime: %dms\n", params->movetime);
    return params->movetime;
  }

  // Si infinite, retourner un temps très long
  if (params->infinite) {
    DEBUG_LOG_UCI("Infinite search mode\n");
    return 3600000; // 1 heure
  }

  // Récupérer le temps et l'incrément selon la couleur
  int my_time = (board->to_move == WHITE) ? params->wtime : params->btime;
  int my_inc = (board->to_move == WHITE) ? params->winc : params->binc;

  DEBUG_LOG_UCI("Time available: my_time=%dms, my_inc=%dms\n", my_time, my_inc);

  // Si pas de temps spécifié, utiliser une valeur par défaut
  if (my_time < 0) {
    DEBUG_LOG_UCI("No time specified, using default: 1000ms\n");
    return 1000; // 1 seconde par défaut
  }

  // Calcul du temps alloué avec stratégie CONSERVATIVE pour éviter les pertes
  // au temps
  int moves_to_go = params->movestogo;
  if (moves_to_go <= 0) {
    // Estimer le nombre de coups restants de manière conservatrice
    moves_to_go = 40; // Hypothèse : 40 coups jusqu'à la fin
  }

  // Formule TRÈS conservative : utiliser beaucoup moins de temps par coup
  // (temps_restant / (coups_restants * 3)) + (incrément * 0.5)
  int allocated_time = (my_time / (moves_to_go * 3)) + (my_inc / 2);

  // Sécurités renforcées :
  // 1. Ne JAMAIS utiliser plus de 1/15 du temps restant
  int max_time = my_time / 15;
  if (allocated_time > max_time) {
    allocated_time = max_time;
  }

  // 2. Minimum de 10ms pour éviter des moves trop rapides
  if (allocated_time < 10) {
    allocated_time = 10;
  }

  // 3. Laisser un buffer de 100ms pour la communication et overhead
  if (allocated_time > 100) {
    allocated_time -= 100;
  } else if (allocated_time > 20) {
    allocated_time -= 20;
  }

  // 4. Maximum absolu pour éviter de bloquer trop longtemps
  if (allocated_time > 2000) {
    allocated_time = 2000; // Max 2 secondes par coup
  }

  DEBUG_LOG_UCI("Allocated time for this move: %dms (moves_to_go=%d)\n",
                allocated_time, moves_to_go);

  return allocated_time;
}

// Gestionnaire commande "go"
void handle_go(Board *board, char *params) {
  DEBUG_LOG_UCI("=== HANDLE_GO START ===\n");
  stop_search_flag = false; // Réinitialiser le drapeau d'arrêt

  // Parser les paramètres
  // IMPORTANT: dupliquer params car parse_go_params utilise strtok qui modifie
  // la chaîne
  char params_copy[4096];
  if (params) {
    strncpy(params_copy, params, sizeof(params_copy) - 1);
    params_copy[sizeof(params_copy) - 1] = '\0';
  } else {
    params_copy[0] = '\0';
  }

  GoParams go_params;
  parse_go_params(params_copy, &go_params);

  // Calculer le temps alloué
  int time_limit_ms = calculate_time_for_move(board, &go_params);

  // Déterminer la profondeur maximale
  int max_depth =
      (go_params.depth > 0) ? go_params.depth : 64; // 64 = pratiquement infini

  DEBUG_LOG_UCI("Starting search: max_depth=%d, time_limit=%dms\n", max_depth,
                time_limit_ms);

  // Lancer la recherche
  SearchResult result =
      search_iterative_deepening(board, max_depth, time_limit_ms);

  DEBUG_LOG_UCI("Search complete: depth=%d, score=%d, nodes=%d, nps=%d\n",
                result.depth, result.score, result.nodes, result.nps);

  // ============================================================
  // VALIDATION CRITIQUE : Vérifier que le coup est VRAIMENT légal
  // ============================================================

  // 1. Vérifier que le coup n'est pas invalide (from == -1 ou from == to)
  if (result.best_move.from == -1 || result.best_move.to == -1 ||
      result.best_move.from == result.best_move.to) {
    DEBUG_LOG_UCI("❌ ERREUR CRITIQUE: Coup invalide retourné par la recherche "
                  "(from=%d, to=%d)!\n",
                  result.best_move.from, result.best_move.to);

    // Génération d'urgence d'un coup légal
    MoveList emergency_moves;
    generate_legal_moves(board, &emergency_moves);

    if (emergency_moves.count > 0) {
      result.best_move = emergency_moves.moves[0];
      DEBUG_LOG_UCI("    Fallback d'urgence: %s\n",
                    move_to_string(&result.best_move));
    } else {
      // Situation impossible (pas de coups légaux)
      DEBUG_LOG_UCI("    ERREUR: Aucun coup légal disponible!\n");
      printf("bestmove 0000\n");
      fflush(stdout);
      DEBUG_LOG_UCI("=== HANDLE_GO END (ERROR) ===\n\n");
      return;
    }
  }

  // 2. DOUBLE VÉRIFICATION : Le coup doit être dans la liste des coups légaux
  MoveList legal_moves_check;
  generate_legal_moves(board, &legal_moves_check);

  int move_is_legal = 0;
  for (int i = 0; i < legal_moves_check.count; i++) {
    if (legal_moves_check.moves[i].from == result.best_move.from &&
        legal_moves_check.moves[i].to == result.best_move.to &&
        legal_moves_check.moves[i].type == result.best_move.type) {
      // Utiliser le coup exact de la liste légale pour être sûr
      result.best_move = legal_moves_check.moves[i];
      move_is_legal = 1;
      break;
    }
  }

  if (!move_is_legal) {
    DEBUG_LOG_UCI("❌ ERREUR CRITIQUE: Le coup %s n'est PAS légal dans la "
                  "position actuelle!\n",
                  move_to_string(&result.best_move));

    // Fallback d'urgence
    if (legal_moves_check.count > 0) {
      result.best_move = legal_moves_check.moves[0];
      DEBUG_LOG_UCI("    Fallback d'urgence: %s\n",
                    move_to_string(&result.best_move));
    } else {
      DEBUG_LOG_UCI("    ERREUR: Aucun coup légal disponible!\n");
      printf("bestmove 0000\n");
      fflush(stdout);
      DEBUG_LOG_UCI("=== HANDLE_GO END (ERROR) ===\n\n");
      return;
    }
  }

  // 3. VÉRIFICATION FINALE : from != to (pas de coup vers la même case)
  if (result.best_move.from == result.best_move.to) {
    DEBUG_LOG_UCI(
        "❌ ERREUR CRITIQUE: Coup vers la même case détecté! (%d -> %d)\n",
        result.best_move.from, result.best_move.to);

    // Fallback ultime
    if (legal_moves_check.count > 0) {
      result.best_move = legal_moves_check.moves[0];
      DEBUG_LOG_UCI("    Fallback ultime: %s\n",
                    move_to_string(&result.best_move));
    }
  }

  // Envoyer le résultat au format UCI
  printf("info depth %d score cp %d nodes %d nps %d pv %s\n", result.depth,
         result.score, result.nodes, result.nps,
         move_to_string(&result.best_move));
  fflush(stdout);

  printf("bestmove %s\n", move_to_string(&result.best_move));
  fflush(stdout);

  DEBUG_LOG_UCI("=== HANDLE_GO END ===\n\n");
}

// Gestionnaire commande "stop"
void handle_stop() {
  stop_search_flag = true; // Lever le drapeau pour arrêter la recherche
  printf("# Search stopped\n");
  fflush(stdout);
}

// Gestionnaire commande "quit"
void handle_quit() { exit(0); }

// Applique un coup UCI en mettant à jour correctement l'état du board (version
// robuste)
void apply_uci_move(Board *board, const Move *move) {
  Board dummy_backup; // Non utilisé, mais requis par make_move_temp

  // Sauvegarder les informations AVANT que le coup ne soit joué
  PieceType moving_piece = get_piece_type(board, move->from);
  int is_capture =
      (move->type == MOVE_CAPTURE || move->type == MOVE_EN_PASSANT);
  Couleur moving_color = board->to_move;

  // Appliquer le coup en utilisant la fonction centralisée et corrigée
  make_move_temp(board, move, &dummy_backup);

  // Mettre à jour les compteurs de coups, que make_move_temp ne gère pas

  // Compteur de demi-coups (pour la règle des 50 coups)
  if (moving_piece == PAWN || is_capture) {
    board->halfmove_clock = 0;
  } else {
    board->halfmove_clock++;
  }

  // Compteur de coups complets (incrémenté après le coup des noirs)
  if (moving_color == BLACK) {
    board->move_number++;
  }
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

    Move actual_move;
    int found = 0;
    // Logique de correspondance robuste
    for (int i = 0; i < legal_moves.count; i++) {
      // Vérifier si les cases correspondent
      if (legal_moves.moves[i].from == uci_move.from &&
          legal_moves.moves[i].to == uci_move.to) {

        // Si le coup UCI est une promotion, le type de promotion doit aussi
        // correspondre
        if (uci_move.type == MOVE_PROMOTION) {
          if (legal_moves.moves[i].type == MOVE_PROMOTION &&
              legal_moves.moves[i].promotion == uci_move.promotion) {
            actual_move = legal_moves.moves[i];
            found = 1;
            break;
          }
        } else {
          // Pour tous les autres types de coups (normaux, captures, roques,
          // etc.), la correspondance des cases est suffisante.
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
