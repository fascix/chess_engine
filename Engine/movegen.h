#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"

// Types de coups spéciaux
typedef enum {
  MOVE_NORMAL = 0,     // Coup normal
  MOVE_CAPTURE = 1,    // Capture
  MOVE_EN_PASSANT = 2, // En passant
  MOVE_CASTLE = 3,     // Roque
  MOVE_PROMOTION = 4   // Promotion de pion
} MoveType;

// Structure d'un coup - Simple et claire
typedef struct {
  Square from;              // Case de départ
  Square to;                // Case d'arrivée
  MoveType type;            // Type de coup
  PieceType promotion;      // Pièce de promotion (EMPTY si pas de promotion)
  PieceType captured_piece; // Pièce capturée (EMPTY si pas de capture)
} Move;

// Liste de coups pour la génération
typedef struct {
  Move moves[256]; // Maximum ~200 coups possibles par position exactement 218
                   // selon Claude Shannon
  int count;       // Nombre de coups générés
} MoveList;

// Fonctions de base
void movelist_init(MoveList *list);
void movelist_add(MoveList *list, Move move);
Move create_move(Square from, Square to, MoveType type);
Move create_promotion_move(Square from, Square to, PieceType promotion);

// Génération de coups
void generate_moves(const Board *board, MoveList *moves);
void generate_pawn_moves(const Board *board, Couleur color, MoveList *moves);
void generate_rook_moves(const Board *board, Couleur color, MoveList *moves);
void generate_bishop_moves(const Board *board, Couleur color, MoveList *moves);
void generate_knight_moves(const Board *board, Couleur color, MoveList *moves);
void generate_queen_moves(const Board *board, Couleur color, MoveList *moves);
void generate_king_moves(const Board *board, Couleur color, MoveList *moves);

// Détection d'échec
int is_square_attacked(const Board *board, Square square,
                       Couleur attacking_color);
int is_in_check(const Board *board, Couleur color);

// Mouvements légaux et fin de partie
void generate_legal_moves(const Board *board, MoveList *moves);
int is_move_legal(const Board *board, const Move *move);
void filter_legal_moves(const Board *board, MoveList *moves);
int is_stalemate(const Board *board);
int is_checkmate(const Board *board);
int is_fifty_move_rule(const Board *board);

// Fonctions make/unmake temporaires pour la recherche
void make_move_temp(Board *board, const Move *move, Board *backup);
void unmake_move_temp(Board *board, const Board *backup);

// Résultat de partie
typedef enum {
  GAME_ONGOING,
  GAME_CHECKMATE_WHITE,
  GAME_CHECKMATE_BLACK,
  GAME_STALEMATE,
  GAME_FIFTY_MOVE_RULE
} GameResult;

GameResult get_game_result(const Board *board);

// Utilitaires d'affichage
void print_move(const Move *move);
void print_movelist(const MoveList *list);
char *move_to_string(const Move *move);

#define ADD_PROMOTIONS(from, to, captured, moves)                              \
  do {                                                                         \
    Move promoQ = create_promotion_move((from), (to), QUEEN);                  \
    promoQ.captured_piece = (captured);                                        \
    movelist_add((moves), promoQ);                                             \
    Move promoR = create_promotion_move((from), (to), ROOK);                   \
    promoR.captured_piece = (captured);                                        \
    movelist_add((moves), promoR);                                             \
    Move promoB = create_promotion_move((from), (to), BISHOP);                 \
    promoB.captured_piece = (captured);                                        \
    movelist_add((moves), promoB);                                             \
    Move promoN = create_promotion_move((from), (to), KNIGHT);                 \
    promoN.captured_piece = (captured);                                        \
    movelist_add((moves), promoN);                                             \
  } while (0)

#endif // MOVEGEN_H