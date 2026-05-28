#ifndef SYZYGY_H
#define SYZYGY_H

#include "board.h"
#include "movegen.h"

// Syzygy Tablebase Probe Interface
// Note: Full implementation requires linking with a library like 'fathom'
// or implementing the RTBW/RTBZ compression format.

// Initialize Syzygy tablebases from the given directory path
void syzygy_init(const char *path);

// Probe WDL (Win/Draw/Loss) for the current position
// Returns: 0 (Loss), 1 (Blessed Loss), 2 (Draw), 3 (Cursed Win), 4 (Win)
// Returns -1 if position not found in tablebases
int syzygy_probe_wdl(const Board *board);

// Probe DTZ (Distance to Zeroing) for the current position
// Returns the distance to the next zeroing move (pawn move or capture)
// Returns 0 if position not found or not in endgame
int syzygy_probe_dtz(const Board *board, Move *best_move);

#endif // SYZYGY_H
