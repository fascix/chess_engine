#ifndef POLYGLOT_H
#define POLYGLOT_H

#include "board.h"
#include "movegen.h"

// Polyglot Entry Structure (16 bytes)
typedef struct {
  uint64_t key;    // 8 bytes
  uint16_t move;   // 2 bytes
  uint16_t weight; // 2 bytes
  uint32_t learn;  // 4 bytes
} PolyglotEntry;

// Polyglot move format:
// bits 0-5: to file (0-7), rank (0-7) -> but it's actually:
// bits 0-2: to file
// bits 3-5: to rank
// bits 6-8: from file
// bits 9-11: from rank
// bits 12-14: promotion (0: none, 1: knight, 2: bishop, 3: rook, 4: queen)

void polyglot_init(void);
Move polyglot_get_move(const Board *board, const char *book_path);
uint64_t polyglot_hash(const Board *board);
Move parse_polyglot_move(uint16_t move_bits, const Board *board);

#endif // POLYGLOT_H
