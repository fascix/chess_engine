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

  for (int i = 0; i < moves->count; i++) {
    ordered->moves[i] = moves->moves[i];
    Move *move = &ordered->moves[i];
    int score = 0;

    // 1. Hash move (priorité maximale)
    if (hash_move.from == move->from && hash_move.to == move->to) {
      score = 1000000;
    }
    // 2. Captures (MVV-LVA)
    else if (move->type == MOVE_CAPTURE || move->type == MOVE_EN_PASSANT) {
      score = 100000 + mvv_lva_score(move);
    }
#if VERSION >= 9
    // 3. Killer moves
    else if (is_killer_move(*move, ply)) {
      score = 90000;
    }
#endif
#if VERSION >= 8
    // 4. History heuristic pour les coups quiet ← CETTE LIGNE MANQUE !
    else {
      score = history_scores[board->to_move][move->from][move->to];
    }
#else
    // Sans history : score par défaut
    else {
      score = 0;
    }
#endif

    ordered->scores[i] = score;
  }

  // Tri par score décroissant
  for (int i = 0; i < ordered->count - 1; i++) {
    for (int j = i + 1; j < ordered->count; j++) {
      if (ordered->scores[j] > ordered->scores[i]) {
        // Swap scores
        int temp_score = ordered->scores[i];
        ordered->scores[i] = ordered->scores[j];
        ordered->scores[j] = temp_score;
        // Swap moves
        Move temp_move = ordered->moves[i];
        ordered->moves[i] = ordered->moves[j];
        ordered->moves[j] = temp_move;
      }
    }
  }
}