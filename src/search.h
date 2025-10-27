#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "evaluation.h"
#include "movegen.h"
#include <stdint.h>

// Scores spéciaux pour la recherche
#define MATE_SCORE 30000
#define STALEMATE_SCORE 0
#define INFINITY_SCORE 50000

// Transposition Table
#define TT_SIZE 1048576 // 2^20 entrées
#define TT_MASK (TT_SIZE - 1)

typedef enum {
  TT_EXACT,      // Score exact
  TT_UPPERBOUND, // Borne supérieure (fail-low)
  TT_LOWERBOUND  // Borne inférieure (fail-high)
} TTEntryType;

typedef struct {
  uint64_t key;     // Zobrist hash key
  int depth;        // Profondeur de recherche
  int score;        // Score de la position
  TTEntryType type; // Type d'entrée
  Move best_move;   // Meilleur mouvement trouvé
  uint8_t age;      // Age de l'entrée
} TTEntry;

typedef struct {
  TTEntry entries[TT_SIZE];
  uint8_t current_age;
} TranspositionTable;

// Types de recherche
typedef struct {
  Move best_move;
  int score;
  int depth;
  int nodes;          // Ajoute ce champ
  int nps;            // Ajoute ce champ
  int nodes_searched; // Ajoute ce champ
} SearchResult;

// Algorithmes de recherche

// Interface principale de recherche
SearchResult search_best_move(Board *board, int depth);
SearchResult search_iterative_deepening(Board *board, int max_depth,
                                        int time_limit_ms);

// Transposition Table
void tt_init(TranspositionTable *tt);
void tt_store(TranspositionTable *tt, uint64_t key, int depth, int score,
              TTEntryType type, Move best_move);
TTEntry *tt_probe(TranspositionTable *tt, uint64_t key);
void tt_new_search(TranspositionTable *tt);

// Zobrist hashing
uint64_t zobrist_hash(const Board *board);
void init_zobrist();

// Initialisation du moteur
void initialize_engine();
void test_zobrist_uniqueness();

// Move Ordering
typedef struct {
  Move moves[256];
  int scores[256];
  int count;
} OrderedMoveList;

void order_moves(const Board *board, MoveList *moves, OrderedMoveList *ordered,
                 Move hash_move, int ply);
int mvv_lva_score(const Move *move);
void init_killer_moves();
void store_killer_move(Move move, int ply);
int is_killer_move(Move move, int ply);
void update_history(Move move, int depth, Couleur color);

// Quiescence Search
int quiescence_search(Board *board, int alpha, int beta, Couleur color,
                      int ply);
int quiescence_search_depth(Board *board, int alpha, int beta, Couleur color,
                            int ply);
void generate_capture_moves(const Board *board, MoveList *moves);

// Fonctions utilitaires pour make/unmake
void apply_move(Board *board, const Move *move, int ply);
void undo_move(Board *board, int ply);

// Helpers from search.c
int gives_check(const Board *board, const Move *move);
int moves_toward_center(const Board *board, const Move *move);
int is_obviously_bad_move(const Board *board, const Move *move);

#endif // SEARCH_H
