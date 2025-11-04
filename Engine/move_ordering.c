#include "move_ordering.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

// ========== TABLES GLOBALES ==========

static Move killer_moves[128][2];     // [ply][killer_slot]
static int history_scores[2][64][64]; // [color][from][to]

// ========== INITIALISATION ==========

void init_killer_moves(void) {
  memset(killer_moves, 0, sizeof(killer_moves));
  memset(history_scores, 0, sizeof(history_scores));
}

// ========== MVV-LVA ==========

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

// ========== KILLER MOVES ==========

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

int is_killer_move(Move move, int ply) {
  if (ply >= 128)
    return 0;

  return ((killer_moves[ply][0].from == move.from &&
           killer_moves[ply][0].to == move.to) ||
          (killer_moves[ply][1].from == move.from &&
           killer_moves[ply][1].to == move.to));
}

// ========== HISTORY HEURISTIC ==========

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

// ========== SEE (Static Exchange Evaluation) ==========

static int see_capture(const Board *board, const Move *move) {
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

// ========== ORDONNANCEMENT DES COUPS ==========

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
