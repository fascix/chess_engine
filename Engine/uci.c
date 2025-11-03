#include "uci.h"
#include "search.h"
#include "timemanager.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macro pour logs de debug conditionnels (dynamique)
#define DEBUG_LOG_UCI(...)                                                     \
  do {                                                                         \
    if (uci_debug)                                                             \
      fprintf(stderr, "[UCI] " __VA_ARGS__);                                   \
  } while (0)

volatile bool stop_search_flag = false; // Variable pour arrêter la recherche
volatile int uci_debug = 0;             // Debug dynamique (0 ou 1)

// Options UCI configurables
UCIOptions uci_options = {
    .hash_size_mb = 16, // Défaut: 16 MB
    .ponder = 0,        // Défaut: désactivé
    .own_book = 0,      // Défaut: pas de livre
    .analyse_mode = 0   // Défaut: mode normal
};

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
  } else if (strcmp(command, "debug") == 0) {
    char *params = strtok(NULL, "");
    handle_debug(params);
  } else if (strcmp(command, "setoption") == 0) {
    char *params = strtok(NULL, "");
    handle_setoption(params);
  } else if (strcmp(command, "register") == 0) {
    char *params = strtok(NULL, "");
    handle_register(params);
  } else if (strcmp(command, "ucinewgame") == 0) {
    handle_ucinewgame();
  } else if (strcmp(command, "position") == 0) {
    char *params = strtok(NULL, "");
    handle_position(board, params);
  } else if (strcmp(command, "go") == 0) {
    char *params = strtok(NULL, "");
    handle_go(board, params);
  } else if (strcmp(command, "ponderhit") == 0) {
    handle_ponderhit();
  } else if (strcmp(command, "stop") == 0) {
    handle_stop();
  } else if (strcmp(command, "quit") == 0) {
    handle_quit();
  } else {
    DEBUG_LOG_UCI("Unknown command: %s\n", command);
  }
}

// Gestionnaire commande "uci"
void handle_uci() {
  printf("id name ChessEngine v2.0\n");
  fflush(stdout);
  printf("id author Fascix\n");
  fflush(stdout);

  // Envoyer les options supportées (obligatoire pour conformité UCI)
  printf("option name Hash type spin default 16 min 1 max 1024\n");
  fflush(stdout);
  printf("option name Ponder type check default false\n");
  fflush(stdout);
  printf("option name OwnBook type check default false\n");
  fflush(stdout);
  printf("option name UCI_AnalyseMode type check default false\n");
  fflush(stdout);

  printf("uciok\n");
  fflush(stdout);
}

// Gestionnaire commande "isready"
void handle_isready() {
  printf("readyok\n");
  fflush(stdout);
}

// Gestionnaire commande "debug"
void handle_debug(char *params) {
  if (!params)
    return;

  if (strcmp(params, "on") == 0) {
    uci_debug = 1;
    fprintf(stderr, "[UCI] Debug mode enabled\n");
  } else if (strcmp(params, "off") == 0) {
    uci_debug = 0;
    fprintf(stderr, "[UCI] Debug mode disabled\n");
  }
}

// Gestionnaire commande "setoption"
void handle_setoption(char *params) {
  if (!params)
    return;

  // Format: name <id> [value <x>]
  char *name_token = strstr(params, "name");
  if (!name_token)
    return;

  name_token += 5; // Passer "name "
  while (*name_token == ' ')
    name_token++; // Sauter les espaces

  char *value_token = strstr(name_token, " value ");
  char option_name[256];

  if (value_token) {
    size_t name_len = value_token - name_token;
    strncpy(option_name, name_token, name_len);
    option_name[name_len] = '\0';
    value_token += 7; // Passer " value "
  } else {
    strncpy(option_name, name_token, sizeof(option_name) - 1);
    option_name[sizeof(option_name) - 1] = '\0';
  }

  // Traiter les options connues
  if (strcmp(option_name, "Hash") == 0 && value_token) {
    int hash_mb = atoi(value_token);
    if (hash_mb >= 1 && hash_mb <= 1024) {
      uci_options.hash_size_mb = hash_mb;
      DEBUG_LOG_UCI("Hash set to %d MB\n", hash_mb);
    }
  } else if (strcmp(option_name, "Ponder") == 0 && value_token) {
    uci_options.ponder = (strcmp(value_token, "true") == 0) ? 1 : 0;
    DEBUG_LOG_UCI("Ponder set to %d\n", uci_options.ponder);
  } else if (strcmp(option_name, "OwnBook") == 0 && value_token) {
    uci_options.own_book = (strcmp(value_token, "true") == 0) ? 1 : 0;
    DEBUG_LOG_UCI("OwnBook set to %d\n", uci_options.own_book);
  } else if (strcmp(option_name, "UCI_AnalyseMode") == 0 && value_token) {
    uci_options.analyse_mode = (strcmp(value_token, "true") == 0) ? 1 : 0;
    DEBUG_LOG_UCI("UCI_AnalyseMode set to %d\n", uci_options.analyse_mode);
  } else {
    DEBUG_LOG_UCI("Unknown or unsupported option: %s\n", option_name);
  }
}

// Gestionnaire commande "register"
void handle_register(char *params) {
  // Cette commande est optionnelle (protection contre la copie)
  // On l'ignore mais on la reconnaît pour éviter "unknown command"
  DEBUG_LOG_UCI("Register command received (ignored): %s\n",
                params ? params : "");
  // On pourrait répondre "registration ok" ou "registration error"
  // Pour un moteur libre, on ne fait rien
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
  board->to_move = WHITE;
  board->en_passant = -1;
  board->halfmove_clock = 0;
  board->move_number = 1;
}

void setup_from_fen(Board *board, char *params) {
  char *fen_start = strstr(params, "fen");
  if (!fen_start)
    return;

  fen_start += 4; // Passer "fen "
  char *moves_start = strstr(fen_start, " moves");

  if (moves_start) {
    // FEN se termine avant "moves"
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

  // Promotion possible si un 5ème caractère est fourni
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
      move.promotion = QUEEN; // fallback raisonnable
      break;
    }
  }

  return move;
}

// Récupère un coup d'urgence (premier coup légal disponible)
static int get_emergency_move(Board *board, Move *out_move) {
  MoveList emergency_moves;
  generate_legal_moves(board, &emergency_moves);

  if (emergency_moves.count > 0) {
    *out_move = emergency_moves.moves[0];
    DEBUG_LOG_UCI("Emergency fallback: %s\n", move_to_string(out_move));
    return 1;
  }

  DEBUG_LOG_UCI("ERROR: No legal moves available!\n");
  return 0;
}

// Valide qu'un coup est légal et le corrige si nécessaire
static int validate_and_fix_move(Board *board, Move *move) {
  // Validation 1: Coup invalide (from == -1 ou from == to)
  if (move->from == -1 || move->to == -1 || move->from == move->to) {
    DEBUG_LOG_UCI("❌ Invalid move structure (from=%d, to=%d)\n", move->from,
                  move->to);
    return get_emergency_move(board, move);
  }

  // Validation 2: Vérifier qu'une pièce valide existe sur la case from
  PieceType piece_on_from = get_piece_type(board, move->from);
  Couleur color_on_from = get_piece_color(board, move->from);

  if (piece_on_from == EMPTY || color_on_from != board->to_move) {
    DEBUG_LOG_UCI("❌ No valid piece on from square! from=%c%d piece=%d "
                  "color=%d expected_color=%d\n",
                  'a' + (move->from % 8), 1 + (move->from / 8), piece_on_from,
                  color_on_from, board->to_move);
    return get_emergency_move(board, move);
  }

  // Validation 3: Vérifier que le coup est dans la liste des coups légaux
  MoveList legal_moves;
  generate_legal_moves(board, &legal_moves);

  for (int i = 0; i < legal_moves.count; i++) {
    if (legal_moves.moves[i].from == move->from &&
        legal_moves.moves[i].to == move->to &&
        legal_moves.moves[i].type == move->type) {
      // Utiliser le coup exact de la liste légale
      *move = legal_moves.moves[i];
      return 1;
    }
  }

  DEBUG_LOG_UCI("❌ Move %s is NOT legal in current position!\n",
                move_to_string(move));
  return get_emergency_move(board, move);
}

// Gestionnaire commande "go"
void handle_go(Board *board, char *params) {
  DEBUG_LOG_UCI("=== HANDLE_GO START ===\n");
  stop_search_flag = false;

  // Parser les paramètres (dupliquer car strtok modifie la chaîne)
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
  int max_depth = (go_params.depth > 0) ? go_params.depth : 64;

  DEBUG_LOG_UCI("Starting search: max_depth=%d, time_limit=%dms\n", max_depth,
                time_limit_ms);

  // Lancer la recherche
  SearchResult result =
      search_iterative_deepening(board, max_depth, time_limit_ms);

  DEBUG_LOG_UCI("Search complete: depth=%d, score=%d, nodes=%d, nps=%d\n",
                result.depth, result.score, result.nodes, result.nps);

  // Valider et corriger le coup si nécessaire
  if (!validate_and_fix_move(board, &result.best_move)) {
    // Aucun coup légal disponible
    printf("bestmove 0000\n");
    fflush(stdout);
    DEBUG_LOG_UCI("=== HANDLE_GO END (NO LEGAL MOVES) ===\n\n");
    return;
  }

  // Envoyer le bestmove (info déjà envoyé par search_iterative_deepening)
  printf("bestmove %s\n", move_to_string(&result.best_move));
  fflush(stdout);

  DEBUG_LOG_UCI("=== HANDLE_GO END ===\n\n");
}

// Gestionnaire commande "ponderhit"
void handle_ponderhit() {
  // Quand le moteur était en mode "ponder" et que le coup prédit a été joué
  // On doit basculer la recherche en mode normal
  // Pour l'instant, on reconnaît la commande sans l'implémenter
  DEBUG_LOG_UCI("Ponderhit received (pondering not yet implemented)\n");
  // TODO: Quand le pondering sera implémenté, basculer le temps en mode normal
}

// Gestionnaire commande "stop"
void handle_stop() {
  stop_search_flag = true;
  DEBUG_LOG_UCI("Stop command received\n");
  fflush(stdout);
}

// Gestionnaire commande "quit"
void handle_quit() {
  DEBUG_LOG_UCI("Quit command received, exiting\n");
  exit(0);
}

// Applique un coup UCI en mettant à jour l'état du board
void apply_uci_move(Board *board, const Move *move) {
  Board dummy_backup;

  // Sauvegarder les informations AVANT le coup
  PieceType moving_piece = get_piece_type(board, move->from);
  int is_capture =
      (move->type == MOVE_CAPTURE || move->type == MOVE_EN_PASSANT);
  Couleur moving_color = board->to_move;

  // Appliquer le coup
  make_move_temp(board, move, &dummy_backup);

  // Mettre à jour les compteurs de coups
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
      if (legal_moves.moves[i].from == uci_move.from &&
          legal_moves.moves[i].to == uci_move.to) {

        // Si promotion, vérifier aussi le type de promotion
        if (uci_move.type == MOVE_PROMOTION) {
          if (legal_moves.moves[i].type == MOVE_PROMOTION &&
              legal_moves.moves[i].promotion == uci_move.promotion) {
            actual_move = legal_moves.moves[i];
            found = 1;
            break;
          }
        } else {
          // Pour les autres coups, la correspondance des cases suffit
          actual_move = legal_moves.moves[i];
          found = 1;
          break;
        }
      }
    }

    if (found) {
      apply_uci_move(board, &actual_move);
    } else {
      printf("info string illegal move '%s' for side %d\n", move_str,
             board->to_move);
      fflush(stdout);
    }

    move_str = strtok(NULL, " ");
  }
}
