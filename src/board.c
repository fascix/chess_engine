// board.c
#include "board.h"
#include <stdio.h>
#include <string.h>

void board_init(Board *board) {
  // Initialise position de départ
  memset(board, 0, sizeof(Board));
  // Met tous les champs de Board à zéro avant initialisation


  // Position initiale des pièces blanches
  board->pieces[WHITE][ROOK] = 0x81ULL;   // a1, h1
  board->pieces[WHITE][KNIGHT] = 0x42ULL; // b1, g1
  board->pieces[WHITE][BISHOP] = 0x24ULL; // c1, f1
  board->pieces[WHITE][QUEEN] = 0x8ULL;   // d1
  board->pieces[WHITE][KING] = 0x10ULL;   // e1
  board->pieces[WHITE][PAWN] = 0xFF00ULL; // rangée 2

  // Position initiale des pièces noires
  board->pieces[BLACK][ROOK] = 0x8100000000000000ULL;
  board->pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;
  board->pieces[BLACK][BISHOP] = 0x2400000000000000ULL;
  board->pieces[BLACK][QUEEN] = 0x800000000000000ULL;
  board->pieces[BLACK][KING] = 0x1000000000000000ULL;
  board->pieces[BLACK][PAWN] = 0xFF000000000000ULL;

  // Calcul des bitboards dérivés
  board->occupied[WHITE] =
      board->pieces[WHITE][PAWN] | board->pieces[WHITE][KNIGHT] |
      board->pieces[WHITE][BISHOP] | board->pieces[WHITE][ROOK] |
      board->pieces[WHITE][QUEEN] | board->pieces[WHITE][KING];

  board->occupied[BLACK] =
      board->pieces[BLACK][PAWN] | board->pieces[BLACK][KNIGHT] |
      board->pieces[BLACK][BISHOP] | board->pieces[BLACK][ROOK] |
      board->pieces[BLACK][QUEEN] | board->pieces[BLACK][KING];

  board->all_pieces = board->occupied[WHITE] | board->occupied[BLACK];

  // État initial
  board->to_move = WHITE;
  board->castle_rights = 0xF; // Tous les roques possibles
  board->en_passant = -1;
  board->halfmove_clock = 0;
  board->move_number = 1;
}