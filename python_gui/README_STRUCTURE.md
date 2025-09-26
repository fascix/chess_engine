# Structure Modulaire du Chess Engine

## Aperçu de la nouvelle organisation

Le code a été réorganisé en modules séparés par responsabilité pour une meilleure lisibilité et maintenabilité.

## Structure des fichiers

```
python_gui/
├── main.py              # Point d'entrée principal (simplifié)
├── settings.py          # Configuration et constantes
├── support.py           # Fonctions utilitaires (images, plateau, etc.)
├── game_logic.py        # Logique de jeu et mouvements
├── gui.py              # Interface graphique et affichage
├── menu.py             # Tous les menus du jeu
├── game_modes.py       # Modes de jeu (2 joueurs, vs bot)
├── background.jpg      # Image du fond (pas très esthétique mais provisoire)
└── pieces/
    ├── b/              # Images des pièces nires
    └── w/              # Images des pièces blanches
```

## Description des modules

### `main.py` (Point d'entrée)

- Initialise Pygame
- Lance le menu principal
- Code minimal et propre

### `game_logic.py` (Logique de jeu)

- Gestion des mouvements de pièces (`handle_click`, `handle_drag`, `handle_drop`)
- Exécution des coups (`execute_move`)
- Threading pour le moteur d'IA (`engine_worker`, `start_engine_calculation`)
- Gestion des timers (`update_timers`, `is_time_up`)
- Variables globales du jeu (plateau, pièces sélectionnées, etc.)

### `gui.py` (Interface graphique)

- Affichage des timers (`draw_timer`)
- Rendu des pièces capturées avec grille (`draw_captured_pieces`)
- Statut du moteur (`draw_engine_status`)
- Messages d'échec et mat (`display_checkmate`)
- Menu de promotion des pions (`promote_pawn`)
- Fonction de rendu principal (`render_game`)

### `menu.py` (Menus)

- Menu principal (`main_menu`)
- Sélection du mode de jeu (`game_mode_menu`)
- Choix du timer (`timer_menu`)
- Sélection des couleurs (`color_choice_menu`)
- Menu de pause (`pause_menu`)
- Paramètres (`settings_menu`)

### `game_modes.py` (Modes de jeu)

- Mode 2 joueurs (`solo_game`)
- Mode contre l'ordinateur (`chess_engine`)
- Gestion des événements et boucles principales

### `support.py` (Utilitaires - inchangé)

- Chargement des images (`load_images`)
- Dessin du plateau (`draw_board`, `draw_pieces`)
- Mise en surbrillance (`highlight_selected_piece`, `highlight_legal_moves`)

## Améliorations implémentées

### ✅ Problèmes résolus

1. **Threading du moteur d'IA** : Plus de freeze pendant que l'ordinateur réfléchit
2. **Menu in-game** : Pause avec ESC, options de sortie
3. **Débordement des pièces capturées** : Système de grille avec lignes multiples
4. **Titres de menu dynamiques** : Chaque menu a son titre approprié
5. **Choix aléatoire des couleurs** : Option pour jouer blancs/noirs/aléatoire
6. **Structure modulaire** : Code organisé par responsabilité

### 🎮 Nouvelles fonctionnalités

- **Menu de pause** : ESC pendant le jeu pour pause/quitter
- **Choix des couleurs** : Menu pour sélectionner blanc/noir/aléatoire
- **Threading asynchrone** : L'interface reste responsive pendant le calcul du bot
- **Affichage amélioré** : Statut "L'ordinateur réfléchit..." pendant le calcul

### 🏗️ Architecture modulaire

- **Séparation claire des responsabilités** : Chaque module a un rôle défini
- **Facilité de maintenance** : Modifications localisées dans les modules appropriés
- **Réutilisabilité** : Fonctions réutilisables entre modules
- **Lisibilité** : Code plus organisé et facile à comprendre

## Usage

Pour lancer le jeu :

```bash
cd python_gui
python main.py
```
