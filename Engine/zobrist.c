#include "zobrist.h"
#include "movegen.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

// ========== TABLES ZOBRIST GLOBALES ==========
static uint64_t zobrist_pieces[2][6][64]; // [color][piece][square]
static uint64_t zobrist_castling[16];     // [castle_rights]
static uint64_t zobrist_en_passant[64];   // [square]
static uint64_t zobrist_side_to_move;     // Joueur actuel

// ========== GÉNÉRATEUR DE NOMBRES ALÉATOIRES ==========

// Générateur Xorshift* (rapide et de bonne qualité)
static uint64_t random_uint64(void) {
  static uint64_t seed = 0;

  // Initialiser le seed UNE SEULE FOIS avec timestamp + PID
  if (seed == 0) {
    seed = (uint64_t)time(NULL) ^ ((uint64_t)getpid() << 16);

    // Warm-up du générateur (important pour la qualité)
    for (int i = 0; i < 64; i++) {
      seed ^= seed >> 12;
      seed ^= seed << 25;
      seed ^= seed >> 27;
      seed *= 0x2545F4914F6CDD1DULL;
    }
  }

  // Xorshift* (excellent générateur rapide)
  seed ^= seed >> 12;
  seed ^= seed << 25;
  seed ^= seed >> 27;
  return seed * 0x2545F4914F6CDD1DULL;
}

// ========== INITIALISATION ZOBRIST ==========

void init_zobrist(void) {
  // Initialiser les clés pour chaque pièce sur chaque case
  for (int color = 0; color < 2; color++) {
    for (int piece = 0; piece < 6; piece++) {
      for (int square = 0; square < 64; square++) {
        zobrist_pieces[color][piece][square] = random_uint64();
      }
    }
  }

  // Droits de roque (16 combinaisons possibles)
  for (int i = 0; i < 16; i++) {
    zobrist_castling[i] = random_uint64();
  }

  // En passant (64 cases possibles)
  for (int i = 0; i < 64; i++) {
    zobrist_en_passant[i] = random_uint64();
  }

  // Joueur actuel
  zobrist_side_to_move = random_uint64();

#ifdef DEBUG
  // VALIDATION : vérifier qu'aucune clé n'est à 0
  int zero_count = 0;
  for (int c = 0; c < 2; c++) {
    for (int p = 0; p < 6; p++) {
      for (int s = 0; s < 64; s++) {
        if (zobrist_pieces[c][p][s] == 0)
          zero_count++;
      }
    }
  }
  if (zero_count > 0) {
    DEBUG_LOG("WARNING: %d clés Zobrist sont à 0 !\n", zero_count);
  }
  DEBUG_LOG("Zobrist initialisé : %d clés non-nulles\n",
            2 * 6 * 64 + 16 + 64 + 1 - zero_count);
#endif
}

// ========== CALCUL DU HASH ZOBRIST ==========

uint64_t zobrist_hash(const Board *board) {
  if (!board) {
    DEBUG_LOG("zobrist_hash: null board pointer\n");
    return 0;
  }

  uint64_t hash = 0;

  // Hash des pièces (défensif: vérification bornes)
  for (int color = WHITE; color <= BLACK; color++) {
    for (int piece = PAWN; piece <= KING; piece++) {
      Bitboard pieces = board->pieces[color][piece];
      while (pieces) {
        int square = __builtin_ctzll(pieces);
        if (square < 0 || square >= 64) {
          DEBUG_LOG("zobrist_hash: invalid square=%d (color=%d piece=%d)\n",
                    square, color, piece);
          // Clear lowest bit and continue defensively
          pieces &= pieces - 1;
          continue;
        }
        hash ^= zobrist_pieces[color][piece][square];
        pieces &= pieces - 1;
      }
    }
  }

  // Droits de roque (vérifier borne)
  if (board->castle_rights >= 0 && board->castle_rights < 16) {
    hash ^= zobrist_castling[board->castle_rights];
  } else {
    DEBUG_LOG("zobrist_hash: invalid castle_rights=%d\n", board->castle_rights);
  }

  // En passant (vérifier borne)
  if (board->en_passant >= 0 && board->en_passant < 64) {
    hash ^= zobrist_en_passant[board->en_passant];
  } else if (board->en_passant != -1) {
    // -1 signifie pas d'en-passant; autres valeurs sont suspectes
    DEBUG_LOG("zobrist_hash: invalid en_passant=%d\n", board->en_passant);
  }

  // Joueur actuel
  if (board->to_move == BLACK) {
    hash ^= zobrist_side_to_move;
  }

  return hash;
}

// ========== TEST DE VALIDATION ==========

void test_zobrist_uniqueness(void) {
  DEBUG_LOG("\n=== TEST UNICITÉ ZOBRIST ===\n");

  // Test 1 : Positions différentes = hash différents
  Board b1, b2;
  board_init(&b1);
  board_init(&b2);

  uint64_t h1 = zobrist_hash(&b1);
  DEBUG_LOG("Position initiale : hash = %016llx\n", (unsigned long long)h1);

  // Bouger un pion
  Move m = {.from = E2, .to = E4, .type = MOVE_NORMAL};
  Board backup;
  make_move_temp(&b2, &m, &backup);

  uint64_t h2 = zobrist_hash(&b2);
  DEBUG_LOG("Après e2e4      : hash = %016llx\n", (unsigned long long)h2);

  if (h1 == h2) {
    DEBUG_LOG("❌ ERREUR : Hash identiques pour positions différentes !\n");
  } else {
    DEBUG_LOG("✓ Hash différents (écart = %lld)\n",
              (long long)(h2 > h1 ? h2 - h1 : h1 - h2));
  }

  // Test 2 : Vérifier qu'aucun hash n'est 0
  if (h1 == 0 || h2 == 0) {
    DEBUG_LOG("❌ ERREUR : Hash égal à 0 détecté !\n");
  } else {
    DEBUG_LOG("✓ Aucun hash nul\n");
  }

  DEBUG_LOG("=== FIN TEST ===\n\n");
}
