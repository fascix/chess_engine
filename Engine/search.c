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

// Fonction de recherche itérative deepening (stub)
SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms) {
  // TODO: Implémenter la recherche réelle
  // Pour l'instant, retourner simplement le premier coup légal

  SearchResult result;
  memset(&result, 0, sizeof(result));

  MoveList legal_moves;
  generate_legal_moves(board, &legal_moves);

  if (legal_moves.count > 0) {
    result.best_move = legal_moves.moves[0];
    result.depth = 1;
    result.score = 0;
    result.nodes = legal_moves.count;
    result.nps = 0;
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
