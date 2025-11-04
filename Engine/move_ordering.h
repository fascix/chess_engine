#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "board.h"
#include "movegen.h"

// Structure pour les coups ordonnés avec leurs scores
typedef struct {
  Move moves[256];
  int scores[256];
  int count;
} OrderedMoveList;

// ========== FONCTIONS PRINCIPALES ==========

// Ordonne les coups pour optimiser l'alpha-beta
void order_moves(const Board *board, MoveList *moves, OrderedMoveList *ordered,
                 Move hash_move, int ply);

// Initialise les tables de killer moves et history
void init_killer_moves(void);

// ========== KILLER MOVES ==========

// Stocke un killer move
void store_killer_move(Move move, int ply);

// Vérifie si c'est un killer move
int is_killer_move(Move move, int ply);

// ========== HISTORY HEURISTIC ==========

// Met à jour l'historique pour un coup qui a causé une coupure
void update_history(Move move, int depth, Couleur color);

// ========== MVV-LVA ==========

// Score MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
int mvv_lva_score(const Move *move);

#endif // MOVE_ORDERING_H
