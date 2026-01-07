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

## Configuration

- **Makefile** : Options de compilation et optimisations
- **GUI/config.py** : Paramètres de l'interface graphique
- **tests/** : Configuration des tests

## Exemples de fonctionnalités techniques

- Génération de coups légaux avec move ordering
- Recherche alpha-beta avec pruning
- Quiescence search pour éviter l'effet horizon
- Table de transposition pour éviter les recalculs
- Time management pour gérer le temps de réflexion
- Support des variantes d'échecs (via l'interface)

