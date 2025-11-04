#include "search_helpers.h"
#include <math.h>
#include <stdio.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

// ========== BACKUP STACK ==========

static Board search_backup_stack[128]; // Stack pour sauvegardes (par ply)

// ========== TABLE LMR ==========

static int lmr_reductions[64][64]; // [depth][move_number]

// ========== GESTION DES COUPS ==========

void apply_move(Board *board, const Move *move, int ply) {
  // Sauvegarder l'état AVANT modification
  search_backup_stack[ply] = *board;

  // Appliquer le mouvement
  Board dummy_backup; // Non utilisé dans make_move_temp
  make_move_temp(board, move, &dummy_backup);
}

void undo_move(Board *board, int ply) { *board = search_backup_stack[ply]; }

// ========== HELPERS DE RECHERCHE ==========

int is_quiet_move(const Move *move) {
  // Un coup est quiet s'il n'est pas :
  // - Une capture
  // - Une promotion
  // - Un en-passant (type spécial de capture)
  return (move->type != MOVE_CAPTURE && move->type != MOVE_PROMOTION &&
          move->type != MOVE_EN_PASSANT);
}

// ========== LMR ==========

void init_lmr_table(void) {
  for (int depth = 1; depth < 64; depth++) {
    for (int move_num = 1; move_num < 64; move_num++) {
      // Formule standard : R = log(depth) * log(move_num) / diviseur
      // diviseur = 2.5 (équilibre entre vitesse et précision)
      double reduction = log(depth) * log(move_num) / 2.5;
      lmr_reductions[depth][move_num] = (int)reduction;

      // Clamp entre 0 et depth-1
      if (lmr_reductions[depth][move_num] < 0) {
        lmr_reductions[depth][move_num] = 0;
      }
      if (lmr_reductions[depth][move_num] >= depth) {
        lmr_reductions[depth][move_num] = depth - 1;
      }
    }
  }
}

int get_lmr_reduction(int depth, int move_number) {
  // Clamp les indices
  if (depth < 0 || depth >= 64)
    return 0;
  if (move_number < 0 || move_number >= 64)
    return 0;

  return lmr_reductions[depth][move_number];
}
