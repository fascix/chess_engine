#ifndef UTILS_H
#define UTILS_H

#include "board.h"
#include "movegen.h"

// ============================================================================
// UTILITAIRES D'AFFICHAGE ET CONVERSION
// ============================================================================

// Conversion et affichage de coups
void print_move(const Move *move);
void print_movelist(const MoveList *list);
char *move_to_string(const Move *move);

// Affichage du board (debug)
void print_board_debug(const Board *board);

// ============================================================================
// UTILITAIRES D'ANALYSE DE PIÈCES
// ============================================================================

// Valeur des pièces
int piece_value(PieceType piece);

// Vérifications sur les pièces
int has_non_pawn_material(const Board *board, Couleur color);
int count_pieces(const Board *board, PieceType piece, Couleur color);

// ============================================================================
// UTILITAIRES D'ANALYSE DE PIONS (dans evaluation.h/c)
// ============================================================================

// is_pawn_passed, is_pawn_isolated, is_pawn_doubled sont dans evaluation.h

// ============================================================================
// UTILITAIRES D'ANALYSE DE COUPS
// ============================================================================

// Vérifications sur les coups
int gives_check(const Board *board, const Move *move);
int moves_toward_center(const Board *board, const Move *move);
int is_obviously_bad_move(const Board *board, const Move *move);
int is_capture(const Move *move);
int is_promotion(const Move *move);

// ============================================================================
// UTILITAIRES DE PHASE DE JEU (dans evaluation.h/c)
// ============================================================================

typedef enum { OPENING_PHASE, MIDDLEGAME_PHASE, ENDGAME_PHASE } GamePhase;

// is_endgame, get_game_phase, get_phase_factor sont dans evaluation.h

// ============================================================================
// UTILITAIRES DE GÉOMÉTRIE ÉCHIQUIER
// ============================================================================

// Distance et centralité
int square_distance(Square sq1, Square sq2);
int square_to_center_distance(Square sq);
int is_center_square(Square sq);
int is_extended_center_square(Square sq);

// Colonnes et rangées
int same_file(Square sq1, Square sq2);
int same_rank(Square sq1, Square sq2);
int same_diagonal(Square sq1, Square sq2);
int file_of(Square sq);
int rank_of(Square sq);

// ============================================================================
// UTILITAIRES DE TEMPS ET PERFORMANCE
// ============================================================================

// Timer simple pour mesurer les performances
typedef struct {
  long start_time_ms;
  long end_time_ms;
} Timer;

void timer_start(Timer *timer);
void timer_stop(Timer *timer);
long timer_elapsed_ms(const Timer *timer);

// Obtenir le temps actuel en millisecondes
long get_time_ms();

#endif // UTILS_H
