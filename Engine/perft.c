#include "perft.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Variable globale pour activer les traces de debug
int perft_debug = 0;

// Fonction perft : compte toutes les positions légales jusqu'à depth
// Optimisée avec bulk counting à depth 1
unsigned long perft(Board *board, int depth) {
  if (depth == 0)
    return 1;

  MoveList moves;
  generate_legal_moves(board, &moves);

#ifdef DEBUG
  if (perft_debug) {
    printf("[PERFT depth=%d] Generated %d legal moves\n", depth, moves.count);
    for (int i = 0; i < moves.count && i < 10; i++) {
      printf("  Move %d: ", i);
      print_move(&moves.moves[i]);
      printf("\n");
    }
    if (moves.count > 10) {
      printf("  ... (%d more moves)\n", moves.count - 10);
    }
  }
#endif

  // OPTIMISATION: À depth 1, juste retourner le nombre de coups légaux
  // Pas besoin de les jouer, on sait qu'ils sont tous légaux
  if (depth == 1)
    return moves.count;

  unsigned long nodes = 0;
  for (int i = 0; i < moves.count; i++) {
    Board
        backup; // Déclaré mais NON initialisé - sera rempli par make_move_temp
    make_move_temp(board, &moves.moves[i], &backup);
    nodes += perft(board, depth - 1);
    *board = backup; // Restaure avec le backup créé par make_move_temp
  }

  return nodes;
}

// Fonction perft_divide : affiche le nombre de positions pour chaque coup
void perft_divide(Board *board, int depth) {
  MoveList moves;
  generate_legal_moves(board, &moves);

  for (int i = 0; i < moves.count; i++) {
    Board backup;
    make_move_temp(board, &moves.moves[i], &backup);
    unsigned long count = perft(board, depth - 1);
    *board = backup;

    char *move_str = move_to_string(&moves.moves[i]);
    printf("%s: %lu\n", move_str, count);
  }
}

// Fonction perft_test : test complet avec statistiques de performance
void perft_test(Board *board, int depth) {
  printf("\n=== PERFT TEST ===\n");
  printf("Depth: %d\n\n", depth);

  clock_t start = clock();

  MoveList moves;
  generate_legal_moves(board, &moves);
  unsigned long total = 0;

  // Afficher chaque coup avec son nombre de nodes
  for (int i = 0; i < moves.count; i++) {
    Board backup;
    make_move_temp(board, &moves.moves[i], &backup);
    unsigned long count = perft(board, depth - 1);
    *board = backup;

    char *move_str = move_to_string(&moves.moves[i]);
    printf("%-6s: %lu\n", move_str, count);
    total += count;
  }

  clock_t end = clock();
  double elapsed_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

  // Statistiques finales
  printf("\n=== RESULTS ===\n");
  printf("Nodes: %lu\n", total);
  printf("Time:  %.0f ms\n", elapsed_ms);
  if (elapsed_ms > 0) {
    printf("NPS:   %.0f\n", total / (elapsed_ms / 1000.0));
  }
  printf("\n");
}