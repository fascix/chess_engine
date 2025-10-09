#include "perft.h"
#include <stdio.h>
#include <string.h> // Pour memset si besoin

// Fonction perft : compte toutes les positions légales jusqu'à depth
unsigned long perft(Board *board, int depth) {
  if (depth == 0)
    return 1;

  unsigned long nodes = 0;
  MoveList moves;
  generate_legal_moves(board, &moves); // Générer seulement les coups légaux

  for (int i = 0; i < moves.count; i++) {
    Board backup = *board; // Sauvegarder le board
    make_move_temp(board, &moves.moves[i], &backup);
    nodes += perft(board, depth - 1);
    *board = backup; // Revenir en arrière
  }

  return nodes;
}

// Fonction perft_divide : affiche le nombre de positions pour chaque coup à
// depth-1
void perft_divide(Board *board, int depth) {
  MoveList moves;
  generate_legal_moves(board, &moves);

  for (int i = 0; i < moves.count; i++) {
    Board backup = *board;
    make_move_temp(board, &moves.moves[i], &backup);
    unsigned long count = perft(board, depth - 1);
    *board = backup;

    char *move_str = move_to_string(&moves.moves[i]);
    printf("%s: %lu\n", move_str, count);
  }
}