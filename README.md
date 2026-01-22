# Chess Engine

Un moteur d'échecs UCI écrit en C avec une interface graphique Python.

## Caractéristiques

- **Moteur d'échecs UCI** : Implémentation complète du protocole UCI pour la compatibilité avec les interfaces d'échecs
- **Interface graphique** : GUI interactive développée avec Pygame
- **Recherche avancée** : Alpha-beta avec quiescence search, move ordering, et table de transposition
- **Évaluation** : Fonction d'évaluation basée sur le matériel, la structure de pions, et les tables pièce-carré
- **Hachage Zobrist** : Pour des mises à jour incrémentales rapides de la position
- **Tests** : Scripts de tests perft, UCI compliance, et fastchess pour mesurer les performances

## Structure du projet

```
.
├── Engine/              Code source du moteur en C
│   ├── board.c/h       Représentation de l'échiquier
│   ├── movegen.c/h     Génération de coups
│   ├── search.c/h      Algorithme de recherche
│   ├── evaluation.c/h  Fonction d'évaluation
│   ├── uci.c/h         Interface UCI
│   └── ...
├── GUI/                 Interface graphique Python
│   ├── main.py         Point d'entrée
│   ├── gui.py          Affichage de l'échiquier
│   ├── game_logic.py   Logique du jeu
│   └── ...
├── docs/                Documentation et diagrammes
├── tests/               Scripts de test
│   ├── perft_test.sh
│   ├── uci_compliance_test.sh
│   └── tests_fastchess.sh
└── Makefile             Build system
```

## Compilation

### Mode release (optimisé)
```bash
make release
```

### Mode debug (avec sanitizers)
```bash
make debug
```

### Nettoyer les builds
```bash
make clean
```

## Utilisation

### Moteur UCI seul
```bash
./chess_engine
```
Le moteur accepte les commandes UCI standard (uci, isready, position, go, quit, etc.)

### Interface graphique
```bash
cd GUI
python main.py
```

Dépendances Python : `pip install -r requirements.txt`

## Tests

### Tests perft (génération de coups)
```bash
./tests/perft_test.sh
```

### Tests de compliance UCI
```bash
./tests/uci_compliance_test.sh
```

### Tests de performance (fastchess)
```bash
./tests/tests_fastchess.sh
```

### Tests unitaires
Les tests unitaires utilisent le framework Unity pour tester les modules de base du moteur.

```bash
# Compiler et exécuter tous les tests
make test

# Compiler uniquement les tests
make build-tests

# Nettoyer les tests compilés
make clean-tests
```

Les tests couvrent actuellement :
- **test_board.c** : Tests de la représentation de l'échiquier et du parsing FEN
- **test_zobrist.c** : Tests du hachage Zobrist

Pour ajouter de nouveaux tests, créez un fichier `tests/test_*.c` qui utilise Unity. Les tests seront automatiquement découverts et compilés par le Makefile.

## Configuration

- **Makefile** : Options de compilation et optimisations
- **GUI/config.py** : Paramètres de l'interface graphique
- **tests/** : Configuration des tests

### Logging

Le moteur utilise la bibliothèque [log.c](https://github.com/rxi/log.c/) pour un système de logging flexible et configurable.

#### Configuration du logging en C
Le niveau de log par défaut est défini automatiquement :
- **Mode DEBUG** (`make debug`) : LOG_DEBUG et supérieur
- **Mode RELEASE** (`make release`) : LOG_INFO et supérieur

Les logs sont envoyés sur stderr par défaut. Vous pouvez configurer le logging dans votre code :

```c
#include "logger.h"

// Initialiser le système de logging
logger_init();

// Changer le niveau de log
logger_set_level(LOG_DEBUG);  // LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL

// Ajouter un fichier de log
logger_add_file("engine.log", LOG_DEBUG);

// Utiliser les macros de logging
LOG_DEBUG("Position évaluée : %d", score);
LOG_INFO("Recherche terminée en %d ms", time_ms);
LOG_WARN("Table de transposition pleine");
LOG_ERROR("Erreur lors de l'analyse : %s", error_msg);
```

#### Configuration du logging en Python (GUI)
L'interface graphique utilise le module `logging` standard de Python avec une configuration centralisée :

```python
from logging_config import setup_logging, get_logger

# Configurer le logging au démarrage
setup_logging(level=logging.INFO, log_to_file=True)

# Obtenir un logger pour votre module
logger = get_logger(__name__)

# Utiliser le logger
logger.info("Application démarrée")
logger.debug("Détails de debug")
logger.error("Une erreur s'est produite")
```

Les logs Python sont sauvegardés dans `logs/gui.log`.

## Exemples de fonctionnalités techniques

- Génération de coups légaux avec move ordering
- Recherche alpha-beta avec pruning
- Quiescence search pour éviter l'effet horizon
- Table de transposition pour éviter les recalculs
- Time management pour gérer le temps de réflexion
- Support des variantes d'échecs (via l'interface)

