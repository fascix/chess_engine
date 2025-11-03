#include "search.h"
#include <stdio.h>
#include <string.h>

// Fonction d'initialisation du moteur (stub)
void initialize_engine() {
  // TODO: Implémenter l'initialisation des tables de transposition, etc.
#ifdef DEBUG
  fprintf(stderr, "[SEARCH] Engine initialized\n");
#endif
}

// Fonction utilitaire pour envoyer des infos UCI pendant la recherche
void send_search_info(int depth, int score, int nodes, int nps,
                      const Move *pv_move) {
  if (pv_move) {
    printf("info depth %d score cp %d nodes %d nps %d pv %s\n", depth, score,
           nodes, nps, move_to_string(pv_move));
  } else {
    printf("info depth %d score cp %d nodes %d nps %d\n", depth, score, nodes,
           nps);
  }
  fflush(stdout);
}

// Fonction de recherche itérative deepening (stub amélioré avec info UCI)
SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms) {
  // TODO: Implémenter la recherche réelle
  // Pour l'instant, retourner simplement le premier coup légal
  // MAIS envoyer des infos UCI pour montrer la progression

  SearchResult result;
  memset(&result, 0, sizeof(result));

  MoveList legal_moves;
  generate_legal_moves(board, &legal_moves);

  if (legal_moves.count > 0) {
    result.best_move = legal_moves.moves[0];
    result.depth = 1;
    result.score = 0;
    result.nodes = legal_moves.count;
    result.nps =
        legal_moves.count * 1000 / (time_limit_ms > 0 ? time_limit_ms : 1);

    // Envoyer une info UCI pour montrer qu'on a trouvé un coup
    send_search_info(result.depth, result.score, result.nodes, result.nps,
                     &result.best_move);
  } else {
    // Aucun coup légal - retourner un coup invalide
    result.best_move.from = -1;
    result.best_move.to = -1;
    result.depth = 0;
    result.score = 0;
    result.nodes = 0;
    result.nps = 0;
  }

#ifdef DEBUG
  fprintf(stderr, "[SEARCH] Stub search: depth=%d, time_limit=%dms, moves=%d\n",
          max_depth, time_limit_ms, legal_moves.count);
#endif

  return result;
}
