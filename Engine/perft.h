#ifndef PERFT_H
#define PERFT_H

#include "board.h"
#include "movegen.h"

// Variable globale pour activer les traces de debug
extern int perft_debug;

// Compte toutes les positions légales jusqu'à depth
// Optimisée avec bulk counting à depth 1
unsigned long perft(Board *board, int depth);

// Affiche le nombre de positions pour chaque coup (sans stats)
void perft_divide(Board *board, int depth);

// Test complet avec statistiques (temps, NPS, nodes)
void perft_test(Board *board, int depth);

#endif // PERFT_H
