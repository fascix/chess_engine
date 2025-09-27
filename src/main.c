#include "board.h"
#include "movegen.h"
#include "perft.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Main complet pour lancer un perft interactif
int main(int argc, char **argv) {
  Board board;
  board_init(&board);

  int depth = 6;
  if (argc > 1) {
    depth = atoi(argv[1]);
  }

  printf("Position initiale :\n");
  print_board(&board);

  // --- AJOUT : Afficher tous les coups légaux à depth 1 ---
  MoveList moves;
  generate_legal_moves(&board, &moves);
  printf("\nCoups légaux à depth 1 (%d coups) :\n", moves.count);
  for (int i = 0; i < moves.count; i++) {
    char *move_str = move_to_string(&moves.moves[i]);
    Couleur color = board.to_move;
    printf("%2d. %s (%s)\n", i + 1, move_str,
           (color == WHITE) ? "Blanc" : "Noir");
  }
  // ----------------------------------------------------------

  printf("\nPerft à profondeur %d...\n", depth);
  unsigned long total = perft(&board, depth);
  printf("Noeuds total à profondeur %d : %lu\n", depth, total);

  printf("\nPerft divide à profondeur %d :\n", depth);
  perft_divide(&board, depth);

  return 0;
}