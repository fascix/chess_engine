#ifndef PERFT_H
#define PERFT_H

#include "board.h"
#include "movegen.h"

// Compte toutes les positions légales jusqu'à depth
unsigned long perft(Board *board, int depth);

// Affiche le nombre de positions pour chaque coup à depth-1
void perft_divide(Board *board, int depth);

#endif // PERFT_H