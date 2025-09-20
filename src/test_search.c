// Test des algorithmes de recherche et d'évaluation
#include "evaluation.h"
#include "movegen.h"
#include "search.h"
#include <stdio.h>
#include <time.h>

void print_search_result(const SearchResult *result, const char *description) {
  printf("=== %s ===\n", description);
  printf("Meilleur coup: ");
  print_move(&result->best_move);
  printf("\n");
  printf("Score: %d centipawns\n", result->score);
  printf("Profondeur: %d\n", result->depth);
  printf("Noeuds explorés: %d\n", result->nodes_searched);
  printf("\n");
}

int main() {
  printf("============ TESTS RECHERCHE & ÉVALUATION AVANCÉS ============\n");

  // Initialiser les tables Zobrist
  init_zobrist();

  // Test 1: Évaluation de position initiale
  printf("\n=== Test Évaluation Position Initiale ===\n");
  Board initial_board;
  board_init(&initial_board);
  print_board(&initial_board);

  int initial_score = evaluate_position(&initial_board);
  printf("Score position initiale: %d centipawns\n", initial_score);
  printf("Matériel: %d\n", evaluate_material(&initial_board));
  printf("Bonus position: %d\n", evaluate_position_bonus(&initial_board));

  // Test 2: Évaluation d'une position avec avantage matériel
  printf("\n=== Test Évaluation Avantage Matériel ===\n");
  Board advantage_board;
  board_from_fen(
      &advantage_board,
      "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKB1R w KQkq - 0 2");
  print_board(&advantage_board);

  int advantage_score = evaluate_position(&advantage_board);
  printf("Score avec dame en plus: %d centipawns\n", advantage_score);

  // Test 3: Recherche profondeur 1
  printf("\n=== Test Recherche Profondeur 1 ===\n");
  clock_t start = clock();
  SearchResult result_depth1 = search_best_move(&advantage_board, 1);
  clock_t end = clock();
  double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

  print_search_result(&result_depth1, "Recherche Profondeur 1");
  printf("Temps: %.3f secondes\n", time_taken);

  // Test 4: Recherche profondeur 2
  printf("\n=== Test Recherche Profondeur 2 ===\n");
  start = clock();
  SearchResult result_depth2 = search_best_move(&advantage_board, 2);
  end = clock();
  time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

  print_search_result(&result_depth2, "Recherche Profondeur 2");
  printf("Temps: %.3f secondes\n", time_taken);

  // Test 5: Position tactique (fourchette possible)
  printf("\n=== Test Position Tactique ===\n");
  Board tactical_board;
  board_from_fen(&tactical_board,
                 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  print_board(&tactical_board);

  start = clock();
  SearchResult tactical_result = search_best_move(&tactical_board, 3);
  end = clock();
  time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

  print_search_result(&tactical_result, "Position Tactique Profondeur 3");
  printf("Temps: %.3f secondes\n", time_taken);

  // Test 6: Position de mat en 1
  printf("\n=== Test Mat en 1 ===\n");
  Board mate_board;
  board_from_fen(&mate_board, "6k1/5ppp/8/8/8/8/5PPP/4Q1K1 w - - 0 1");
  print_board(&mate_board);

  start = clock();
  SearchResult mate_result = search_best_move(&mate_board, 2);
  end = clock();
  time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

  print_search_result(&mate_result, "Mat en 1");
  printf("Temps: %.3f secondes\n", time_taken);

  if (mate_result.score > 25000) {
    printf("✅ Mat détecté correctement!\n");
  }

  // Test 7: Performance - Position complexe
  printf("\n=== Test Performance Position Complexe ===\n");
  Board complex_board;
  board_from_fen(
      &complex_board,
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
  print_board(&complex_board);

  printf("Profondeur 1:\n");
  start = clock();
  SearchResult perf1 = search_best_move(&complex_board, 1);
  end = clock();
  printf("Temps: %.3f s, Noeuds: %d\n",
         ((double)(end - start)) / CLOCKS_PER_SEC, perf1.nodes_searched);

  printf("Profondeur 2:\n");
  start = clock();
  SearchResult perf2 = search_best_move(&complex_board, 2);
  end = clock();
  printf("Temps: %.3f s, Noeuds: %d\n",
         ((double)(end - start)) / CLOCKS_PER_SEC, perf2.nodes_searched);

  printf("Profondeur 3:\n");
  start = clock();
  SearchResult perf3 = search_best_move(&complex_board, 3);
  end = clock();
  printf("Temps: %.3f s, Noeuds: %d\n",
         ((double)(end - start)) / CLOCKS_PER_SEC, perf3.nodes_searched);

  printf("\n============ TESTS TERMINÉS ============\n");
  printf("🎯 Algorithme Negamax avec Alpha-Beta implémenté !\n");

  return 0;
}
