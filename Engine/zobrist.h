#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "board.h"
#include <stdint.h>

// Initialise les tables Zobrist (à appeler une seule fois au démarrage)
void init_zobrist(void);

// Calcule le hash Zobrist d'une position donnée
uint64_t zobrist_hash(const Board *board);

// Fonctions pour obtenir les clés Zobrist individuelles (pour l'update
// incrémental)
uint64_t zobrist_get_piece_key(Couleur color, PieceType piece, Square square);
uint64_t zobrist_get_castling_key(int castle_rights);
uint64_t zobrist_get_en_passant_key(Square square);
uint64_t zobrist_get_side_key(void);

// Test de validation de l'unicité des hash (debug)
void test_zobrist_uniqueness(void);

#endif // ZOBRIST_H