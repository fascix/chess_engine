# Guide d'utilisation du Logger

Ce guide explique comment utiliser le système de logging intégré dans le moteur d'échecs Pallas.

## Vue d'ensemble

Le système de logging utilise la bibliothèque [log.c](https://github.com/rxi/log.c/) qui fournit 6 niveaux de logging :

1. **LOG_TRACE** : Informations de traçage très détaillées (désactivé par défaut)
2. **LOG_DEBUG** : Informations de débogage pour le développement
3. **LOG_INFO** : Informations générales sur le fonctionnement
4. **LOG_WARN** : Avertissements sur des situations anormales
5. **LOG_ERROR** : Erreurs qui peuvent être récupérées
6. **LOG_FATAL** : Erreurs fatales qui arrêtent le programme

## Utilisation de base

### 1. Initialisation (déjà fait dans main.c)

```c
#include "logger.h"

int main() {
    // Initialiser le logger (configuration automatique selon DEBUG/RELEASE)
    logger_init();
    
    // ... reste du code
}
```

### 2. Utilisation des macros de logging

```c
#include "logger.h"

void ma_fonction(int profondeur, int alpha, int beta) {
    // Debug : informations détaillées pour le développement
    LOG_DEBUG("Recherche : profondeur=%d alpha=%d beta=%d", profondeur, alpha, beta);
    
    // Info : événements importants
    LOG_INFO("Recherche terminée : %d noeuds en %d ms", noeuds, temps);
    
    // Warning : situations suspectes mais non critiques
    LOG_WARN("Table de transposition à 90%% de capacité");
    
    // Error : erreurs récupérables
    LOG_ERROR("Échec de l'allocation mémoire : %zu bytes demandés", taille);
    
    // Fatal : erreurs critiques (arrête généralement le programme)
    LOG_FATAL("Corruption de la table de transposition détectée");
}
```

### 3. Configuration avancée

```c
#include "logger.h"

void configurer_logging() {
    // Changer le niveau de log manuellement
    logger_set_level(LOG_TRACE);  // Afficher tous les logs y compris TRACE
    
    // Désactiver la sortie sur stderr (mode silencieux)
    logger_set_quiet(true);
    
    // Rediriger les logs vers un fichier
    logger_add_file("logs/moteur.log", LOG_DEBUG);
}
```

## Exemples concrets du projet

### Dans search.c
```c
// Initialisation du moteur
LOG_DEBUG("=== INITIALISATION DU MOTEUR ===\n");
init_zobrist();
init_killer_moves();
LOG_DEBUG("=== MOTEUR PRÊT ===\n\n");

// Pendant la recherche
LOG_DEBUG("[NEGAMAX] ply=%d depth=%d eval=%d color=%s\n", 
          ply, depth, eval, color == WHITE ? "WHITE" : "BLACK");
```

### Dans transposition.c
```c
// Initialisation de la table de transposition
LOG_DEBUG("TT initialisée : %zu entrées (%zu bytes)\n", 
          nb_entries, total_bytes);

// Lors d'une collision
LOG_DEBUG("[TT] Collision : ancien hash=0x%llx remplacé\n", old_hash);
```

### Dans zobrist.c
```c
// Vérification de l'initialisation
LOG_DEBUG("Zobrist initialisé : %d clés non-nulles\n", count);
```

## Comportement selon le mode de compilation

### Mode DEBUG (`make debug`)
- Niveau par défaut : **LOG_DEBUG**
- Affiche : DEBUG, INFO, WARN, ERROR, FATAL
- Sortie : stderr (console)
- Exemple de sortie :
```
14:31:15 DEBUG Engine/search.c:21: === INITIALISATION DU MOTEUR ===
14:31:15 DEBUG Engine/zobrist.c:79: Zobrist initialisé : 849 clés non-nulles
14:31:15 INFO Engine/search.c:350: Recherche terminée : 15420 noeuds en 234 ms
```

### Mode RELEASE (`make release`)
- Niveau par défaut : **LOG_INFO**
- Affiche : INFO, WARN, ERROR, FATAL (pas de DEBUG)
- Sortie : stderr (console)
- Les logs DEBUG sont complètement supprimés (zéro impact performance)

## Bonnes pratiques

### ✅ À faire

```c
// Utiliser LOG_DEBUG pour les détails de l'algorithme
LOG_DEBUG("[SEARCH] Profondeur %d : %d coups générés", depth, move_count);

// Utiliser LOG_INFO pour les événements importants
LOG_INFO("Meilleur coup trouvé : %s (score: %d)", move_str, score);

// Utiliser LOG_WARN pour les situations anormales
LOG_WARN("Temps dépassé, arrêt anticipé de la recherche");

// Utiliser LOG_ERROR pour les erreurs récupérables
LOG_ERROR("Position FEN invalide : %s", fen_string);

// Formater les messages clairement
LOG_DEBUG("[MODULE] Description : param1=%d param2=%s", val1, val2);
```

### ❌ À éviter

```c
// Ne pas utiliser LOG_DEBUG pour des logs trop fréquents dans les boucles critiques
for (int i = 0; i < 1000000; i++) {
    LOG_DEBUG("Itération %d", i);  // ❌ Trop verbeux
}

// Ne pas utiliser LOG_FATAL pour des erreurs non critiques
if (move_count == 0) {
    LOG_FATAL("Aucun coup légal");  // ❌ Utiliser LOG_WARN ou LOG_INFO
}

// Ne pas oublier d'inclure le header
LOG_DEBUG("Test");  // ❌ Erreur de compilation sans #include "logger.h"
```

## Désactiver temporairement les logs

Si vous voulez désactiver tous les logs temporairement :

```c
logger_set_quiet(true);  // Désactive stderr
logger_set_level(LOG_FATAL + 1);  // Ne log que si niveau > FATAL (jamais)
```

## Logging dans les tests

Les tests unitaires voient aussi les logs en mode DEBUG :

```bash
$ make test
Running build_tests/test_board...
14:29:43 DEBUG Engine/zobrist.c:79: Zobrist initialisé : 849 clés non-nulles
tests/test_board.c:76:test_board_initial_position:PASS
```

Pour des tests silencieux, ajoutez dans votre test :
```c
void setUp(void) {
    logger_init();
    logger_set_quiet(true);  // Silence les logs pendant les tests
}
```

## Résumé rapide

| Macro | Quand l'utiliser | Exemple |
|-------|------------------|---------|
| `LOG_TRACE` | Traçage ultra-détaillé | Variables internes, flow d'exécution |
| `LOG_DEBUG` | Développement, débogage | États intermédiaires, décisions algorithmiques |
| `LOG_INFO` | Événements normaux importants | Fin de recherche, meilleur coup trouvé |
| `LOG_WARN` | Situations anormales OK | Table pleine, timeout approchant |
| `LOG_ERROR` | Erreurs récupérables | Allocation échouée, FEN invalide |
| `LOG_FATAL` | Erreurs critiques | Corruption mémoire, état impossible |

