#ifndef SEARCH_HELPERS_H
#define SEARCH_HELPERS_H

#include "board.h"
#include "movegen.h"

// Scores spéciaux pour la recherche
#define MATE_SCORE 30000
#define STALEMATE_SCORE 0
#define INFINITY_SCORE 50000

// ========== GESTION DES COUPS ==========

// Applique temporairement un mouvement avec sauvegarde
void apply_move(Board *board, const Move *move, int ply);

// Annule un mouvement (restaure depuis backup)
void undo_move(Board *board, int ply);

// ========== HELPERS DE RECHERCHE ==========

// Vérifie si un coup est "quiet" (pas de capture, pas de promotion)
int is_quiet_move(const Move *move);

// ========== LMR (Late Move Reductions) ==========

// Initialise la table de réductions LMR
void init_lmr_table(void);

// Récupère la réduction LMR pour un depth et move_number donnés
int get_lmr_reduction(int depth, int move_number);

#endif // SEARCH_HELPERS_H
