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

// Accès aux tables Zobrist pour mise à jour incrémentale
extern uint64_t zobrist_pieces[2][6][64];
extern uint64_t zobrist_castling[16];
extern uint64_t zobrist_en_passant[64];
extern uint64_t zobrist_side_to_move;

#endif // ZOBRIST_H
