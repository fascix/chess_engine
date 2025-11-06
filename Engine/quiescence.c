#include "quiescence.h"
#include "evaluation.h"
#include "move_ordering.h"
#include "utils.h"
#include <stdio.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

// ========== GÉNÉRATION DES CAPTURES ==========

void generate_capture_moves(const Board *board, MoveList *moves) {
  MoveList all_moves;
  generate_legal_moves(board, &all_moves);

  movelist_init(moves);

  for (int i = 0; i < all_moves.count; i++) {
    if (all_moves.moves[i].type == MOVE_CAPTURE ||
        all_moves.moves[i].type == MOVE_EN_PASSANT ||
        all_moves.moves[i].type == MOVE_PROMOTION) {
      movelist_add(moves, all_moves.moves[i]);
    }
  }
}

// ========== QUIESCENCE SEARCH ==========

int quiescence_search(Board *board, int alpha, int beta, Couleur color,
                      int ply) {
  return quiescence_search_depth(board, alpha, beta, color, ply);
}

int quiescence_search_depth(Board *board, int alpha, int beta, Couleur color,
                            int ply) {
  // Limite de profondeur pour éviter les boucles infinies
  if (ply >= 128) { // Sécurité maximale
    int score = evaluate_position(board);
#ifdef DEBUG
    DEBUG_LOG("[QUIESCENCE] ply>=128, eval=%d\n", score);
#endif
    return score;
  }

  // Évaluation statique
  int stand_pat = evaluate_position(board);

  // Beta cutoff
  if (stand_pat >= beta) {
#ifdef DEBUG
    DEBUG_LOG("[QUIESCENCE] stand_pat=%d >= beta=%d, cutoff\n", stand_pat, beta);
#endif
    return beta;
  }

  // Alpha improvement
  if (stand_pat > alpha) {
    alpha = stand_pat;
  }

  // Générer uniquement les captures
  MoveList capture_moves;
  generate_capture_moves(board, &capture_moves);

  // Pas de captures = position quiète
  if (capture_moves.count == 0) {
#ifdef DEBUG
    DEBUG_LOG("[QUIESCENCE] Quiet position, stand_pat=%d\n", stand_pat);
#endif
    return stand_pat;
  }

  // Trier les captures par MVV-LVA
  OrderedMoveList ordered_captures;
  Move null_move = {.from = -1,
                    .to = -1,
                    .type = MOVE_NORMAL,
                    .promotion = EMPTY,
                    .captured_piece = EMPTY};
  order_moves(board, &capture_moves, &ordered_captures, null_move, ply);

  // Chercher dans les captures
  for (int i = 0; i < ordered_captures.count; i++) {
    // FIX: Utiliser une sauvegarde locale au lieu du stack partagé
    // Cela évite la corruption quand quiescence_search est appelé depuis
    // negamax
    Board local_backup = *board;

    // Appliquer le mouvement directement
    Board dummy_backup;
    make_move_temp(board, &ordered_captures.moves[i], &dummy_backup);

    // Delta pruning - ignorer les captures très faibles
    int delta = piece_value(ordered_captures.moves[i].captured_piece) + 200;
    if (stand_pat + delta < alpha) {
      *board = local_backup; // Restaurer depuis le backup local
#ifdef DEBUG
      DEBUG_LOG("[QUIESCENCE] Delta prune: stand_pat=%d delta=%d alpha=%d\n", stand_pat, delta, alpha);
#endif
      continue;
    }

    // Recherche récursive
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int score =
        -quiescence_search_depth(board, -beta, -alpha, opponent, ply + 1);

    // Restaurer le plateau depuis le backup local
    *board = local_backup;

#ifdef DEBUG
    DEBUG_LOG("[QUIESCENCE] ply=%d move=%s score=%d\n", ply, move_to_string(&ordered_captures.moves[i]), score);
#endif

    // Mise à jour alpha-beta
    if (score >= beta) {
#ifdef DEBUG
      DEBUG_LOG("[QUIESCENCE] Beta cutoff: score=%d >= beta=%d\n", score, beta);
#endif
      return beta; // Beta cutoff
    }

    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}
