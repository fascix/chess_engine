#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "movegen.h"

// Structure pour le résultat de la recherche
typedef struct {
  Move best_move; // Meilleur coup trouvé
  int depth;      // Profondeur atteinte
  int score;      // Score de la position (en centipawns)
  int nodes;      // Nombre de nœuds explorés
  int nps;        // Nœuds par seconde
} SearchResult;

// Fonctions de recherche (à implémenter)
SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms);
void initialize_engine();

// Fonction utilitaire pour envoyer des infos UCI pendant la recherche
void send_search_info(int depth, int score, int nodes, int nps,
                      const Move *pv_move);

#endif // SEARCH_H
