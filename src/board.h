#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

// Déclaration du type pour le Bitboard (64 bits)
typedef uint64_t Bitboard;

// Énumération des couleurs
typedef enum { WHITE, BLACK } Couleur;

// Énumération des différentes pièces
typedef enum { EMPTY = -1, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } PieceType;

// Énumération des cases
typedef enum {
  A1,
  B1,
  C1,
  D1,
  E1,
  F1,
  G1,
  H1,
  A2,
  B2,
  C2,
  D2,
  E2,
  F2,
  G2,
  H2,
  A3,
  B3,
  C3,
  D3,
  E3,
  F3,
  G3,
  H3,
  A4,
  B4,
  C4,
  D4,
  E4,
  F4,
  G4,
  H4,
  A5,
  B5,
  C5,
  D5,
  E5,
  F5,
  G5,
  H5,
  A6,
  B6,
  C6,
  D6,
  E6,
  F6,
  G6,
  H6,
  A7,
  B7,
  C7,
  D7,
  E7,
  F7,
  G7,
  H7,
  A8,
  B8,
  C8,
  D8,
  E8,
  F8,
  G8,
  H8
} Square;

typedef struct {
  // Bitboard par pièce et par couleur
  Bitboard pieces[2][6];
  Bitboard occupied[2];
  // occupied[WHITE] renvoie toutes les cases occupées par des pièces blanches
  // (de même pour BLACK) sous forme de bits (64)
  Bitboard all_pieces;
  // Possède le même rôle que occupied mais sans distinction de couleur,
  // renverra un bit (64)

  // État du jeu
  Couleur to_move;   // Pour savoir c'est à qui de jouer
  int castle_rights; // Possibilité de roquer ou non (stocké dans un 4 bits car
                     // 4 possibilités :
  // [Roque BLANC/NOIRE et GRAND ou PETIT ROQUE])
  Square en_passant;
  // Case ou il est possible de capturer grâce à la règle "en passant"
  // (ex: le pion blanc va de E2 vers E4 => en_passant = E3)
  // -1 si aucun en passant possible
  int halfmove_clock; // A INTEGRER DANS LE GUI
  // Compteur de demi-coups (1 action = 1 demi-coup)
  // Sert à la règle des 50 coups : remis à 0 après un pion joué ou une capture
  int move_number; // Numérote le nombre de coup de la partie
} Board;

// Fonctions de base à intégrer dans un bitboard
void board_init(Board *board);

void board_from_fen(
    Board *board,
    const char *fen); // Initialise le plateau à partir d'une chaîne FEN
// FEN = format texte qui décrit la position, le joueur à jouer, les droits de
// roque, la case en passant, le compteur de demi-coups et le numéro de coup
// complet

void print_board(const Board *board);
bool is_square_occupied(const Board *board, Square square);
PieceType get_piece_type(const Board *board, Square square);
Couleur get_piece_color(const Board *board, Square square);
void board_from_fen(Board *board, const char *fen);
void fen_char_to_piece_info(char c, PieceType *type, Couleur *couleur);

// Macros utiles à intégrer dans un bitboard
#define SET_BIT(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define CLEAR_BIT(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define GET_BIT(bitboard, square) (((bitboard) >> (square)) & 1)

#endif
