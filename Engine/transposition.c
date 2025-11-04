#include "transposition.h"
#include <stdio.h>
#include <string.h>

// Macro pour logs de debug conditionnels
#ifdef DEBUG
#define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

// ========== INITIALISATION ==========

void tt_init(TranspositionTable *tt) {
  memset(tt, 0, sizeof(TranspositionTable));
  tt->current_age = 1;

#ifdef DEBUG
  DEBUG_LOG("TT initialisée : %zu entrées (%zu bytes)\n", (size_t)TT_SIZE,
            sizeof(TranspositionTable));
#endif
}

// ========== STOCKAGE ==========

void tt_store(TranspositionTable *tt, uint64_t key, int depth, int score,
              TTEntryType type, Move best_move) {
  uint32_t index = key & TT_MASK;
  TTEntry *entry = &tt->entries[index];

  // VALIDATION : key ne doit JAMAIS être 0
  if (key == 0) {
#ifdef DEBUG
    static int zero_key_warnings = 0;
    if (zero_key_warnings++ < 3) {
      DEBUG_LOG("WARNING: tt_store() appelé avec key=0 !\n");
    }
#endif
    return; // NE PAS STOCKER
  }

  // Stratégie de remplacement améliorée :
  // 1. Case vide (key == 0)
  // 2. Même position (key == entry->key)
  // 3. Profondeur supérieure (depth > entry->depth)
  // 4. Entrée obsolète (age < current_age - 2)

  int should_replace = 0;

  if (entry->key == 0) {
    should_replace = 1; // Case vide
  } else if (entry->key == key) {
    should_replace = 1; // Même position (mise à jour)
  } else if (depth > entry->depth) {
    should_replace = 1; // Profondeur supérieure (info plus précise)
  } else if (entry->age < tt->current_age - 2) {
    should_replace = 1; // Entrée trop ancienne (2 recherches en arrière)
  }

  if (should_replace) {
    entry->key = key;
    entry->depth = depth;
    entry->score = score;
    entry->type = type;
    entry->best_move = best_move;
    entry->age = tt->current_age;

#ifdef DEBUG
    static int store_count = 0;
    if (store_count++ < 10) {
      DEBUG_LOG("TT_STORE: index=%u key=%016llx depth=%d score=%d\n", index,
                (unsigned long long)key, depth, score);
    }
#endif
  }
}

// ========== SONDAGE ==========

TTEntry *tt_probe(TranspositionTable *tt, uint64_t key) {
  // VALIDATION : key ne doit JAMAIS être 0
  if (key == 0) {
#ifdef DEBUG
    static int zero_key_probes = 0;
    if (zero_key_probes++ < 3) {
      DEBUG_LOG("WARNING: tt_probe() appelé avec key=0 !\n");
    }
#endif
    return NULL;
  }

  uint32_t index = key & TT_MASK;
  TTEntry *entry = &tt->entries[index];

  // Vérifier que la clé match ET que l'entrée n'est pas vide
  if (entry->key == key && entry->depth > 0) {
#ifdef DEBUG
    static int hit_count = 0;
    if (hit_count++ < 10) {
      DEBUG_LOG("TT_HIT: index=%u key=%016llx depth=%d score=%d\n", index,
                (unsigned long long)key, entry->depth, entry->score);
    }
#endif
    return entry;
  }

  return NULL; // Pas trouvé
}

// ========== NOUVELLE RECHERCHE ==========

void tt_new_search(TranspositionTable *tt) {
  tt->current_age++;
  if (tt->current_age == 0)
    tt->current_age = 1; // Éviter overflow
}
