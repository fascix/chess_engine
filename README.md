# 🏛️ Pallas - Chess Engine & GUI

**Pallas** est un projet de fin de licence de jeu d'échecs, il possède d'une part un moteur d'échecs écrit en **C** (Engine) et une interface graphique moderne développée en **Python** (GUI). 
Le nom Pallas fait référence à l'épithète de la déesse Athéna, symbolisant la stratégie et la sagesse guerrière.

---

## 📂 Structure du Projet

```
.
├── Engine/              # Code source du moteur en C
│   ├── board.c/h       # Représentation par Bitboards
│   ├── movegen.c/h     # Générateur de coups (126 tests perft OK)
│   ├── search.c/h      # Algorithmes de recherche (Alpha-Beta, PVS)
│   ├── evaluation.c/h  # Fonction d'évaluation (PeSTO)
│   ├── polyglot.c/h    # Support Opening Book (.bin)
│   ├── syzygy.c/h      # Support Tablebases de finales
│   └── uci.c/h         # Interface de communication UCI
├── GUI/                 # Interface graphique Python
│   ├── main.py         # Point d'entrée
│   ├── gui.py          # Rendu Pygame
│   ├── game_logic.py   # Gestion des événements et threading
│   └── assets/         # Ressources (images des pièces)
├── docs/                # Documentation technique détaillée
├── tests/               # Suite de tests (Perft, UCI, Unity)
└── Makefile             # Système de build automatisé
```

---

## 🚀 L'Engine (Pallas Engine)

La base de ce projet est un moteur d'échecs conforme au protocole **UCI**.

### Caractéristiques Techniques
- **Représentation du plateau** : Utilisation de **Bitboards** (64 bits) pour une manipulation ultra-rapide des positions et de la génération de coups.
- **Algorithme de Recherche** : 
    - **Alpha-Beta Pruning** avec **Iterative Deepening** pour une recherche de plus en plus profonde.
    - **PVS (Principal Variation Search)** pour optimiser l'exploration des meilleurs coups.
    - **Quiescence Search** avec MVV-LVA pour éliminer l'effet horizon lors des captures tactiques.
    - **Transposition Table (TT)** avec hachage **Zobrist** pour mémoriser et réutiliser les analyses précédentes.
    - **Move Ordering** avancé : Hash Move, Captures (SEE/MVV-LVA), Killer Moves, History Heuristic.
    - **Élagage (Pruning)** : Null Move Pruning (NMP), Reverse Futility Pruning (RFP), Late Move Reductions (LMR).
- **Évaluation** : Implémentation de la fonction d'évaluation **PeSTO**, utilisant des tables Piece-Square (PST) optimisées pour le milieu de jeu et la finale avec une interpolation fluide.
- **Extensions** :
    - **Opening Book** : Support des livres au format **PolyGlot** (`.bin`) avec sélection pondérée.
    - **Endgame Tablebases** : Interface pour le support des **Syzygy Tablebases**.

### Compilation du moteur
Le moteur peut être compilé dans différentes versions selon les besoins :
```bash
make pallas          # Version complète (avec Book et Tablebases)
make pallas-no-book  # Version sans livre d'ouverture
make pallas-no-tb    # Version sans tablebases de fin de partie
make pallas-pure     # Version "Pure" (uniquement l'algorithme de recherche)
make debug           # Version avec symboles de debug et sanitizers
```

---

## 🎨 L'Interface Graphique (Pallas GUI)

L'interface utilisateur permet de jouer contre Pallas de manière intuitive.

### Features du GUI
- **Moteur de rendu** : Développé avec **Pygame** pour une fluidité optimale et un rendu propre.
- **Interactivité** : Support du Drag & Drop, mise en évidence des coups légaux, dernier coup joué et cases attaquées.
- **Gestion de partie** : 
    - Chargement et sauvegarde de positions via chaînes **FEN**.
    - Historique complet des coups de la partie.
    - Pendule intégrée supportant les incréments (Time Control UCI).
- **Analyse en temps réel** : Affichage de l'évaluation (en centipawns), de la profondeur de recherche et du NPS (Nodes Per Second).
- **Threading** : Le moteur tourne dans un thread séparé, garantissant que l'interface reste réactive même pendant les calculs intensifs.

---

## 📖 Tutoriel : Comment utiliser Pallas ?

### 1. Installation Automatique (Recommandé)
Le moyen le plus simple d'installer Pallas et ses dépendances est d'utiliser le script d'installation :
```bash
# Rendre le script exécutable (si nécessaire)
chmod +x install.sh

# Lancer l'installation
./install.sh
```
Ce script vérifiera vos dépendances (C, Python, Make), installera les bibliothèques Python nécessaires et compilera le moteur.

### 2. Installation Manuelle
Si vous préférez installer chaque composant séparément :

#### A. Dépendances Python
Assurez-vous d'avoir Python 3.x installé.
```bash
pip install -r requirements.txt
```

#### B. Compilation de l'Engine
```bash
# Compiler la version optimisée
make pallas
```

### 3. Lancer le jeu
```bash
# Lancer l'interface graphique
cd GUI
python main.py
```

### 4. Utilisation en mode console (UCI)
Pallas peut être utilisé directement en ligne de commande ou intégré dans d'autres interfaces (comme Arena ou CuteChess).
```bash
./pallas
uci
isready
position startpos
go depth 10
```

---

## 🛠️ Développement et Tests

Le projet inclut une batterie de tests rigoureux pour garantir la stabilité et la force de jeu :

### Tests unitaires (Unity)
Le framework **Unity** permet de tester les différents composants (Board, Zobrist, MoveGen).
```bash
make test
```

### Validation de la génération de coups (Perft)
Le moteur passe avec succès plus de 125 tests Perft standard, garantissant qu'aucun coups illégaux (roque, en passant, promotion) n'est violée.
```bash
bash tests/run_all_tests.sh
```

### Benchmarking (FastChess)
Pour mesurer la progression de l'engine, des scripts permettent de lancer des matchs automatisés entre différentes versions.
```bash
cd tests
bash tests_fastchess.sh
```

---

## 📊 Puissance et Performance

Pallas a été optimisé pour offrir une expérience de jeu compétitive sur du matériel standard.

- **Vitesse de calcul** : ~500,000 - 600,000 nœuds par seconde (NPS) sur un processeur moderne.
- **Profondeur de recherche** : Atteint généralement une profondeur de 12 à 18 demi-coups en conditions de jeu standard (blitz).
- **ELO Approximatif** : `        ` (En cours de benchmarking via tests STS et matchs contre d'autres moteurs).

---

## 📝 Configuration et Logging

### Logging en C
Le système de log est configurable via `Engine/logger.h`. En mode `make debug`, les logs sont très complet pour faciliter le traçage des bugs de recherche.

### Logging en Python
L'interface graphique génère des logs dans `logs/gui.log` pour diagnostiquer les problèmes de communication avec l'engine.

---

## 📜 Licence
Ce projet est sous licence MIT. Voir le fichier `LICENSE` pour plus de détails.

