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

// Default version if not specified
#ifndef VERSION
#define VERSION 10
#endif

// Variables globales pour gérer le temps de recherche
static clock_t search_start_time;
static int search_time_limit_ms;
static volatile int search_should_stop;
static long global_nodes_searched; // Global counter for all nodes explored

#if VERSION >= 3
// V3: Table de transposition globale
static TranspositionTable tt_global;
#endif

// ========== INITIALISATION DU MOTEUR ==========

void initialize_engine(void) {
  DEBUG_LOG("=== INITIALISATION DU MOTEUR (V%d) ===\n", VERSION);
  init_zobrist();
#if VERSION >= 9
  init_killer_moves(); // V9: Killer Moves
#endif
#if VERSION >= 7
  init_lmr_table();    // V7: Late Move Reductions
#endif
#if VERSION >= 3
  tt_init(&tt_global); // V3: Transposition Table
#endif
  DEBUG_LOG("=== MOTEUR PRÊT ===\n\n");
}

// ========== NEGAMAX (V1: Alpha-Beta + Quiescence) ==========

int negamax_alpha_beta(Board *board, int depth, int alpha, int beta,
                       Couleur color, int ply, int in_null_move) {
  // Increment global node counter
  global_nodes_searched++;

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

  // Variables needed for TT (used conditionally)
#if VERSION >= 3
  uint64_t hash = zobrist_hash(board);
  int tt_score;
  TTEntry *tt_entry = tt_probe(&tt_global, hash, ply, &tt_score);
#else
  uint64_t hash = 0;
  TTEntry *tt_entry = NULL;
  (void)hash; // Suppress unused warning
  (void)tt_entry; // Suppress unused warning
#endif

#if VERSION >= 3
  // V3: Transposition Table Probe
  if (tt_entry != NULL && tt_entry->depth >= depth) {
    if (tt_entry->type == TT_EXACT) {
      return tt_score;
    } else if (tt_entry->type == TT_LOWERBOUND) {
      if (tt_score >= beta) {
        return tt_score;
      }
      // Update alpha if we have a better lower bound
      if (tt_score > alpha) {
        alpha = tt_score;
      }
    } else if (tt_entry->type == TT_UPPERBOUND) {
      if (tt_score <= alpha) {
        return tt_score;
      }
      // Update beta if we have a better upper bound
      if (tt_score < beta) {
        beta = tt_score;
      }
    }
  }
#endif

  if (ply >= 128) {
    int eval = evaluate_position(board);
    // evaluate_position returns from white's perspective, adjust for current
    // player
    if (color == BLACK)
      eval = -eval;
#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] ply=%d depth=%d eval=%d (static) color=%s\n", ply,
              depth, eval, color == WHITE ? "WHITE" : "BLACK");
#endif
    return eval;
  }

  if (depth == 0) {
    int eval = quiescence_search(board, alpha, beta, color, ply);
#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] ply=%d depth=%d eval=%d (quiescence) color=%s\n", ply,
              depth, eval, color == WHITE ? "WHITE" : "BLACK");
#endif
    return eval;
  }

#if VERSION >= 5
  // V5: Reverse Futility Pruning
  if (depth <= 2 && !is_in_check(board, color)) {
    int static_eval = evaluate_position(board);
    // evaluate_position returns from white's perspective, adjust for current
    // player
    if (color == BLACK)
      static_eval = -static_eval;
    int rfp_margin = 150 * depth;
    if (static_eval - rfp_margin >= beta) {
#ifdef DEBUG
      DEBUG_LOG(
          "[NEGAMAX] RFP prune at ply=%d, static_eval=%d, margin=%d, beta=%d\n",
          ply, static_eval, rfp_margin, beta);
#endif
      return static_eval - rfp_margin; // Cutoff
    }
  }
#endif

#if VERSION >= 6
  // V6: Null Move Pruning
  if (depth >= 3 && !in_null_move && !is_in_check(board, color) &&
      has_non_pawn_material(board, color)) {
    // "Jouer" le coup nul
    Board backup = *board;
    board->to_move = (color == WHITE) ? BLACK : WHITE;
    board->en_passant = -1;

    // Réduction R : adaptative selon la profondeur
    // R=3 pour profondeur >= 6 (position stable, peut réduire plus)
    // R=2 pour profondeur < 6 (éviter les erreurs tactiques)
    int R = (depth >= 6) ? 3 : 2;
    int null_score =
        -negamax_alpha_beta(board, depth - 1 - R, -beta, -beta + 1,
                            (color == WHITE) ? BLACK : WHITE, ply + 1, 1);

    *board = backup; // Restaure l'état

#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] Null move prune? score=%d beta=%d ply=%d\n",
              null_score, beta, ply);
#endif
    if (null_score >= beta) {
      return beta; // Pruning
    }
  }
#endif

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

#if VERSION >= 3
  // V3: Récupérer le meilleur coup de la TT pour le move ordering
  if (tt_entry != NULL) {
    Move candidate = tt_entry->best_move;

    // ✅ VALIDER d'abord que le coup a des cases valides (0-63 pour un échiquier 8x8)
    if (candidate.from >= 0 && candidate.from < 64 && candidate.to >= 0 &&
        candidate.to < 64) {
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
          DEBUG_LOG("[TT] Hash move VALIDÉ: %s\n",
                    move_to_string(&hash_move));
#endif
          break;
        }
      }
    }

#ifdef DEBUG
    if (!hash_move_valid && (candidate.from != 0 || candidate.to != 0)) {
      DEBUG_LOG("[TT] Hash move REJETÉ (illégal): from=%d to=%d\n",
                candidate.from, candidate.to);
    }
#endif
  }
#endif

#if VERSION >= 2
  // V2: Move Ordering (utiliser hash_move seulement s'il est validé)
  OrderedMoveList ordered_moves;
  order_moves(board, &moves, &ordered_moves,
              hash_move_valid ? hash_move : (Move){0}, ply);
#else
  // V1: Pas d'ordonnancement, utiliser l'ordre de génération
  OrderedMoveList ordered_moves;
  ordered_moves.count = moves.count;
  for (int i = 0; i < moves.count; i++) {
    ordered_moves.moves[i] = moves.moves[i];
    ordered_moves.scores[i] = 0;
  }
#endif

  int max_score = -INFINITY_SCORE;
  Move best_move = {0};
  int alpha_orig = alpha;

#if VERSION >= 10
  int static_eval_for_futility = -INFINITY_SCORE;
  int futility_pruning_active = (depth <= 2 && !is_in_check(board, color));
  if (futility_pruning_active) {
    static_eval_for_futility = evaluate_position(board);
    // evaluate_position returns from white's perspective, adjust for current
    // player
    if (color == BLACK)
      static_eval_for_futility = -static_eval_for_futility;
  }
#endif

  for (int i = 0; i < ordered_moves.count; i++) {
#if VERSION >= 10
    // V10: Futility Pruning
    if (futility_pruning_active && is_quiet_move(&ordered_moves.moves[i])) {
      int futility_margin = 200 * depth;
      if (static_eval_for_futility + futility_margin < alpha) {
        continue; // Prune ce coup
      }
    }
#endif

    apply_move(board, &ordered_moves.moves[i], ply);
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int score;

#if VERSION >= 4
    // V4: Principal Variation Search (PVS)
    if (i == 0) {
      // Premier coup (PV-node), recherche avec une fenêtre complète
      score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                  ply + 1, 0);
    } else {
#if VERSION >= 7
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
#else
      // Sans LMR: recherche directe avec fenêtre nulle
      score = -negamax_alpha_beta(board, depth - 1, -alpha - 1, -alpha,
                                  opponent, ply + 1, 0);
#endif

      // Si la recherche (réduite ou non) est prometteuse, faire une recherche
      // complète (PVS)
      if (score > alpha && score < beta) {
        score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                    ply + 1, 0);
      }
    }
#else
    // V1-V3: Recherche standard sans PVS
    score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                ply + 1, 0);
#endif

    undo_move(board, ply);

#ifdef DEBUG
    DEBUG_LOG("[NEGAMAX] ply=%d move=%s score=%d color=%s\n", ply,
              move_to_string(&ordered_moves.moves[i]), score,
              color == WHITE ? "WHITE" : "BLACK");
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
#if VERSION >= 8
      if (is_quiet_move(&best_move)) {
        update_history(best_move, depth, color);
#if VERSION >= 9
        store_killer_move(best_move, ply);
#endif
      }
#endif
#if VERSION >= 3
      tt_store(&tt_global, hash, depth, beta, TT_LOWERBOUND, best_move, ply);
#endif
#ifdef DEBUG
      DEBUG_LOG("[NEGAMAX] ply=%d beta cutoff move=%s score=%d\n", ply,
                move_to_string(&best_move), beta);
#endif
      return beta;
    }
  }

#if VERSION >= 3
  // V3: Transposition Table Store
  TTEntryType tt_type;
  if (max_score <= alpha_orig) {
    tt_type = TT_UPPERBOUND;
  } else if (max_score >= beta) {
    tt_type = TT_LOWERBOUND;
  } else {
    tt_type = TT_EXACT;
  }
  tt_store(&tt_global, hash, depth, max_score, tt_type, best_move, ply);
#endif

  return max_score;
}

// ========== RECHERCHE ITÉRATIVE (Iterative Deepening) ==========

SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms) {
  search_start_time = clock();
  search_time_limit_ms = time_limit_ms;
  search_should_stop = 0;
  global_nodes_searched = 0; // Reset global counter

#if VERSION >= 3
  tt_new_search(&tt_global); // V3
#endif

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
      DEBUG_LOG("[ITERATIVE] depth=%d move=%s score=%d root_player=%s\n",
                current_depth, move_to_string(&ordered_moves.moves[i]), score,
                root_player == WHITE ? "WHITE" : "BLACK");
#endif

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
    best_result.nodes_searched = global_nodes_searched; // Use global counter

    clock_t end_time = clock();
    int elapsed_ms =
        (int)(((double)(end_time - search_start_time)) / CLOCKS_PER_SEC * 1000);
    if (elapsed_ms == 0)
      elapsed_ms = 1;
    int nps = (int)(best_result.nodes_searched * 1000 / elapsed_ms);

    // ✅ Normalisation du score pour UCI (toujours du point de vue BLANC)
    best_result.score =
        (root_player == WHITE) ? best_score_overall : -best_score_overall;

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