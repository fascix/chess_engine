#include "evaluation.h"
#include "Piece_Square_tables.h"
#include "utils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// PeSTO piece values (MG, EG)
static const int mg_value[6] = {MG_PAWN, MG_KNIGHT, MG_BISHOP,
                                MG_ROOK, MG_QUEEN,  MG_KING};
static const int eg_value[6] = {EG_PAWN, EG_KNIGHT, EG_BISHOP,
                                EG_ROOK, EG_QUEEN,  EG_KING};

// Game phase weights
static const int phase_weight[6] = {0, 1, 1, 2, 4, 0};

// Évaluation PeSTO
int evaluate_position(const Board *board) {
  // 1. Vérifier si la partie est terminée
  GameResult result = get_game_result(board);
  switch (result) {
  case GAME_CHECKMATE_WHITE:
    return -MATE_SCORE;
  case GAME_CHECKMATE_BLACK:
    return MATE_SCORE;
  case GAME_STALEMATE:
  case GAME_FIFTY_MOVE_RULE:
    return STALEMATE_SCORE;
  case GAME_ONGOING:
    break;
  default:
    break;
  }

  int mg[2] = {0, 0};
  int eg[2] = {0, 0};
  int game_phase = 0;

  // 2. Accumuler les scores MG et EG pour chaque couleur
  for (Couleur color = WHITE; color <= BLACK; color++) {
    for (PieceType type = PAWN; type <= KING; type++) {
      Bitboard pieces = board->pieces[color][type];
      while (pieces) {
        Square sq = __builtin_ctzll(pieces);
        pieces &= pieces - 1;

        // Pour les noirs, on inverse la case (miroir vertical)
        Square pesto_sq = (color == WHITE) ? sq : (sq ^ 56);

        mg[color] += mg_value[type] + mg_table[type][pesto_sq];
        eg[color] += eg_value[type] + eg_table[type][pesto_sq];
        game_phase += phase_weight[type];
      }
    }
  }

  // 3. Calculer le score final (du point de vue des blancs)
  int mg_score = mg[WHITE] - mg[BLACK];
  int eg_score = eg[WHITE] - eg[BLACK];

  // 4. Interpolation selon la phase
  // game_phase va de 0 (endgame) à 24 (opening)
  if (game_phase > 24)
    game_phase = 24;

  int score = ((mg_score * game_phase) + (eg_score * (24 - game_phase))) / 24;

  // Retourner le score selon la couleur qui doit jouer ?
  // Non, evaluate_position doit retourner du point de vue BLANC par convention
  // dans ce moteur (confirmé par search.c qui ajuste le score selon la couleur)
  return score;
}

// Fonctions de compatibilité (si nécessaire)
int is_endgame(const Board *board) {
  int game_phase = 0;
  for (Couleur color = WHITE; color <= BLACK; color++) {
    for (PieceType type = KNIGHT; type <= QUEEN; type++) {
      game_phase +=
          __builtin_popcountll(board->pieces[color][type]) * phase_weight[type];
    }
  }
  return game_phase <= 10; // Seuil arbitraire pour "endgame"
}

GamePhase get_game_phase(const Board *board) {
  int game_phase = 0;
  for (Couleur color = WHITE; color <= BLACK; color++) {
    for (PieceType type = KNIGHT; type <= QUEEN; type++) {
      game_phase +=
          __builtin_popcountll(board->pieces[color][type]) * phase_weight[type];
    }
  }
  if (game_phase > 20)
    return OPENING_PHASE;
  if (game_phase > 10)
    return MIDDLEGAME_PHASE;
  return ENDGAME_PHASE;
}

// Stub pour les autres fonctions si elles sont encore appelées ailleurs
int evaluate_material(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_piece_square_tables(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_pawn_structure(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_king_safety(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_piece_development(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_center_control(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_mobility(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_opening(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_endgame(const Board *board) {
  (void)board;
  return 0;
}
int evaluate_position_interpolated(const Board *board, GamePhase phase,
                                   float phase_factor) {
  (void)phase;
  (void)phase_factor;
  return evaluate_position(board);
}
