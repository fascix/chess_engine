#include "timemanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG_TIME(...) fprintf(stderr, "[TIME] " __VA_ARGS__)
#else
#define DEBUG_LOG_TIME(...)
#endif

// Constantes pour la gestion du temps
#define DEFAULT_TIME_MS 1000
#define MIN_TIME_MS 50
#define PANIC_THRESHOLD_MS 1000
#define PANIC_TIME_DIVISOR 5
#define PANIC_MIN_TIME_MS 10
#define SAFETY_BUFFER_MS 50
#define MAX_TIME_PER_MOVE_MS 5000
#define MAX_TIME_FRACTION 10 // Utiliser max 1/10 du temps restant

// Parse les paramètres de la commande "go"
void parse_go_params(char *params, GoParams *go_params) {
  // Initialiser avec des valeurs par défaut
  go_params->wtime = -1;
  go_params->btime = -1;
  go_params->winc = 0;
  go_params->binc = 0;
  go_params->movestogo = -1;
  go_params->depth = -1;
  go_params->nodes = -1;
  go_params->mate = -1;
  go_params->movetime = -1;
  go_params->infinite = 0;
  go_params->ponder = 0;

  DEBUG_LOG_TIME("Parsing go params: '%s'\n", params ? params : "(null)");

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
    } else if (strcmp(token, "nodes") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->nodes = atoi(token);
    } else if (strcmp(token, "mate") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->mate = atoi(token);
    } else if (strcmp(token, "movetime") == 0) {
      token = strtok(NULL, " ");
      if (token)
        go_params->movetime = atoi(token);
    } else if (strcmp(token, "infinite") == 0) {
      go_params->infinite = 1;
    } else if (strcmp(token, "ponder") == 0) {
      go_params->ponder = 1;
    } else if (strcmp(token, "searchmoves") == 0) {
      // Ignorer proprement searchmoves (liste de coups à considérer)
      // Format: searchmoves e2e4 d2d4 ...
      // On saute tous les tokens jusqu'à un autre keyword
      DEBUG_LOG_TIME("searchmoves parameter ignored\n");
      token = strtok(NULL, " ");
      while (token != NULL) {
        // Vérifier si c'est un keyword connu
        if (strcmp(token, "wtime") == 0 || strcmp(token, "btime") == 0 ||
            strcmp(token, "winc") == 0 || strcmp(token, "binc") == 0 ||
            strcmp(token, "movestogo") == 0 || strcmp(token, "depth") == 0 ||
            strcmp(token, "nodes") == 0 || strcmp(token, "mate") == 0 ||
            strcmp(token, "movetime") == 0 || strcmp(token, "infinite") == 0 ||
            strcmp(token, "ponder") == 0) {
          // Revenir en arrière pour retraiter ce token
          break;
        }
        token = strtok(NULL, " ");
      }
      continue; // Retraiter le token courant
    } else {
      token = strtok(NULL, " ");
    }
  }

  DEBUG_LOG_TIME(
      "Parsed: wtime=%d btime=%d winc=%d binc=%d depth=%d movetime=%d\n",
      go_params->wtime, go_params->btime, go_params->winc, go_params->binc,
      go_params->depth, go_params->movetime);
}

// Estime le nombre de coups restants selon la phase de jeu
int estimate_moves_to_go(const Board *board) {
  if (board->move_number < 10) {
    return 30; // Début de partie
  } else if (board->move_number < 30) {
    return 20; // Milieu de partie
  } else {
    return 15; // Fin de partie
  }
}

// Calcule le temps alloué pour ce coup
int calculate_time_for_move(const Board *board, const GoParams *params) {
  DEBUG_LOG_TIME("Calculating time for move (to_move=%s)\n",
                 board->to_move == WHITE ? "WHITE" : "BLACK");

  // Cas 1: Temps fixe spécifié
  if (params->movetime > 0) {
    int allocated_time = (int)(params->movetime * 0.9); // 90% du movetime
    DEBUG_LOG_TIME("Using fixed movetime: %dms, allocated: %dms\n", params->movetime, allocated_time);
    return allocated_time;
  }

  // Cas 2: Mode infini
  if (params->infinite) {
    DEBUG_LOG_TIME("Infinite search mode\n");
    return 3600000; // 1 heure
  }

  // Récupérer le temps et l'incrément selon la couleur
  int my_time = (board->to_move == WHITE) ? params->wtime : params->btime;
  int my_inc = (board->to_move == WHITE) ? params->winc : params->binc;

  DEBUG_LOG_TIME("Available: time=%dms, inc=%dms\n", my_time, my_inc);

  // Cas 3: Pas de temps spécifié
  if (my_time < 0) {
    DEBUG_LOG_TIME("No time specified, using default: %dms\n", DEFAULT_TIME_MS);
    return DEFAULT_TIME_MS;
  }

  // Cas 4: Mode panique (moins d'1 seconde)
  if (my_time < PANIC_THRESHOLD_MS) {
    int panic_time = my_time / PANIC_TIME_DIVISOR;
    if (panic_time < PANIC_MIN_TIME_MS) {
      panic_time = PANIC_MIN_TIME_MS;
    }
    DEBUG_LOG_TIME("Panic mode: %dms\n", panic_time);
    return panic_time;
  }

  // Cas 5: Gestion normale du temps
  int moves_to_go =
      params->movestogo > 0 ? params->movestogo : estimate_moves_to_go(board);

  // Formule: (temps_restant * 2) / (coups_restants * 3) + incrément
  // Cela alloue ~2/3 du temps moyen par coup, laissant une marge
  int allocated_time = (my_time * 2 / (moves_to_go * 3)) + my_inc;

  // Sécurité 1: Maximum 1/10 du temps restant + incrément
  int max_time = (my_time / MAX_TIME_FRACTION) + my_inc;
  if (allocated_time > max_time) {
    allocated_time = max_time;
  }

  // Sécurité 2: Minimum pour permettre une recherche décente
  if (allocated_time < MIN_TIME_MS) {
    allocated_time = MIN_TIME_MS;
  }

  // Sécurité 3: Buffer de sécurité pour éviter les time-outs (5%)
  int safety_margin = (int)(allocated_time * 0.05);
  if (allocated_time > safety_margin) {
    allocated_time -= safety_margin;
  }

  // Sécurité 4: Maximum absolu pour positions complexes
  if (allocated_time > MAX_TIME_PER_MOVE_MS) {
    allocated_time = MAX_TIME_PER_MOVE_MS;
  }

  DEBUG_LOG_TIME("Allocated time: %dms (moves_to_go=%d)\n", allocated_time,
                 moves_to_go);

  return allocated_time;
}
