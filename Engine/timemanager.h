#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include "board.h"

// Structure pour les paramètres de la commande "go"
typedef struct {
  int wtime;     // Temps restant pour les blancs (ms)
  int btime;     // Temps restant pour les noirs (ms)
  int winc;      // Incrément pour les blancs (ms)
  int binc;      // Incrément pour les noirs (ms)
  int movestogo; // Nombre de coups avant le prochain contrôle de temps
  int depth;     // Profondeur maximale
  int nodes;     // Nombre de nœuds maximal à explorer
  int mate;      // Chercher un mat en X coups
  int movetime;  // Temps fixe pour ce coup (ms)
  int infinite;  // Recherche infinie
  int ponder;    // Recherche en mode pondering
} GoParams;

// Parse les paramètres de la commande "go"
void parse_go_params(char *params, GoParams *go_params);

// Calcule le temps alloué pour ce coup
int calculate_time_for_move(const Board *board, const GoParams *params);

// Estime le nombre de coups restants selon la phase de jeu
int estimate_moves_to_go(const Board *board);

#endif // TIMEMANAGER_H
