#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "evaluation.h"
#include "move_ordering.h"
#include "movegen.h"
#include "quiescence.h"
#include "search_helpers.h"
#include "transposition.h"
#include "utils.h"
#include "zobrist.h"
#include <stdint.h>

// Structure pour le résultat de la recherche
typedef struct {
  Move best_move;     // Meilleur coup trouvé
  int depth;          // Profondeur atteinte
  int score;          // Score de la position (en centipawns)
  int nodes;          // Nombre de nœuds explorés
  int nps;            // Nœuds par seconde
  int nodes_searched; // Nombre de nœuds explorés (alias)
} SearchResult;

// ========== FONCTIONS PRINCIPALES ==========

// Interface principale de recherche
SearchResult search_best_move(Board *board, int depth);
SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms);

// Initialisation du moteur
void initialize_engine(void);

// Fonction utilitaire pour envoyer des infos UCI pendant la recherche
void send_search_info(int depth, int score, int nodes, int nps,
                      const Move *pv_move);

// ========== NEGAMAX ==========

// Negamax avec Alpha-Beta et toutes les optimisations
int negamax_alpha_beta(Board *board, int depth, int alpha, int beta,
                       Couleur color, int ply, int in_null_move);

#endif // SEARCH_H
