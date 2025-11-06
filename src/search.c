#include "search.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

// Forward declaration for move ordering
void order_moves(const Board *board, MoveList *moves, OrderedMoveList *ordered,
                 Move hash_move, int ply);

// Table de transposition globale
static TranspositionTable tt_global;

// Tables Zobrist pour le hachage
static uint64_t zobrist_pieces[2][6][64];
static uint64_t zobrist_castling[16];
static uint64_t zobrist_en_passant[64];
static uint64_t zobrist_side_to_move;

// Tables pour Move Ordering
static Move killer_moves[128][2];     // [ply][killer_slot]
static int history_scores[2][64][64]; // [color][from][to]

static Board search_backup_stack[128]; // Stack pour sauvegardes (par ply)

// ========== TABLE DE RÉDUCTIONS LMR ==========
// Formule : reduction = log(depth) * log(move_number) / 2.5
// Pré-calculée pour éviter les appels à log() pendant la recherche
static int lmr_reductions[64][64]; // [depth][move_number]

// Initialise la table de réductions LMR (appelée une fois au démarrage)
void init_lmr_table() {
  for (int depth = 1; depth < 64; depth++) {
    for (int move_num = 1; move_num < 64; move_num++) {
      // Formule standard : R = log(depth) * log(move_num) / diviseur
      // diviseur = 2.0 (améliore la réduction, cherche plus de coups)
      double reduction = log(depth) * log(move_num) / 2.0;
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

// Applique temporairement un mouvement avec sauvegarde correcte
void apply_move(Board *board, const Move *move, int ply) {
  // Sauvegarder l'état AVANT modification
  search_backup_stack[ply] = *board;

  // Appliquer le mouvement
  Board dummy_backup; // Non utilisé dans make_move_temp
  make_move_temp(board, move, &dummy_backup);
}

// Annule un mouvement (restaure depuis backup)
void undo_move(Board *board, int ply) { *board = search_backup_stack[ply]; }

// ========== HELPER POUR NULL MOVE PRUNING ==========

// Vérifie si le joueur a du matériel autre que des pions
// (pour éviter les problèmes de zugzwang en finale de pions)
int has_non_pawn_material(const Board *board, Couleur color) {
  // On vérifie s'il y a des pièces autres que pions et roi
  if (board->pieces[color][KNIGHT] != 0)
    return 1; // Cavaliers
  if (board->pieces[color][BISHOP] != 0)
    return 1; // Fous
  if (board->pieces[color][ROOK] != 0)
    return 1; // Tours
  if (board->pieces[color][QUEEN] != 0)
    return 1; // Dames

  return 0; // Seulement roi + pions (ou seulement roi)
}

// Vérifie si un coup est "quiet" (pas de capture, pas de promotion, pas
// d'échec) Utilisé pour LMR et Futility Pruning
int is_quiet_move(const Move *move) {
  // Un coup est quiet s'il n'est pas :
  // - Une capture
  // - Une promotion
  // - Un en-passant (type spécial de capture)
  return (move->type != MOVE_CAPTURE && move->type != MOVE_PROMOTION &&
          move->type != MOVE_EN_PASSANT);
}

// ========== NEGAMAX AVEC ALPHA-BETA, PVS ET NULL MOVE PRUNING ==========

// Negamax avec Alpha-Beta et PVS (Principal Variation Search)
// PVS: recherche le 1er coup avec fenêtre complète [-beta, -alpha],
//      puis les suivants avec null window [-alpha-1, -alpha] pour détecter
//      rapidement les coups qui n'améliorent pas alpha (avec re-recherche si
//      nécessaire)
//
// NOUVEAU: in_null_move = 1 si on vient de faire un null move (évite double
// NMP)
int negamax_alpha_beta(Board *board, int depth, int alpha, int beta,
                       Couleur color, int ply, int in_null_move) {
  // Vérifier le temps tous les 1024 noeuds (pour ne pas trop ralentir)
  static int node_count = 0;
  if (++node_count >= 1024) {
    node_count = 0;
    clock_t current = clock();
    int elapsed =
        (int)(((double)(current - search_start_time)) / CLOCKS_PER_SEC * 1000);
    if (elapsed >= search_time_limit_ms) {
      search_should_stop = 1;
      return 0; // Retourner immédiatement
    }
  }

  // Si on doit arrêter, retourner immédiatement
  if (search_should_stop) {
    return 0;
  }

  // Sécurité: ply trop grand
  if (ply >= 128) {
    DEBUG_LOG("ERROR: ply=%d trop grand, retour score neutre\n", ply);
    return 0; // score neutre
  }
  // Table de transposition : probe au début
  TTEntry *entry = tt_probe(&tt_global, zobrist_hash(board));
  if (entry != NULL && entry->depth >= depth) {
#ifdef DEBUG
    if (ply <= 3) {
      static int tt_hit_count = 0;
      if (tt_hit_count++ < 10) {
        DEBUG_LOG("          [TT_HIT] ply=%d depth=%d score=%d (from TT)\n",
                  ply, depth, entry->score);
      }
    }
#endif
    return entry->score;
  }
  if (depth == 0) {
    int eval = quiescence_search(board, alpha, beta, color, ply);
    int static_eval = evaluate_position(board);
    static_eval = (color == WHITE) ? static_eval : -static_eval;
#ifdef DEBUG
    if (ply == 3) { // Log au ply 3 (profondeur 0)
      static int qs_count = 0;
      if (qs_count++ < 5) { // Log les 5 premières fois
        DEBUG_LOG(
            "          [QS] ply=%d eval=%d static_eval=%d alpha=%d beta=%d\n",
            ply, eval, static_eval, alpha, beta);
      }
    }
#endif
    return eval;
  }

  // ========== NULL MOVE PRUNING ==========
  // Idée : Si même en donnant un coup gratuit à l'adversaire (= passer notre
  // tour),
  //        notre position reste >= beta, alors on peut couper cette branche.
  //        L'adversaire trouvera forcément mieux avec un vrai coup.
  //
  // Conditions de sécurité CRITIQUES :
  // 1. depth >= 3 : Éviter les erreurs tactiques à faible profondeur
  // 2. !in_null_move : Pas de double null move consécutif
  // 3. !is_in_check : Interdit en échec (passer son tour serait illégal)
  // 4. beta < MATE_SCORE : Ne pas utiliser dans séquence de mat
  // 5. has_non_pawn_material : Désactiver en finale de pions (zugzwang)

  if (depth >= 3 &&                          // Profondeur suffisante
      !in_null_move &&                       // Pas déjà en null move
      !is_in_check(board, color) &&          // Pas en échec
      beta < MATE_SCORE &&                   // Pas dans séquence de mat
      has_non_pawn_material(board, color)) { // Pas finale de pions seuls

    // Sauvegarder l'état actuel
    Couleur old_to_move = board->to_move;
    int old_en_passant = board->en_passant;

    // "Jouer" le coup null : inverser le trait (= passer notre tour)
    board->to_move = (color == WHITE) ? BLACK : WHITE;
    board->en_passant = -1; // Annuler l'en-passant (un coup a été "joué")

    // Réduction R : adaptative selon la profondeur
    // R=3 pour profondeur >= 6 (position stable, peut réduire plus)
    // R=2 pour profondeur < 6 (éviter les erreurs tactiques)
    int R = (depth >= 6) ? 3 : 2;

    // Recherche avec profondeur réduite et fenêtre null [-beta, -beta+1]
    // On teste juste si score >= beta (fenêtre nulle pour aller vite)
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int null_score =
        -negamax_alpha_beta(board, depth - R - 1, -beta, -beta + 1, opponent,
                            ply + 1, 1); // 1 = in_null_move

    // Restaurer l'état du plateau
    board->to_move = old_to_move;
    board->en_passant = old_en_passant;

    // Si la recherche null échoue haut (null_score >= beta)
    // → Même en donnant un coup gratuit, notre position est trop bonne
    // → Cutoff ! L'adversaire ne choisira jamais cette branche
    if (null_score >= beta) {
#ifdef DEBUG
      if (ply <= 2) {
        static int nmp_cutoff_count = 0;
        if (nmp_cutoff_count++ < 5) {
          DEBUG_LOG("[NMP_CUTOFF] ply=%d depth=%d null_score=%d >= beta=%d\n",
                    ply, depth, null_score, beta);
        }
      }
#endif
      return beta; // Cutoff (cohérence avec fail-high classique)
    }
  }
  // Fin du Null Move Pruning

  // ========== REVERSE FUTILITY PRUNING (Static Null Move Pruning) ==========
  // Idée : Si notre évaluation statique est TROP BONNE (bien au-dessus de
  // beta),
  //        on peut couper directement sans chercher les coups.
  //        L'adversaire ne choisira jamais cette branche.
  //
  // Conditions :
  // - depth <= 2 : SEULEMENT à très faible profondeur (moins agressif)
  // - !is_in_check : Pas en échec (les tactiques peuvent renverser la
  // situation)
  // - eval - margin >= beta : Notre position est bien meilleure que beta

  if (depth <= 2 && !is_in_check(board, color)) {
    int static_eval = evaluate_position(board);
    static_eval = (color == WHITE) ? static_eval : -static_eval;

    // Marge augmentée pour être moins agressif
    int rfp_margin = 150 * depth; // 150 centipawns par ply (au lieu de 100)

    if (static_eval - rfp_margin >= beta) {
#ifdef DEBUG
      if (ply <= 2) {
        static int rfp_cutoff_count = 0;
        if (rfp_cutoff_count++ < 5) {
          DEBUG_LOG("[RFP_CUTOFF] ply=%d depth=%d eval=%d margin=%d beta=%d\n",
                    ply, depth, static_eval, rfp_margin, beta);
        }
      }
#endif
      return static_eval - rfp_margin; // Cutoff conservateur
    }
  }
  // Fin du Reverse Futility Pruning

  MoveList moves;
  generate_legal_moves(board, &moves);

  // Limiter la taille de moves pour OrderedMoveList à 256 coups max
  if (moves.count > 256) {
    DEBUG_LOG("WARNING: moves.count=%d > 256, tronqué à 256 coups\n",
              moves.count);
    moves.count = 256;
  }

  if (moves.count == 0) {
    if (is_in_check(board, color)) {
      return -MATE_SCORE + ply; // Plus ply est petit, plus c'est mauvais
    } else {
      return STALEMATE_SCORE;
    }
  }

  int max_score = -INFINITY_SCORE;
  Move best_move = {0};
  OrderedMoveList ordered_moves;
  Move hash_move = {0};
  int hash_move_valid = 0;

  // Récupérer et VALIDER le hash_move de la TT si disponible
  if (entry != NULL && entry->best_move.from != 0) {
    Move candidate = entry->best_move;

    // VALIDATION CRITIQUE : Vérifier que la pièce appartient à la bonne couleur
    Couleur piece_color = get_piece_color(board, candidate.from);
    if (piece_color != color) {
#ifdef DEBUG
      if (ply <= 2) {
        DEBUG_LOG("[TT] ⚠️ Hash move REJETÉ (mauvaise couleur): %c%d%c%d "
                  "piece_color=%d expected=%d (ply=%d)\n",
                  'a' + (candidate.from % 8), 1 + (candidate.from / 8),
                  'a' + (candidate.to % 8), 1 + (candidate.to / 8), piece_color,
                  color, ply);
      }
#endif
    } else {
      // Vérifier que le coup est dans la liste des coups légaux
      for (int i = 0; i < moves.count; i++) {
        if (moves.moves[i].from == candidate.from &&
            moves.moves[i].to == candidate.to &&
            moves.moves[i].type == candidate.type &&
            (candidate.type != MOVE_PROMOTION ||
             moves.moves[i].promotion == candidate.promotion)) {
          hash_move = moves.moves[i];
          hash_move_valid = 1;
#ifdef DEBUG
          if (ply <= 2) {
            DEBUG_LOG("[TT] Hash move validé: %c%d%c%d (ply=%d)\n",
                      'a' + (hash_move.from % 8), 1 + (hash_move.from / 8),
                      'a' + (hash_move.to % 8), 1 + (hash_move.to / 8), ply);
          }
#endif
          break;
        }
      }

#ifdef DEBUG
      if (!hash_move_valid && ply <= 2) {
        DEBUG_LOG("[TT] ⚠️ Hash move REJETÉ (illégal): %c%d%c%d (ply=%d)\n",
                  'a' + (candidate.from % 8), 1 + (candidate.from / 8),
                  'a' + (candidate.to % 8), 1 + (candidate.to / 8), ply);
      }
#endif
    }
  }

  // Utiliser le hash_move seulement s'il est valide
  Move invalid_move = {.from = -1,
                       .to = -1,
                       .type = MOVE_NORMAL,
                       .promotion = EMPTY,
                       .captured_piece = EMPTY};
  order_moves(board, &moves, &ordered_moves,
              hash_move_valid ? hash_move : invalid_move, ply);
  // Sécurité OrderedMoveList: limiter à 256
  if (ordered_moves.count > 256) {
    DEBUG_LOG("WARNING: ordered_moves.count=%d > 256, tronqué à 256 coups\n",
              ordered_moves.count);
    ordered_moves.count = 256;
  }

  // Pré-calculer l'évaluation statique pour Futility Pruning
  int static_eval_for_futility = -1;
  int futility_pruning_active = 0;

  // CORRECTION: Futility seulement à depth <= 2 (au lieu de 4)
  if (depth <= 2 && !is_in_check(board, color)) {
    static_eval_for_futility = evaluate_position(board);
    static_eval_for_futility =
        (color == WHITE) ? static_eval_for_futility : -static_eval_for_futility;
    futility_pruning_active = 1;
  }

  for (int i = 0; i < ordered_moves.count; i++) {
    // Sécurité avant apply_move
    if (ply >= 128) {
      DEBUG_LOG("ERROR: ply=%d trop grand avant apply_move, skip coup\n", ply);
      continue;
    }

    // ========== FUTILITY PRUNING ==========
    // Idée : Si notre position est TROP MAUVAISE (eval + marge < alpha),
    //        et que le coup est "quiet", on peut l'ignorer car il ne peut
    //        probablement pas améliorer alpha.
    //
    // Conditions :
    // - depth <= 2 : Seulement à très faible profondeur (moins agressif)
    // - !is_in_check : Pas en échec
    // - Le coup est quiet (pas de capture/promotion)
    // - static_eval + margin < alpha : Position trop mauvaise

    if (futility_pruning_active && i >= 1 && // Au moins essayer le premier coup
        is_quiet_move(&ordered_moves.moves[i])) {

      // Marge optimiste augmentée pour être moins agressif
      int futility_margin =
          200 * depth; // 200 centipawns par ply (augmenté de 150)

      if (static_eval_for_futility + futility_margin < alpha) {
#ifdef DEBUG
        if (ply <= 2 && i < 10) {
          static int futility_skip_count = 0;
          if (futility_skip_count++ < 5) {
            DEBUG_LOG("[FUTILITY_SKIP] ply=%d depth=%d i=%d eval=%d margin=%d "
                      "alpha=%d\n",
                      ply, depth, i, static_eval_for_futility, futility_margin,
                      alpha);
          }
        }
#endif
        continue; // Ignorer ce coup quiet
      }
    }
    // Fin du Futility Pruning

    apply_move(board, &ordered_moves.moves[i], ply);

    // ========== RECHERCHE RÉCURSIVE AVEC PVS + LMR ==========
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int score;

    if (i == 0) {
      // ===== Premier coup (PV node) : recherche complète =====
      score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                  ply + 1, 0);
    } else {
      // ===== Coups suivants : PVS + LMR =====

      // Calculer la réduction LMR
      int reduction = 0;

      // Conditions LMR :
      // 1. depth >= 3 : Profondeur suffisante
      // 2. i >= 4 : Au moins 4 coups déjà explorés (les meilleurs)
      // 3. Le coup est "quiet" (pas de capture, promotion, échec)
      // 4. Pas en échec (on veut réduire seulement en positions calmes)

      if (depth >= 3 && i >= 4 && is_quiet_move(&ordered_moves.moves[i]) &&
          !is_in_check(board, color)) {

        // Utiliser la table pré-calculée (clampée à 63 max)
        int d_idx = (depth < 64) ? depth : 63;
        int m_idx = (i < 64) ? i : 63;
        reduction = lmr_reductions[d_idx][m_idx];

        // Bonus : Réduire encore plus si le coup a un mauvais score
        // d'historique (histoire heuristique négative = coup qui échoue
        // souvent)
        if (history_scores[color][ordered_moves.moves[i].from]
                          [ordered_moves.moves[i].to] < 0) {
          reduction++; // Réduction supplémentaire
        }

        // Clamp la réduction pour ne pas descendre à depth <= 0
        if (reduction >= depth - 1) {
          reduction = depth - 2;
        }
        if (reduction < 0) {
          reduction = 0;
        }
      }

      // Étape 1 : Recherche avec réduction (si LMR applicable)
      if (reduction > 0) {
        // Recherche réduite avec fenêtre null
        score = -negamax_alpha_beta(board, depth - 1 - reduction, -alpha - 1,
                                    -alpha, opponent, ply + 1, 0);
      } else {
        // Pas de réduction : recherche normale avec fenêtre null (PVS standard)
        score = -negamax_alpha_beta(board, depth - 1, -alpha - 1, -alpha,
                                    opponent, ply + 1, 0);
      }

      // Étape 2 : Re-recherche si le coup réduit surprend (score > alpha)
      if (reduction > 0 && score > alpha) {
        // Le coup réduit est meilleur que prévu ! Re-rechercher sans réduction
        score = -negamax_alpha_beta(board, depth - 1, -alpha - 1, -alpha,
                                    opponent, ply + 1, 0);
      }

      // Étape 3 : Re-recherche PVS si score améliore alpha (PV node potentiel)
      if (score > alpha && score < beta) {
        // Re-search avec fenêtre complète pour obtenir le score exact
        score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                    ply + 1, 0);
      }
    }

    undo_move(board, ply);

#ifdef DEBUG
    if (ply == 1 && i < 5) { // Log les 5 premiers coups au ply 1
      DEBUG_LOG("      [ply=%d i=%d] coup %c%d%c%d -> score=%d (max_score=%d, "
                "alpha=%d, beta=%d)\n",
                ply, i, 'a' + (ordered_moves.moves[i].from % 8),
                1 + (ordered_moves.moves[i].from / 8),
                'a' + (ordered_moves.moves[i].to % 8),
                1 + (ordered_moves.moves[i].to / 8), score, max_score, alpha,
                beta);
    }
    if (ply == 2 && i < 3) { // Log les 3 premiers coups au ply 2
      DEBUG_LOG(
          "        [ply=%d i=%d] score=%d (max_score=%d, alpha=%d, beta=%d)\n",
          ply, i, score, max_score, alpha, beta);
    }
#endif

    if (score > max_score) {
      max_score = score;
      best_move = ordered_moves.moves[i];
    }

    if (score >= beta) {
      store_killer_move(ordered_moves.moves[i], ply);
      update_history(ordered_moves.moves[i], depth, color);
      // Stocker dans la TT (fail-high)
#ifdef DEBUG
      if (ply == 2) {
        static int tt_store_count = 0;
        if (tt_store_count++ < 3) {
          DEBUG_LOG(
              "          [TT_STORE] ply=%d depth=%d score=%d (beta cutoff)\n",
              ply, depth, beta);
        }
      }
#endif
      tt_store(&tt_global, zobrist_hash(board), depth, beta, TT_LOWERBOUND,
               ordered_moves.moves[i]);
      return beta;
    }

    if (score > alpha) {
      alpha = score;
    }
  }
  // Stocker dans la TT (exact si max_score > alpha initial, sinon upperbound)
  TTEntryType ttype = (max_score <= alpha) ? TT_UPPERBOUND : TT_EXACT;
#ifdef DEBUG
  if (ply == 2) {
    static int tt_store_final_count = 0;
    if (tt_store_final_count++ < 5) {
      DEBUG_LOG("          [TT_STORE] ply=%d depth=%d max_score=%d type=%s\n",
                ply, depth, max_score,
                ttype == TT_EXACT ? "EXACT" : "UPPERBOUND");
    }
  }
#endif
  tt_store(&tt_global, zobrist_hash(board), depth, max_score, ttype, best_move);

#ifdef DEBUG
  if (ply == 1) {
    DEBUG_LOG(
        "    [NEGAMAX] ply=%d depth=%d returns max_score=%d (moves_count=%d)\n",
        ply, depth, max_score, ordered_moves.count);
  }
#endif

  return max_score;
}

// ========== TRANSPOSITION TABLE ==========

// Initialise la table de transposition
void tt_init(TranspositionTable *tt) {
  memset(tt, 0, sizeof(TranspositionTable));
  tt->current_age = 1;

#ifdef DEBUG
  DEBUG_LOG("TT initialisée : %zu entrées (%zu bytes)\n", (size_t)TT_SIZE,
            sizeof(TranspositionTable));
#endif
}

// Stocke une entrée dans la table de transposition
void tt_store(TranspositionTable *tt, uint64_t key, int depth, int score,
              TTEntryType type, Move best_move) {
  uint32_t index = key & TT_MASK;
  TTEntry *entry = &tt->entries[index];

  // VALIDATION : key ne doit JAMAIS être 0
  if (key == 0) {
#ifdef DEBUG
    static int zero_key_warnings = 0;
    if (zero_key_warnings++ < 3) {
      DEBUG_LOG("WARNING: tt_store() appelé avec key=0 !\n");
    }
#endif
    return; // NE PAS STOCKER
  }

  // Stratégie de remplacement améliorée :
  // 1. Case vide (key == 0)
  // 2. Même position (key == entry->key)
  // 3. Profondeur supérieure (depth > entry->depth)
  // 4. Entrée obsolète (age < current_age - 2)

  int should_replace = 0;

  if (entry->key == 0) {
    should_replace = 1; // Case vide
  } else if (entry->key == key) {
    should_replace = 1; // Même position (mise à jour)
  } else if (depth > entry->depth) {
    should_replace = 1; // Profondeur supérieure (info plus précise)
  } else if (entry->age < tt->current_age - 2) {
    should_replace = 1; // Entrée trop ancienne (2 recherches en arrière)
  }

  if (should_replace) {
    entry->key = key;
    entry->depth = depth;
    entry->score = score;
    entry->type = type;
    entry->best_move = best_move;
    entry->age = tt->current_age;

#ifdef DEBUG
    static int store_count = 0;
    if (store_count++ < 10) {
      DEBUG_LOG("TT_STORE: index=%u key=%016llx depth=%d score=%d\n", index,
                (unsigned long long)key, depth, score);
    }
#endif
  }
}

// Sonde la table de transposition
TTEntry *tt_probe(TranspositionTable *tt, uint64_t key) {
  // VALIDATION : key ne doit JAMAIS être 0
  if (key == 0) {
#ifdef DEBUG
    static int zero_key_probes = 0;
    if (zero_key_probes++ < 3) {
      DEBUG_LOG("WARNING: tt_probe() appelé avec key=0 !\n");
    }
#endif
    return NULL;
  }

  uint32_t index = key & TT_MASK;
  TTEntry *entry = &tt->entries[index];

  // Vérifier que la clé match ET que l'entrée n'est pas vide
  if (entry->key == key && entry->depth > 0) {
#ifdef DEBUG
    static int hit_count = 0;
    if (hit_count++ < 10) {
      DEBUG_LOG("TT_HIT: index=%u key=%016llx depth=%d score=%d\n", index,
                (unsigned long long)key, entry->depth, entry->score);
    }
#endif
    return entry;
  }

  return NULL; // Pas trouvé
}

// Nouvelle recherche (incrément de l'age)
void tt_new_search(TranspositionTable *tt) {
  tt->current_age++;
  if (tt->current_age == 0)
    tt->current_age = 1; // Éviter overflow
}

// ========== ZOBRIST HASHING ==========

// Générateur de nombres pseudo-aléatoires amélioré (Xorshift*)
static uint64_t random_uint64() {
  static uint64_t seed = 0;

  // Initialiser le seed UNE SEULE FOIS avec timestamp + PID
  if (seed == 0) {
    seed = (uint64_t)time(NULL) ^ ((uint64_t)getpid() << 16);

    // Warm-up du générateur (important !)
    for (int i = 0; i < 64; i++) {
      seed ^= seed >> 12;
      seed ^= seed << 25;
      seed ^= seed >> 27;
      seed *= 0x2545F4914F6CDD1DULL;
    }
  }

  // Xorshift* (excellent générateur rapide)
  seed ^= seed >> 12;
  seed ^= seed << 25;
  seed ^= seed >> 27;
  return seed * 0x2545F4914F6CDD1DULL;
}

// Initialise tout le moteur (Zobrist + TT + move ordering)
void initialize_engine() {
  DEBUG_LOG("=== INITIALISATION DU MOTEUR ===\n");

  // 1. Initialiser Zobrist EN PREMIER
  init_zobrist();
  DEBUG_LOG("✓ Tables Zobrist initialisées\n");

  // 2. Initialiser la TT
  tt_init(&tt_global);
  DEBUG_LOG("✓ Table de transposition initialisée\n");

  // 3. Initialiser killer moves et history
  init_killer_moves();
  DEBUG_LOG("✓ Tables de move ordering initialisées\n");

  // 4. Initialiser la table LMR
  init_lmr_table();
  DEBUG_LOG("✓ Table LMR initialisée\n");

  DEBUG_LOG("=== MOTEUR PRÊT ===\n\n");
}

// Test de validation de l'unicité des hash Zobrist
void test_zobrist_uniqueness() {
  DEBUG_LOG("\n=== TEST UNICITÉ ZOBRIST ===\n");

  // Test 1 : Positions différentes = hash différents
  Board b1, b2;
  board_init(&b1);
  board_init(&b2);

  uint64_t h1 = zobrist_hash(&b1);
  DEBUG_LOG("Position initiale : hash = %016llx\n", (unsigned long long)h1);

  // Bouger un pion
  Move m = {.from = E2, .to = E4, .type = MOVE_NORMAL};
  make_move_temp(&b2, &m, &b1);

  uint64_t h2 = zobrist_hash(&b2);
  DEBUG_LOG("Après e2e4      : hash = %016llx\n", (unsigned long long)h2);

  if (h1 == h2) {
    DEBUG_LOG("❌ ERREUR : Hash identiques pour positions différentes !\n");
  } else {
    DEBUG_LOG("✓ Hash différents (écart = %lld)\n",
              (long long)(h2 > h1 ? h2 - h1 : h1 - h2));
  }

  // Test 2 : Vérifier qu'aucun hash n'est 0
  if (h1 == 0 || h2 == 0) {
    DEBUG_LOG("❌ ERREUR : Hash égal à 0 détecté !\n");
  } else {
    DEBUG_LOG("✓ Aucun hash nul\n");
  }

  DEBUG_LOG("=== FIN TEST ===\n\n");
}

// Initialise les tables Zobrist avec validation
void init_zobrist() {
  // Initialiser les clés pour chaque pièce sur chaque case
  for (int color = 0; color < 2; color++) {
    for (int piece = 0; piece < 6; piece++) {
      for (int square = 0; square < 64; square++) {
        zobrist_pieces[color][piece][square] = random_uint64();
      }
    }
  }

  // Droits de roque
  for (int i = 0; i < 16; i++) {
    zobrist_castling[i] = random_uint64();
  }

  // En passant
  for (int i = 0; i < 64; i++) {
    zobrist_en_passant[i] = random_uint64();
  }

  // Joueur actuel
  zobrist_side_to_move = random_uint64();

// VALIDATION : vérifier qu'aucune clé n'est à 0
#ifdef DEBUG
  int zero_count = 0;
  for (int c = 0; c < 2; c++) {
    for (int p = 0; p < 6; p++) {
      for (int s = 0; s < 64; s++) {
        if (zobrist_pieces[c][p][s] == 0)
          zero_count++;
      }
    }
  }
  if (zero_count > 0) {
    DEBUG_LOG("WARNING: %d clés Zobrist sont à 0 !\n", zero_count);
  }
  DEBUG_LOG("Zobrist initialisé : %d clés non-nulles\n",
            2 * 6 * 64 + 16 + 64 + 1 - zero_count);
#endif
}

// Calcule le hash Zobrist d'une position (défensif)
uint64_t zobrist_hash(const Board *board) {
  if (!board) {
    DEBUG_LOG("zobrist_hash: null board pointer\n");
    return 0;
  }

  uint64_t hash = 0;

  // Hash des pièces (défensif: vérification bornes)
  for (int color = WHITE; color <= BLACK; color++) {
    for (int piece = PAWN; piece <= KING; piece++) {
      Bitboard pieces = board->pieces[color][piece];
      while (pieces) {
        int square = __builtin_ctzll(pieces);
        if (square < 0 || square >= 64) {
          DEBUG_LOG("zobrist_hash: invalid square=%d (color=%d piece=%d)\n",
                    square, color, piece);
          // clear lowest bit and continue defensively
          pieces &= pieces - 1;
          continue;
        }
        hash ^= zobrist_pieces[color][piece][square];
        pieces &= pieces - 1;
      }
    }
  }

  // Droits de roque (vérifier borne)
  if (board->castle_rights >= 0 && board->castle_rights < 16) {
    hash ^= zobrist_castling[board->castle_rights];
  } else {
    DEBUG_LOG("zobrist_hash: invalid castle_rights=%d\n", board->castle_rights);
  }

  // En passant (vérifier borne)
  if (board->en_passant >= 0 && board->en_passant < 64) {
    hash ^= zobrist_en_passant[board->en_passant];
  } else if (board->en_passant != -1) {
    // -1 signifie pas d'en-passant; autres valeurs sont suspectes
    DEBUG_LOG("zobrist_hash: invalid en_passant=%d\n", board->en_passant);
  }

  // Joueur actuel
  if (board->to_move == BLACK) {
    hash ^= zobrist_side_to_move;
  }

  return hash;
}

// ========== MOVE ORDERING ==========

// Initialise les tables de killer moves
void init_killer_moves() {
  memset(killer_moves, 0, sizeof(killer_moves));
  memset(history_scores, 0, sizeof(history_scores));
}

// Score MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
int mvv_lva_score(const Move *move) {
  if (move->type != MOVE_CAPTURE && move->type != MOVE_EN_PASSANT) {
    return 0;
  }

  // Valeurs des pièces pour MVV-LVA
  static const int piece_values[] = {100, 320, 330, 500, 900, 20000};

  int victim_value = piece_values[move->captured_piece];

  // Pour l'attaquant, approximation basée sur les promotions
  int attacker_value = 100; // Pion par défaut
  if (move->type == MOVE_PROMOTION) {
    attacker_value = 100; // C'est un pion qui promeut
  }

  // Score: valeur victime * 100 - valeur attaquant
  return victim_value * 100 - attacker_value;
}

// Stocke un killer move
void store_killer_move(Move move, int ply) {
  if (ply >= 128)
    return;

  // Ne stocker que les coups non-capture comme killer moves
  if (move.type == MOVE_CAPTURE || move.type == MOVE_EN_PASSANT) {
    return;
  }

  // Shift: killer[1] -> killer[0], nouveau -> killer[1]
  if (killer_moves[ply][0].from != move.from ||
      killer_moves[ply][0].to != move.to) {
    killer_moves[ply][1] = killer_moves[ply][0];
    killer_moves[ply][0] = move;
  }
}

// Vérifie si c'est un killer move
int is_killer_move(Move move, int ply) {
  if (ply >= 128)
    return 0;

  return ((killer_moves[ply][0].from == move.from &&
           killer_moves[ply][0].to == move.to) ||
          (killer_moves[ply][1].from == move.from &&
           killer_moves[ply][1].to == move.to));
}

// Met à jour l'historique pour un coup qui a causé une coupure
void update_history(Move move, int depth, Couleur color) {
  if (move.type == MOVE_CAPTURE || move.type == MOVE_EN_PASSANT) {
    return; // Pas d'historique pour les captures
  }

  history_scores[color][move.from][move.to] += depth * depth;

  // Éviter overflow
  if (history_scores[color][move.from][move.to] > 10000) {
    // Diviser tous les scores par 2
    for (int i = 0; i < 64; i++) {
      for (int j = 0; j < 64; j++) {
        history_scores[color][i][j] /= 2;
      }
    }
  }
}

// ========== QUIESCENCE SEARCH ==========

// Génère uniquement les captures
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

// Quiescence Search - recherche uniquement les coups "bruyants"
int quiescence_search(Board *board, int alpha, int beta, Couleur color,
                      int ply) {
  return quiescence_search_depth(board, alpha, beta, color, ply);
}

// Quiescence Search avec limite de profondeur
int quiescence_search_depth(Board *board, int alpha, int beta, Couleur color,
                            int ply) {
  // Limite de profondeur pour éviter les boucles infinies
  if (ply >= 128) { // Sécurité maximale
    int score = evaluate_position(board);
    return (color == WHITE) ? score : -score;
  }
  // Évaluation statique
  int stand_pat = evaluate_position(board);
  if (color == BLACK)
    stand_pat = -stand_pat;

  // Beta cutoff
  if (stand_pat >= beta) {
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
    int delta = piece_value(ordered_captures.moves[i].captured_piece) + 300;
    if (stand_pat + delta < alpha) {
      *board = local_backup; // Restaurer depuis le backup local
      continue;
    }

    // Recherche récursive
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int score =
        -quiescence_search_depth(board, -beta, -alpha, opponent, ply + 1);

    // Restaurer le plateau depuis le backup local
    *board = local_backup;

    // Mise à jour alpha-beta
    if (score >= beta) {
      return beta; // Beta cutoff
    }

    if (score > alpha) {
      alpha = score;
    }
  }

  return alpha;
}

// À ajouter/remplacer dans search.c

// Static Exchange Evaluation simplifiée
int see_capture(const Board *board, const Move *move) {
  if (move->type != MOVE_CAPTURE && move->type != MOVE_EN_PASSANT) {
    return 0;
  }

  Square to = move->to;
  int gain = piece_value(move->captured_piece);

  // Approximation simple : si l'attaquant peut être recapturé
  Couleur attacking_color = board->to_move;
  Couleur defending_color = (attacking_color == WHITE) ? BLACK : WHITE;

  // Si la case de destination est défendue par l'adversaire
  if (is_square_attacked(board, to, defending_color)) {
    int attacker_value = 0;

    // Déterminer la valeur de l'attaquant
    PieceType attacker = get_piece_type(board, move->from);
    attacker_value = piece_value(attacker);

    // Gain net approximatif = valeur_capturée - valeur_attaquant
    gain = gain - attacker_value;
  }

  return gain;
}

// Move ordering amélioré avec SEE
void order_moves(const Board *board, MoveList *moves, OrderedMoveList *ordered,
                 Move hash_move, int ply) {
  ordered->count = moves->count;

#ifdef DEBUG
  int debug_scores[256] = {0};
  char debug_reasons[256][32];
#endif

  for (int i = 0; i < moves->count; i++) {
    ordered->moves[i] = moves->moves[i];
    int score = 0;

#ifdef DEBUG
    strcpy(debug_reasons[i], "history");
#endif

    // 1. Hash move - priorité absolue (vérifier que hash_move est valide)
    if (hash_move.from >= 0 && hash_move.from == moves->moves[i].from &&
        hash_move.to == moves->moves[i].to) {
      score = 2000000;
#ifdef DEBUG
      strcpy(debug_reasons[i], "hash_move");
#endif
    }
    // 2. Bonnes captures (SEE > 0)
    else if (moves->moves[i].type == MOVE_CAPTURE ||
             moves->moves[i].type == MOVE_EN_PASSANT) {
      int see_score = see_capture(board, &moves->moves[i]);
      if (see_score > 0) {
        score = 1000000 + see_score;
#ifdef DEBUG
        strcpy(debug_reasons[i], "good_capture");
        debug_scores[i] = see_score;
#endif
      } else {
        // Mauvaises captures en dernier
        score = -100000 + see_score;
#ifdef DEBUG
        strcpy(debug_reasons[i], "bad_capture");
        debug_scores[i] = see_score;
#endif
      }
    }
    // 3. Promotions
    else if (moves->moves[i].type == MOVE_PROMOTION) {
      score = 900000 + piece_value(moves->moves[i].promotion);
#ifdef DEBUG
      strcpy(debug_reasons[i], "promotion");
#endif
    }
    // 4. Killer moves
    else if (is_killer_move(moves->moves[i], ply)) {
      score = 800000;
#ifdef DEBUG
      strcpy(debug_reasons[i], "killer");
#endif
    }
    // 5. Échecs (approximation)
    else if (gives_check(board, &moves->moves[i])) {
      score = 700000;
#ifdef DEBUG
      strcpy(debug_reasons[i], "check");
#endif
    }
    // 6. Coups qui développent vers le centre
    else if (moves_toward_center(board, &moves->moves[i])) {
      score = 50000;

      // BONUS: En ouverture, préférer développer les pièces plutôt que les
      // pions
      if (board->move_number <= 10) {
        PieceType piece = get_piece_type(board, moves->moves[i].from);
        if (piece == KNIGHT) {
          score += 15000; // Les cavaliers en premier
        } else if (piece == BISHOP) {
          score += 10000; // Puis les fous
        } else if (piece == PAWN) {
          // Bonus pour pions centraux (d4, e4, d5, e5)
          int to_file = moves->moves[i].to % 8;
          int to_rank = moves->moves[i].to / 8;
          if ((to_file == 3 || to_file == 4) &&
              (to_rank == 3 || to_rank == 4)) {
            score += 5000; // e4, d4, e5, d5
          } else {
            score -= 2000; // Pénalité pour autres pions
          }
        }
      }

#ifdef DEBUG
      strcpy(debug_reasons[i], "center");
#endif
    }
    // 7. History heuristic
    else {
      Couleur color = board->to_move;
      score = history_scores[color][moves->moves[i].from][moves->moves[i].to];
#ifdef DEBUG
      debug_scores[i] = score;
#endif
    }

    ordered->scores[i] = score;
  }

  // Tri par insertion (efficace pour petites listes)
  for (int i = 1; i < ordered->count; i++) {
    Move temp_move = ordered->moves[i];
    int temp_score = ordered->scores[i];
#ifdef DEBUG
    char temp_reason[32];
    strcpy(temp_reason, debug_reasons[i]);
    int temp_debug_score = debug_scores[i];
#endif
    int j = i - 1;

    while (j >= 0 && ordered->scores[j] < temp_score) {
      ordered->moves[j + 1] = ordered->moves[j];
      ordered->scores[j + 1] = ordered->scores[j];
#ifdef DEBUG
      strcpy(debug_reasons[j + 1], debug_reasons[j]);
      debug_scores[j + 1] = debug_scores[j];
#endif
      j--;
    }

    ordered->moves[j + 1] = temp_move;
    ordered->scores[j + 1] = temp_score;
#ifdef DEBUG
    strcpy(debug_reasons[j + 1], temp_reason);
    debug_scores[j + 1] = temp_debug_score;
#endif
  }

#ifdef DEBUG
  if (ply == 0 &&
      ordered->count <= 30) { // Log seulement au ply 0 et si pas trop de coups
    DEBUG_LOG("\n[ORDER_MOVES] Tri de %d coups (ply=%d):\n", ordered->count,
              ply);
    for (int i = 0; i < ordered->count && i < 10; i++) { // Top 10 coups
      DEBUG_LOG(
          "  %2d. %c%d%c%d score=%7d reason=%s\n", i + 1,
          'a' + (ordered->moves[i].from % 8), 1 + (ordered->moves[i].from / 8),
          'a' + (ordered->moves[i].to % 8), 1 + (ordered->moves[i].to / 8),
          ordered->scores[i], debug_reasons[i]);
    }
  }
#endif
}

// Fonction helper pour détecter les coups vers le centre
int moves_toward_center(const Board *board, const Move *move) {
  if (board->move_number > 10)
    return 0; // Après l'ouverture

  int to_file = move->to % 8;
  int to_rank = move->to / 8;

  // Cases centrales (d4, d5, e4, e5) et leurs alentours
  return (to_file >= 2 && to_file <= 5 && to_rank >= 2 && to_rank <= 5);
}

// Fonction helper pour détecter les échecs (approximation)
int gives_check(const Board *board, const Move *move) {
  // Test rapide : simuler le coup et vérifier l'échec
  Board temp_board, backup;
  temp_board = *board;

  make_move_temp(&temp_board, move, &backup);

  Couleur opponent = (board->to_move == WHITE) ? BLACK : WHITE;
  int check = is_in_check(&temp_board, opponent);

  return check;
}

// Dans search.c - Paramètres optimisés pour éviter les bourdes

// Recherche avec profondeur minimale pour éviter les bourdes
SearchResult search_best_move_with_min_depth(Board *board, int max_depth,
                                             int min_depth) {
  SearchResult result = {0};
  // Initialisation avec marqueur invalide EXPLICITE
  result.best_move.from = -1;
  result.best_move.to = -1;
  result.best_move.type = MOVE_NORMAL;
  result.best_move.promotion = EMPTY;
  result.best_move.captured_piece = EMPTY;
  result.score = -INFINITY_SCORE;

  MoveList moves;
  generate_legal_moves(board, &moves);

  if (moves.count == 0) {
    result.score =
        is_in_check(board, board->to_move) ? -MATE_SCORE : STALEMATE_SCORE;
    return result;
  }

  // SÉCURITÉ: Recherche minimum de 3 plies pour éviter les bourdes évidentes
  int search_depth = (max_depth < min_depth) ? min_depth : max_depth;

  Couleur color = board->to_move;

  DEBUG_LOG("\n=== RECHERCHE DU MEILLEUR COUP (profondeur=%d) ===\n",
            search_depth);
  DEBUG_LOG("Position: coup #%d, trait aux %s\n", board->move_number,
            color == WHITE ? "BLANCS" : "NOIRS");

  // Ordonner les coups avant de les évaluer
  OrderedMoveList ordered_moves;
  Move hash_move = {.from = -1,
                    .to = -1,
                    .type = MOVE_NORMAL,
                    .promotion = EMPTY,
                    .captured_piece = EMPTY};
  order_moves(board, &moves, &ordered_moves, hash_move, 0);

  DEBUG_LOG("\nÉvaluation de %d coups:\n", ordered_moves.count);

  for (int i = 0; i < ordered_moves.count; i++) {
    // Pré-filtrage des coups évidemment mauvais
    if (is_obviously_bad_move(board, &ordered_moves.moves[i])) {
      DEBUG_LOG("  %2d. %c%d%c%d [SKIP - coup évidemment mauvais]\n", i + 1,
                'a' + (ordered_moves.moves[i].from % 8),
                1 + (ordered_moves.moves[i].from / 8),
                'a' + (ordered_moves.moves[i].to % 8),
                1 + (ordered_moves.moves[i].to / 8));
      continue;
    }

    // Eval avant le coup
#ifdef DEBUG
    int eval_before = evaluate_position(board);
#endif

    apply_move(board, &ordered_moves.moves[i], 0);

#ifdef DEBUG
    int eval_after = evaluate_position(board);
    // Note: eval_after est du point de vue de l'opponent après le coup
#endif

    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    // in_null_move = 0 : on commence la recherche avec un vrai coup
    int score = -negamax_alpha_beta(board, search_depth - 1, -INFINITY_SCORE,
                                    INFINITY_SCORE, opponent, 1, 0);
    result.nodes_searched++;

    undo_move(board, 0);

    // LOG DÉTAILLÉ pour chaque coup
    DEBUG_LOG("  %2d. %c%d%c%d [ordre=%5d] eval_avant=%4d eval_après=%4d -> "
              "score=%6d",
              i + 1, 'a' + (ordered_moves.moves[i].from % 8),
              1 + (ordered_moves.moves[i].from / 8),
              'a' + (ordered_moves.moves[i].to % 8),
              1 + (ordered_moves.moves[i].to / 8), ordered_moves.scores[i],
              eval_before, eval_after, score);

    // Afficher le type de coup
    switch (ordered_moves.moves[i].type) {
    case MOVE_CAPTURE:
      DEBUG_LOG(" [CAPTURE]");
      break;
    case MOVE_PROMOTION:
      DEBUG_LOG(" [PROMOTION]");
      break;
    case MOVE_CASTLE:
      DEBUG_LOG(" [ROQUE]");
      break;
    case MOVE_EN_PASSANT:
      DEBUG_LOG(" [EN_PASSANT]");
      break;
    default:
      DEBUG_LOG(" [NORMAL]");
      break;
    }

    if (score > result.score) {
      result.score = score;
      result.best_move = ordered_moves.moves[i];
      DEBUG_LOG(" *** NOUVEAU MEILLEUR COUP ***");
    }
    DEBUG_LOG("\n");
  }

  // VALIDATION FINALE : S'assurer qu'on a un coup valide
  if (result.best_move.from == -1) {
    DEBUG_LOG("⚠️ AVERTISSEMENT: Aucun coup n'a amélioré le score!\n");
    DEBUG_LOG("    Fallback: utilisation du premier coup non-mauvais\n");

    // Prendre le premier coup qui n'est pas "obviously bad"
    for (int i = 0; i < ordered_moves.count; i++) {
      if (!is_obviously_bad_move(board, &ordered_moves.moves[i])) {
        result.best_move = ordered_moves.moves[i];
        result.score = -INFINITY_SCORE / 2; // Score neutre-mauvais
        break;
      }
    }

    // Si tous les coups sont "bad", prendre quand même le premier
    if (result.best_move.from == -1 && ordered_moves.count > 0) {
      result.best_move = ordered_moves.moves[0];
      result.score = -INFINITY_SCORE / 2;
      DEBUG_LOG("    Fallback final: utilisation du premier coup légal\n");
    }
  }

  DEBUG_LOG("\n>>> MEILLEUR COUP: %c%d%c%d (score=%d) <<<\n\n",
            'a' + (result.best_move.from % 8), 1 + (result.best_move.from / 8),
            'a' + (result.best_move.to % 8), 1 + (result.best_move.to / 8),
            result.score);

  result.depth = search_depth;
  return result;
}

// Filtre les coups évidemment mauvais
int is_obviously_bad_move(const Board *board, const Move *move) {
  // 1. Ne pas mettre une pièce en prise gratuite
  if (move->type == MOVE_NORMAL || move->type == MOVE_CAPTURE) {
    PieceType moving_piece = get_piece_type(board, move->from);
    Couleur moving_color = board->to_move;
    Couleur opponent = (moving_color == WHITE) ? BLACK : WHITE;

    // Si la case de destination est attaquée par l'adversaire
    if (is_square_attacked(board, move->to, opponent)) {
      // Et pas défendue par nous
      if (!is_square_attacked(board, move->to, moving_color)) {
        // Et ce n'est pas une capture qui vaut le coup
        if (move->type != MOVE_CAPTURE ||
            piece_value(move->captured_piece) < piece_value(moving_piece)) {
          return 1; // Coup évidemment mauvais
        }
      }
    }
  }

  // 2. En ouverture, ne pas sortir la dame trop tôt
  if (board->move_number <= 8) {
    PieceType moving_piece = get_piece_type(board, move->from);
    if (moving_piece == QUEEN) {
      int from_rank = move->from / 8;
      int home_rank = (board->to_move == WHITE) ? 0 : 7;

      // Dame qui sort de sa rangée de départ très tôt
      if (from_rank == home_rank) {
        // Sauf si c'est pour capturer quelque chose d'important
        if (move->type != MOVE_CAPTURE ||
            piece_value(move->captured_piece) < 300) {
          return 1;
        }
      }
    }
  }

  return 0;
}

// Interface principale SÉCURISÉE
SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms) {
  // Initialiser les variables globales de temps
  search_start_time = clock();
  search_time_limit_ms = time_limit_ms;
  search_should_stop = 0;

  clock_t start_time = search_start_time;
  SearchResult best_result = {0};
  best_result.score = -INFINITY_SCORE;
  best_result.nodes_searched = 0;
  // Initialisation EXPLICITE du Move invalide
  best_result.best_move.from = -1;
  best_result.best_move.to = -1;
  best_result.best_move.type = MOVE_NORMAL;
  best_result.best_move.promotion = EMPTY;
  best_result.best_move.captured_piece = EMPTY;

  // Adapter la profondeur minimale en fonction du temps disponible
  // Pour des temps très courts (< 100ms), commencer à profondeur 1
  // Sinon commencer à profondeur 2 pour éviter les bourdes évidentes
  int min_depth;
  if (time_limit_ms < 100) {
    min_depth = 1;
  } else if (time_limit_ms < 500) {
    min_depth = 2;
  } else {
    min_depth = 3;
  }

  DEBUG_LOG("\n=== ITERATIVE DEEPENING START ===\n");
  DEBUG_LOG("Max depth: %d, Time limit: %dms, Min depth: %d\n", max_depth,
            time_limit_ms, min_depth);

  tt_new_search(&tt_global);

  for (int depth = min_depth; depth <= max_depth; depth++) {
    // Vérifier le temps AVANT de commencer une nouvelle profondeur
    clock_t current_time = clock();
    int elapsed_ms =
        (int)(((double)(current_time - start_time)) / CLOCKS_PER_SEC * 1000);

    DEBUG_LOG("\n--- Depth %d (elapsed: %dms / %dms) ---\n", depth, elapsed_ms,
              time_limit_ms);

    // AMÉLIORATION: Arrêter seulement si on a utilisé 95% du temps
    // Cela permet de chercher plus profond tout en gardant une petite marge
    if (elapsed_ms >= (time_limit_ms * 19) / 20 && depth > min_depth) {
      DEBUG_LOG("Time limit reached (95%%), stopping at depth %d\n", depth - 1);
      break;
    }

    // Vérifier qu'on a assez de temps pour une itération complète
    // Heuristique: la prochaine profondeur prend ~4x le temps de la précédente
    // On vérifie qu'il reste au moins 5% du temps (au lieu de 20%)
    if (depth > min_depth && elapsed_ms * 20 >= time_limit_ms * 19) {
      DEBUG_LOG("Not enough time for depth %d (need ~4x current), stopping at "
                "depth %d\n",
                depth + 1, depth);
      break;
    }

    SearchResult current_result =
        search_best_move_with_min_depth(board, depth, min_depth);

    // Si la recherche a été interrompue et qu'on n'a pas de coup valide, ne pas
    // l'utiliser
    if (search_should_stop && current_result.best_move.from == -1 &&
        best_result.best_move.from != -1) {
      DEBUG_LOG("Search interrupted, using previous depth result\n");
      break;
    }

    // Accumuler les noeuds
    best_result.nodes_searched += current_result.nodes_searched;

    // Vérifier le temps APRÈS la recherche
    current_time = clock();
    elapsed_ms =
        (int)(((double)(current_time - start_time)) / CLOCKS_PER_SEC * 1000);

    // Calculer NPS (nodes per second)
    if (elapsed_ms > 0) {
      best_result.nps = (best_result.nodes_searched * 1000) / elapsed_ms;
    }

    // Ne mettre à jour que si on a un coup valide
    if (current_result.best_move.from != -1) {
      best_result.best_move = current_result.best_move;
      best_result.score = current_result.score;
      best_result.depth = current_result.depth;
    }

    DEBUG_LOG("Depth %d complete: score=%d, nodes=%d, elapsed=%dms, nps=%d\n",
              depth, current_result.score, best_result.nodes_searched,
              elapsed_ms, best_result.nps);

    // Arrêter si le temps est écoulé (100% utilisé)
    if (elapsed_ms >= time_limit_ms) {
      DEBUG_LOG("Time limit reached (100%%), stopping\n");
      break;
    }

    // SUPPRESSION de la vérification "40% du temps pour cette itération"
    // Cette heuristique est trop agressive et empêche d'utiliser tout le temps
    // On laisse l'iterative deepening continuer tant qu'on a du temps

    // Arrêt si mat trouvé
    if (abs(current_result.score) >= MATE_SCORE - 100) {
      DEBUG_LOG("Mate found, stopping\n");
      break;
    }
  }

  // Assurer que nodes et nps sont définis
  best_result.nodes = best_result.nodes_searched;
  if (best_result.nps == 0) {
    clock_t end_time = clock();
    int total_ms =
        (int)(((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000);
    if (total_ms > 0) {
      best_result.nps = (best_result.nodes_searched * 1000) / total_ms;
    }
  }

  // VALIDATION FINALE CRITIQUE : Vérifier que le coup est légal
  if (best_result.best_move.from != -1) {
    // PREMIÈRE VÉRIFICATION : La pièce doit appartenir à la bonne couleur
    Couleur piece_color = get_piece_color(board, best_result.best_move.from);
    if (piece_color != board->to_move) {
      DEBUG_LOG("❌ ERROR: Best move uses WRONG COLOR piece! piece_color=%d "
                "expected=%d\n",
                piece_color, board->to_move);
      DEBUG_LOG("    Invalid move: %c%d%c%d\n",
                'a' + (best_result.best_move.from % 8),
                1 + (best_result.best_move.from / 8),
                'a' + (best_result.best_move.to % 8),
                1 + (best_result.best_move.to / 8));

      // Forcer la régénération avec le premier coup légal
      MoveList emergency_moves;
      generate_legal_moves(board, &emergency_moves);
      if (emergency_moves.count > 0) {
        best_result.best_move = emergency_moves.moves[0];
        best_result.score = -INFINITY_SCORE / 2;
        DEBUG_LOG("    Emergency fallback: %s\n",
                  move_to_string(&best_result.best_move));
      } else {
        best_result.best_move.from = -1;
        best_result.best_move.to = -1;
      }
    } else {
      // DEUXIÈME VÉRIFICATION : Le coup doit être dans la liste des coups
      // légaux
      MoveList legal_moves;
      generate_legal_moves(board, &legal_moves);

      int move_found = 0;
      for (int i = 0; i < legal_moves.count; i++) {
        if (legal_moves.moves[i].from == best_result.best_move.from &&
            legal_moves.moves[i].to == best_result.best_move.to &&
            legal_moves.moves[i].type == best_result.best_move.type) {
          // Utiliser le coup complet de la liste légale
          best_result.best_move = legal_moves.moves[i];
          move_found = 1;
          break;
        }
      }

      if (!move_found) {
        DEBUG_LOG(
            "❌ ERROR: Best move is NOT in legal moves list! Falling back\n");
        if (legal_moves.count > 0) {
          best_result.best_move = legal_moves.moves[0];
          best_result.score = -INFINITY_SCORE / 2;
        } else {
          best_result.best_move.from = -1;
          best_result.best_move.to = -1;
        }
      }
    }
  }

  clock_t end_time = clock();
  int total_ms =
      (int)(((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000);

  DEBUG_LOG("\n=== ITERATIVE DEEPENING COMPLETE ===\n");
  DEBUG_LOG("Final depth: %d, Score: %d, Nodes: %d, NPS: %d, Time: %dms\n",
            best_result.depth, best_result.score, best_result.nodes,
            best_result.nps, total_ms);
  DEBUG_LOG("Best move: %s\n\n", move_to_string(&best_result.best_move));

  return best_result;
}

SearchResult search_best_move(Board *board, int depth) {
  // Utiliser la version sécurisée avec profondeur minimale
  return search_iterative_deepening(board, depth, 5000); // 5 secondes max
}
