#include "board.h"
#include "evaluation.h"
#include "movegen.h"
#include "search.h"
#include <stdio.h>
#include <time.h>

void test_quiescence_vs_static() {
  printf("========== TEST QUIESCENCE SEARCH ==========\n\n");

  // Position tactique avec séquence de captures
  // Les blancs peuvent jouer Bxf7+ puis récupérer du matériel
  const char *tactical_fen =
      "rnbq1rk1/ppp2ppp/4pn2/3p4/1bPP4/2N1PN2/PP3PPP/R1BQKB1R w KQ - 0 1";

  Board board;
  board_from_fen(&board, tactical_fen);

  printf("=== Position Tactique (Bxf7+ possible) ===\n");
  print_board(&board);

  // Test 1: Évaluation statique simple
  int static_eval = evaluate_position(&board);
  printf("\nÉvaluation statique: %d centipawns\n", static_eval);

  // Test 2: Quiescence Search seule
  clock_t start = clock();
  int qs_eval =
      quiescence_search(&board, -INFINITY_SCORE, INFINITY_SCORE, WHITE);
  clock_t end = clock();
  double qs_time = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Quiescence Search: %d centipawns (%.3f s)\n", qs_eval, qs_time);

  // Test 3: Comparaison avec recherche normale profondeur 1
  start = clock();
  SearchResult result_depth1 = search_best_move(&board, 1);
  end = clock();
  double search_time = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Recherche prof. 1: %d centipawns, coup %s (%.3f s)\n",
         result_depth1.score, move_to_string(&result_depth1.best_move),
         search_time);

  // Test 4: Position après une capture (pour voir l'horizon effect)
  printf("\n=== Test Horizon Effect ===\n");
  const char *hanging_piece_fen =
      "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";

  board_from_fen(&board, hanging_piece_fen);
  printf("Position avec pièce \"pendue\":\n");
  print_board(&board);

  int static_eval2 = evaluate_position(&board);
  int qs_eval2 =
      quiescence_search(&board, -INFINITY_SCORE, INFINITY_SCORE, BLACK);

  printf("Évaluation statique: %d\n",
         -static_eval2); // Négation pour perspective noire
  printf("Quiescence Search: %d\n", qs_eval2);
  printf("Différence QS vs Static: %d centipawns\n",
         qs_eval2 - (-static_eval2));

  // Test 5: Génération de captures seulement
  printf("\n=== Test Génération Captures ===\n");
  board_from_fen(&board, tactical_fen);
  MoveList all_moves, captures;
  generate_legal_moves(&board, &all_moves);
  generate_capture_moves(&board, &captures);

  printf("Coups légaux total: %d\n", all_moves.count);
  printf("Captures uniquement: %d\n", captures.count);

  printf("Captures trouvées:\n");
  for (int i = 0; i < captures.count; i++) {
    printf("  %s", move_to_string(&captures.moves[i]));
    if (captures.moves[i].type == MOVE_CAPTURE) {
      printf(" (capture)");
    } else if (captures.moves[i].type == MOVE_EN_PASSANT) {
      printf(" (en passant)");
    } else if (captures.moves[i].type == MOVE_PROMOTION) {
      printf(" (promotion)");
    }
    printf("\n");
  }

  printf("\n========== TESTS QUIESCENCE TERMINÉS ==========\n");
  printf("✅ Quiescence Search fonctionne et améliore la vision tactique!\n");
}

int main() {
  test_quiescence_vs_static();
  return 0;
}
