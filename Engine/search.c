#include "search.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

// Variables globales pour gérer le temps de recherche
static clock_t search_start_time;
static int search_time_limit_ms;
static volatile int search_should_stop;

// V3: Table de transposition globale
static TranspositionTable tt_global;

// ========== INITIALISATION DU MOTEUR ==========

void initialize_engine(void) {
  DEBUG_LOG("=== INITIALISATION DU MOTEUR (V1) ===\n");
  init_zobrist();
  init_killer_moves(); // Inclus pour l'instant, mais non utilisé en V1
  init_lmr_table();    // Inclus pour l'instant, mais non utilisé en V1
  tt_init(&tt_global); // V3
  DEBUG_LOG("=== MOTEUR PRÊT ===\n\n");
}

// ========== NEGAMAX (V1: Alpha-Beta + Quiescence) ==========

int negamax_alpha_beta(Board *board, int depth, int alpha, int beta,
                       Couleur color, int ply, int in_null_move) {
  // (in_null_move est ignoré en V1)
  (void)in_null_move;

  // Vérifier le temps tous les 2048 noeuds
  static int node_count = 0;
  if (++node_count >= 2048) {
    node_count = 0;
    if (search_time_limit_ms > 0) {
      clock_t current = clock();
      int elapsed_ms = (int)(((double)(current - search_start_time)) /
                             CLOCKS_PER_SEC * 1000);
      if (elapsed_ms >= search_time_limit_ms) {
        search_should_stop = 1;
      }
    }
  }

  if (search_should_stop) {
    return 0;
  }

  // V3: Transposition Table Probe
  uint64_t hash = zobrist_hash(board);
  TTEntry *tt_entry = tt_probe(&tt_global, hash);
  if (tt_entry != NULL && tt_entry->depth >= depth) {
    if (tt_entry->type == TT_EXACT) {
      return tt_entry->score;
    } else if (tt_entry->type == TT_LOWERBOUND) {
      if (tt_entry->score >= beta) {
        return tt_entry->score;
      }
    } else if (tt_entry->type == TT_UPPERBOUND) {
      if (tt_entry->score <= alpha) {
        return tt_entry->score;
      }
    }
  }

  if (ply >= 128) {
    int eval = evaluate_position(board);
#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] ply=%d depth=%d eval=%d (static) color=%s\n", ply, depth, eval, color == WHITE ? "WHITE" : "BLACK");
#endif
    return eval;
  }

  if (depth == 0) {
    int eval = quiescence_search(board, alpha, beta, color, ply);
#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] ply=%d depth=%d eval=%d (quiescence) color=%s\n", ply, depth, eval, color == WHITE ? "WHITE" : "BLACK");
#endif
    return eval;
  }

  // V5: Reverse Futility Pruning
  if (depth <= 3 && !is_in_check(board, color)) {
    int static_eval = evaluate_position(board);
    int rfp_margin = 100 * depth;
    if (static_eval - rfp_margin >= beta) {
#ifdef DEBUG
      DEBUG_LOG("[NEGAMAX] RFP prune at ply=%d, static_eval=%d, margin=%d, beta=%d\n", ply, static_eval, rfp_margin, beta);
#endif
      return static_eval - rfp_margin; // Cutoff
    }
  }

  // V6: Null Move Pruning
  if (depth >= 3 && !in_null_move && !is_in_check(board, color) &&
      has_non_pawn_material(board, color)) {
    // "Jouer" le coup nul
    Board backup = *board;
    board->to_move = (color == WHITE) ? BLACK : WHITE;
    board->en_passant = -1;

    int R = 2; // Réduction
    int null_score =
        -negamax_alpha_beta(board, depth - 1 - R, -beta, -beta + 1,
                            (color == WHITE) ? BLACK : WHITE, ply + 1, 1);

    *board = backup; // Restaure l'état

#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] Null move prune? score=%d beta=%d ply=%d\n", null_score, beta, ply);
#endif
    if (null_score >= beta) {
      return beta; // Pruning
    }
  }

  MoveList moves;
  generate_legal_moves(board, &moves);

  if (moves.count == 0) {
    if (is_in_check(board, color)) {
      return -MATE_SCORE + ply; // Mat
    } else {
      return STALEMATE_SCORE; // Pat
    }
  }

  // ========== FIX #1: VALIDATION DU HASH MOVE ==========
  Move hash_move = {0};
  int hash_move_valid = 0;

  // V3: Récupérer le meilleur coup de la TT pour le move ordering
  if (tt_entry != NULL) {
    Move candidate = tt_entry->best_move;

    // ✅ VALIDER que le coup est dans la liste des coups légaux
    for (int i = 0; i < moves.count; i++) {
      if (moves.moves[i].from == candidate.from &&
          moves.moves[i].to == candidate.to &&
          moves.moves[i].type == candidate.type &&
          // Pour les promotions, vérifier aussi la pièce promue
          (candidate.type != MOVE_PROMOTION ||
           moves.moves[i].promotion == candidate.promotion)) {
        hash_move = moves.moves[i]; // ✅ Coup complet validé
        hash_move_valid = 1;
#ifdef DEBUG
        DEBUG_LOG("[TT] Hash move VALIDÉ: %s\n", move_to_string(&hash_move));
#endif
        break;
      }
    }

#ifdef DEBUG
    if (!hash_move_valid && (candidate.from != 0 || candidate.to != 0)) {
      DEBUG_LOG("[TT] Hash move REJETÉ (illégal): %s\n",
                move_to_string(&candidate));
    }
#endif
  }

  // V2: Move Ordering (utiliser hash_move seulement s'il est validé)
  OrderedMoveList ordered_moves;
  order_moves(board, &moves, &ordered_moves,
              hash_move_valid ? hash_move : (Move){0}, ply);

  int max_score = -INFINITY_SCORE;
  Move best_move = {0};
  int alpha_orig = alpha;

  int static_eval_for_futility = -INFINITY_SCORE;
  int futility_pruning_active = (depth <= 2 && !is_in_check(board, color));
  if (futility_pruning_active) {
    static_eval_for_futility = evaluate_position(board);
  }

  for (int i = 0; i < ordered_moves.count; i++) {
    // V10: Futility Pruning
    if (futility_pruning_active && is_quiet_move(&ordered_moves.moves[i])) {
      int futility_margin = 150 * depth;
      if (static_eval_for_futility + futility_margin < alpha) {
        continue; // Prune ce coup
      }
    }

    apply_move(board, &ordered_moves.moves[i], ply);
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int score;

    // V4: Principal Variation Search (PVS)
    if (i == 0) {
      // Premier coup (PV-node), recherche avec une fenêtre complète
      score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                  ply + 1, 0);
    } else {
      // V7: Late Move Reductions (LMR)
      int reduction = 0;
      if (depth >= 3 && i >= 4 && is_quiet_move(&ordered_moves.moves[i])) {
        reduction = get_lmr_reduction(depth, i);
      }

      // Recherche avec fenêtre nulle, potentiellement réduite
      score = -negamax_alpha_beta(board, depth - 1 - reduction, -alpha - 1,
                                  -alpha, opponent, ply + 1, 0);

      // Si la recherche réduite a battu alpha, il faut re-chercher à la
      // profondeur normale
      if (reduction > 0 && score > alpha) {
        score = -negamax_alpha_beta(board, depth - 1, -alpha - 1, -alpha,
                                    opponent, ply + 1, 0);
      }

      // Si la recherche (réduite ou non) est prometteuse, faire une recherche
      // complète (PVS)
      if (score > alpha && score < beta) {
        score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                    ply + 1, 0);
      }
    }

    undo_move(board, ply);

#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] ply=%d move=%s score=%d color=%s\n", ply, move_to_string(&ordered_moves.moves[i]), score, color == WHITE ? "WHITE" : "BLACK");
#endif

    if (score > max_score) {
      max_score = score;
      best_move = ordered_moves.moves[i];
    }

    if (max_score > alpha) {
      alpha = max_score;
    }

    if (alpha >= beta) {
      // Beta cutoff (fail-high)
      if (is_quiet_move(&best_move)) {
        update_history(best_move, depth, color);
        store_killer_move(best_move, ply);
      }
      tt_store(&tt_global, hash, depth, beta, TT_LOWERBOUND, best_move);
#ifdef DEBUG
      DEBUG_LOG("[NEGAMAX] ply=%d beta cutoff move=%s score=%d\n", ply, move_to_string(&best_move), beta);
#endif
      return beta;
    }
  }

  // V3: Transposition Table Store
  TTEntryType tt_type;
  if (max_score <= alpha_orig) {
    tt_type = TT_UPPERBOUND;
  } else if (max_score >= beta) {
    tt_type = TT_LOWERBOUND;
  } else {
    tt_type = TT_EXACT;
  }
  tt_store(&tt_global, hash, depth, max_score, tt_type, best_move);

  return max_score;
}

// ========== RECHERCHE ITÉRATIVE (Iterative Deepening) ==========

SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms) {
  search_start_time = clock();
  search_time_limit_ms = time_limit_ms;
  search_should_stop = 0;

  tt_new_search(&tt_global); // V3

  SearchResult best_result = {0};
  best_result.score = -INFINITY_SCORE;
  best_result.nodes_searched = 0;

  // ========== FIX #2: INITIALISATION SÉCURISÉE ==========
  Move best_move_overall;
  best_move_overall.from = -1; // ✅ Marqueur invalide
  best_move_overall.to = -1;
  int best_score_overall = -INFINITY_SCORE;

  for (int current_depth = 1; current_depth <= max_depth; current_depth++) {
    MoveList moves;
    generate_legal_moves(board, &moves);
    if (moves.count == 0)
      break;

    OrderedMoveList ordered_moves;
    order_moves(board, &moves, &ordered_moves, (Move){0}, 0);

    Move best_move_this_iter = ordered_moves.moves[0];
    int best_score_this_iter = -INFINITY_SCORE;
    long nodes_this_iter = 0;

    // ✅ Sauvegarder le joueur à la racine
    Couleur root_player = board->to_move;

    for (int i = 0; i < ordered_moves.count; i++) {
      apply_move(board, &ordered_moves.moves[i], 0);

      // La couleur à passer à negamax doit être la couleur qui est maintenant
      // à jouer après avoir appliqué le coup (board->to_move).
      Couleur color_for_negamax = board->to_move;

      // negamax retourne l'évaluation du point de vue de color_for_negamax,
      // on prend ensuite le négatif pour obtenir l'évaluation du joueur racine.
      int score = -negamax_alpha_beta(board, current_depth - 1, -INFINITY_SCORE,
                                      INFINITY_SCORE, color_for_negamax, 1, 0);

#ifdef DEBUG
      DEBUG_LOG("[ITERATIVE] depth=%d move=%s score=%d root_player=%s\n", current_depth, move_to_string(&ordered_moves.moves[i]), score, root_player == WHITE ? "WHITE" : "BLACK");
#endif

      nodes_this_iter++;
      undo_move(board, 0);

      if (search_should_stop)
        break;

      if (score > best_score_this_iter) {
        best_score_this_iter = score;
        best_move_this_iter = ordered_moves.moves[i];
      }
    }

    if (search_should_stop && best_move_overall.from == -1) {
      best_move_overall = best_move_this_iter;
      best_score_overall = best_score_this_iter;
      break;
    } else if (search_should_stop) {
      break;
    }

    best_move_overall = best_move_this_iter;
    best_score_overall = best_score_this_iter;
    best_result.nodes_searched += nodes_this_iter;

    clock_t end_time = clock();
    int elapsed_ms =
        (int)(((double)(end_time - search_start_time)) / CLOCKS_PER_SEC * 1000);
    if (elapsed_ms == 0)
      elapsed_ms = 1;
    int nps = (int)(best_result.nodes_searched * 1000 / elapsed_ms);

    // ✅ Normalisation du score pour UCI (toujours du point de vue BLANC)
    best_result.score = (root_player == WHITE) ? best_score_overall : -best_score_overall;

    printf("info depth %d score cp %d nodes %d nps %d time %d pv %s\n",
           current_depth, best_result.score, best_result.nodes_searched, nps,
           elapsed_ms, move_to_string(&best_move_overall));
    fflush(stdout);

    if (abs(best_score_overall) >= MATE_SCORE - 100) {
      break;
    }
  }

  // ✅ Vérification finale : coup valide ?
  if (best_move_overall.from == -1 || best_move_overall.to == -1) {
    // Fallback d'urgence : prendre le premier coup légal
    MoveList emergency_moves;
    generate_legal_moves(board, &emergency_moves);
    if (emergency_moves.count > 0) {
      best_move_overall = emergency_moves.moves[0];
      DEBUG_LOG("[EMERGENCY] Aucun coup trouvé, fallback vers %s\n",
                move_to_string(&best_move_overall));
    }
  }

  best_result.best_move = best_move_overall;
  // On conserve la normalisation du score pour UCI
  // best_result.score déjà mis à jour dans la boucle
  best_result.depth = max_depth;

  return best_result;
}

// Wrapper pour l'ancienne API
SearchResult search_best_move(Board *board, int depth) {
  return search_iterative_deepening(board, depth, 0);
}