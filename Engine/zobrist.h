#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "board.h"
#include <stdint.h>

// Initialise les tables Zobrist (à appeler une seule fois au démarrage)
void init_zobrist(void);

// Calcule le hash Zobrist d'une position donnée
uint64_t zobrist_hash(const Board *board);

// Test de validation de l'unicité des hash (debug)
void test_zobrist_uniqueness(void);

#endif // ZOBRIST_H
