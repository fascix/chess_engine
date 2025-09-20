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
  Move moves[256]; // Maximum ~200 coups possibles par position
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

// Utilitaires d'affichage
void print_move(const Move *move);
void print_movelist(const MoveList *list);

#endif // MOVEGEN_H
