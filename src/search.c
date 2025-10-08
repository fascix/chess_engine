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

// Mise à jour de negamax_alpha_beta
// Negamax avec Alpha-Beta et PVS (Principal Variation Search)
// PVS: recherche le 1er coup avec fenêtre complète [-beta, -alpha],
//      puis les suivants avec null window [-alpha-1, -alpha] pour détecter
//      rapidement les coups qui n'améliorent pas alpha (avec re-recherche si
//      nécessaire)
int negamax_alpha_beta(Board *board, int depth, int alpha, int beta,
                       Couleur color, int ply) {
  // Sécurité: ply trop grand
  if (ply >= 128) {
    DEBUG_LOG("ERROR: ply=%d trop grand, retour score neutre\n", ply);
    return 0; // score neutre
  }
  // Table de transposition : probe au début
  // TEMPORAIREMENT DÉSACTIVÉ POUR DEBUG
  /*
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
  */

  if (depth == 0) {
    int eval = quiescence_search(board, alpha, beta, color);
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
  // TT désactivée: entry est NULL
  // if (entry != NULL) {
  //   hash_move = entry->best_move;
  // }
  order_moves(board, &moves, &ordered_moves, hash_move, ply);
  // Sécurité OrderedMoveList: limiter à 256
  if (ordered_moves.count > 256) {
    DEBUG_LOG("WARNING: ordered_moves.count=%d > 256, tronqué à 256 coups\n",
              ordered_moves.count);
    ordered_moves.count = 256;
  }

  for (int i = 0; i < ordered_moves.count; i++) {
    // Sécurité avant apply_move
    if (ply >= 128) {
      DEBUG_LOG("ERROR: ply=%d trop grand avant apply_move, skip coup\n", ply);
      continue;
    }
    apply_move(board, &ordered_moves.moves[i], ply);

    // Recherche récursive avec PVS (Principal Variation Search)
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int score;

    if (i == 0) {
      // Premier coup : recherche avec fenêtre complète (PV node)
      score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                  ply + 1);
    } else {
      // Coups suivants : recherche avec fenêtre nulle (null window search)
      score = -negamax_alpha_beta(board, depth - 1, -alpha - 1, -alpha,
                                  opponent, ply + 1);

      // Si le coup améliore alpha et qu'on a une vraie fenêtre, re-rechercher
      if (score > alpha && score < beta) {
        // Re-search avec fenêtre complète
        score = -negamax_alpha_beta(board, depth - 1, -beta, -alpha, opponent,
                                    ply + 1);
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
}

// Stocke une entrée dans la table de transposition
void tt_store(TranspositionTable *tt, uint64_t key, int depth, int score,
              TTEntryType type, Move best_move) {
  uint32_t index = key & TT_MASK;
  TTEntry *entry = &tt->entries[index];

  // Remplacer si: case vide, même position, ou profondeur plus grande, ou age
  // plus ancienne
  if (entry->key == 0 || entry->key == key || entry->depth <= depth ||
      entry->age < tt->current_age) {
    entry->key = key;
    entry->depth = depth;
    entry->score = score;
    entry->type = type;
    entry->best_move = best_move;
    entry->age = tt->current_age;
  }
}

// Sonde la table de transposition
TTEntry *tt_probe(TranspositionTable *tt, uint64_t key) {
  uint32_t index = key & TT_MASK;
  TTEntry *entry = &tt->entries[index];

  if (entry->key == key) {
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

// Générateur de nombres pseudo-aléatoires simple
static uint64_t random_uint64() {
  static uint64_t seed = 1234567890ULL;
  seed ^= seed >> 12;
  seed ^= seed << 25;
  seed ^= seed >> 27;
  return seed * 0x2545F4914F6CDD1DULL;
}

// Initialise les tables Zobrist
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
int quiescence_search(Board *board, int alpha, int beta, Couleur color) {
  return quiescence_search_depth(board, alpha, beta, color, 0);
}

// Quiescence Search avec limite de profondeur
int quiescence_search_depth(Board *board, int alpha, int beta, Couleur color,
                            int qs_depth) {
  // Limite de profondeur pour éviter les boucles infinies
  if (qs_depth >= 8) {
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
  Move null_move = {0};
  order_moves(board, &capture_moves, &ordered_captures, null_move, 0);

  // Chercher dans les captures
  for (int i = 0; i < ordered_captures.count; i++) {
    Board backup = *board;

    // Delta pruning - ignorer les captures très faibles
    int delta = piece_value(ordered_captures.moves[i].captured_piece) + 200;
    if (stand_pat + delta < alpha) {
      continue;
    }

    // Jouer la capture (temporaire)
    make_move_temp(board, &ordered_captures.moves[i], &backup);

    // Recherche récursive
    Couleur opponent = (color == WHITE) ? BLACK : WHITE;
    int score = -quiescence_search(board, -beta, -alpha, opponent);

    // Restaurer le plateau
    *board = backup;

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

    // 1. Hash move - priorité absolue
    if (hash_move.from == moves->moves[i].from &&
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
  result.best_move.from = A1;
  result.best_move.to = A1;
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
  Move hash_move = {0};
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
    int score = -negamax_alpha_beta(board, search_depth - 1, -INFINITY_SCORE,
                                    INFINITY_SCORE, opponent, 1);
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
  clock_t start_time = clock();
  SearchResult best_result = {0};
  best_result.score = -INFINITY_SCORE;

  // SÉCURITÉ: Profondeur minimale de 3 pour éviter les bourdes
  int min_depth = 3;

  tt_new_search(&tt_global);

  for (int depth = min_depth; depth <= max_depth; depth++) {
    SearchResult current_result =
        search_best_move_with_min_depth(board, depth, min_depth);

    // Vérifier le temps
    clock_t current_time = clock();
    int elapsed_ms =
        (int)(((double)(current_time - start_time)) / CLOCKS_PER_SEC * 1000);

    if (elapsed_ms >= time_limit_ms && depth > min_depth) {
      break;
    }

    best_result = current_result;

    // Arrêt si mat trouvé
    if (abs(current_result.score) >= MATE_SCORE - 100) {
      break;
    }
  }

  return best_result;
}

SearchResult search_best_move(Board *board, int depth) {
  // Utiliser la version sécurisée avec profondeur minimale
  return search_iterative_deepening(board, depth, 5000); // 5 secondes max
}
