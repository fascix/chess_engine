#ifndef QUIESCENCE_H
#define QUIESCENCE_H

#include "board.h"
#include "movegen.h"

// Recherche quiescence (recherche uniquement les coups "bruyants")
int quiescence_search(Board *board, int alpha, int beta, Couleur color,
                      int ply);

// Recherche quiescence avec limite de profondeur
int quiescence_search_depth(Board *board, int alpha, int beta, Couleur color,
                            int ply);

// Génère uniquement les captures
void generate_capture_moves(const Board *board, MoveList *moves);

#endif // QUIESCENCE_H
