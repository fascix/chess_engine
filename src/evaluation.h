#ifndef EVALUATION_H
#define EVALUATION_H

#include "board.h"
#include "movegen.h"

// Valeurs des pièces (en centipawns, 100 = 1 pion)
#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 20000

// Scores spéciaux pour l'évaluation
#define MATE_SCORE 30000
#define STALEMATE_SCORE 0

// Bonus d'évaluation
#define DOUBLED_PAWN_PENALTY -10
#define ISOLATED_PAWN_PENALTY -15
#define PASSED_PAWN_BONUS 20
#define BISHOP_PAIR_BONUS 30
#define CASTLED_KING_BONUS 40
#define KING_OPEN_FILE_PENALTY -20
#define CENTER_PAWN_BONUS 10

// Phases de jeu
typedef enum { OPENING_PHASE, MIDDLEGAME_PHASE, ENDGAME_PHASE } GamePhase;

// Fonctions d'évaluation principales
int evaluate_position(const Board *board);
int evaluate_material(const Board *board);
int evaluate_piece_square_tables(const Board *board);

// Évaluation avancée par catégorie
int evaluate_pawn_structure(const Board *board);
int evaluate_king_safety(const Board *board);
int evaluate_piece_development(const Board *board);
int evaluate_center_control(const Board *board);
int evaluate_mobility(const Board *board);

// Évaluation avec phases de jeu (interpolation)
int evaluate_opening(const Board *board);
int evaluate_endgame(const Board *board);
int evaluate_position_interpolated(const Board *board, GamePhase phase,
                                   float phase_factor);

// Fonctions utilitaires
int is_endgame(const Board *board);
int piece_value(PieceType piece);
GamePhase get_game_phase(const Board *board);
float get_phase_factor(const Board *board);
int is_pawn_passed(const Board *board, Square pawn_square, Couleur color);
int is_pawn_isolated(const Board *board, Square pawn_square, Couleur color);
int is_pawn_doubled(const Board *board, Square pawn_square, Couleur color);
int evaluate_safe_development(const Board *board);
int evaluate_hanging_pieces(const Board *board);
int evaluate_pawn_advancement_penalty(const Board *board);

#endif // EVALUATION_H
