#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "board.h"
#include "movegen.h"
#include <stdint.h>

// Taille de la table de transposition (2^20 = 1M entrées)
#define TT_SIZE 1048576
#define TT_MASK (TT_SIZE - 1)

// Type d'entrée dans la table de transposition
typedef enum {
  TT_EXACT,      // Score exact
  TT_UPPERBOUND, // Borne supérieure (fail-low)
  TT_LOWERBOUND  // Borne inférieure (fail-high)
} TTEntryType;

// Entrée de la table de transposition
typedef struct {
  uint64_t key;     // Zobrist hash key
  int depth;        // Profondeur de recherche
  int score;        // Score de la position
  TTEntryType type; // Type d'entrée
  Move best_move;   // Meilleur mouvement trouvé
  uint8_t age;      // Age de l'entrée (pour remplacement)
} TTEntry;

// Table de transposition globale
typedef struct {
  TTEntry entries[TT_SIZE];
  uint8_t current_age;
} TranspositionTable;

// Initialise la table de transposition
void tt_init(TranspositionTable *tt);

// Stocke une entrée dans la table
void tt_store(TranspositionTable *tt, uint64_t key, int depth, int score,
              TTEntryType type, Move best_move);

// Sonde la table (retourne NULL si pas trouvé)
TTEntry *tt_probe(TranspositionTable *tt, uint64_t key);

// Nouvelle recherche (incrémente l'age)
void tt_new_search(TranspositionTable *tt);

#endif // TRANSPOSITION_H
