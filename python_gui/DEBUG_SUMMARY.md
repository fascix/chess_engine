# Résumé du Debug - Problème Drag and Drop case a1

## Problème Initial

La pièce sur la case **a1** (tour blanche en bas à gauche) ne peut pas être draggée en mode drag and drop.

- La pièce disparaît dès qu'elle est sélectionnée
- Le problème n'existe qu'en mode drag and drop (pas en mode clic)
- Le problème n'affecte que la case a1

## Découvertes via les Logs

### 1. Comportement observé

```
[CLICK FIN] selected_piece après traitement: a1
[RENDER] selected_piece=None, drag_mode=True, dragged_pos=(7, 707)
```

**Constat** : `selected_piece` est correctement défini à `a1` à la fin du CLICK, mais devient `None` dès le premier RENDER.

### 2. Absence d'événements DRAG

Entre le CLICK et le DROP, il n'y a **AUCUN** événement `[DRAG]` pour a1, contrairement aux autres cases (b1, etc.) où on voit plusieurs événements DRAG.

Cela signifie : L'utilisateur clique et relâche très rapidement, MAIS le problème est que la pièce disparaît même sans être draguée.

## Cause Racine Identifiée

### Problème d'Import Python

Dans `gui.py`, la fonction `render_game()` utilisait :

```python
from game_logic import selected_piece, legal_moves, dragged_pos, ...
```

**Problème** : Cet import crée une **copie locale de la référence** au moment de l'import. Quand `selected_piece` est modifié dans `game_logic.py` avec `selected_piece = square`, la variable locale dans `render_game` ne voit PAS ce changement.

En Python, `from module import variable` crée une liaison au moment de l'import, pas une liaison dynamique.

## Solutions Tentées

### 1. ✅ Limitation des coordonnées dans `get_square_from_pos()`

**Fichier** : `support.py`

```python
col = max(0, min(col, 7))
row = max(0, min(row, 7))
```

Empêche les débordements quand la souris dépasse les limites (x=800 ou y<0).

### 2. ✅ Vérifications dans `handle_click()` et `handle_drop()`

**Fichier** : `game_logic.py`

- Vérification que la souris est dans les limites : `0 <= mouse_x < WINDOW_SIZE`
- Retour anticipé si hors limites

### 3. ✅ Séparation de la logique drag/click dans `handle_click()`

**Fichier** : `game_logic.py`

```python
dragging = drag_mode  # Au lieu de dragging = True
```

En mode drag, ne gère PAS la désélection dans handle_click (laissé à handle_drop).

### 4. ✅ Protection dans `handle_drop()`

**Fichier** : `game_logic.py`

- Vérifications multiples : `if not drag_mode`, `if not dragging`, `if not selected_piece`
- Annulation propre si drop hors limites

### 5. ✅ Limitation de `dragged_pos` dans `handle_drag()`

**Fichier** : `game_logic.py`

```python
drag_x = max(-TILE_SIZE // 2, min(drag_x, WINDOW_SIZE - TILE_SIZE // 2))
drag_y = max(-TILE_SIZE // 2, min(drag_y, WINDOW_SIZE - TILE_SIZE // 2))
```

Garde la pièce draggée au moins partiellement visible.

### 6. ✅ Changement de l'import dans `render_game()`

**Fichier** : `gui.py`

```python
# AVANT (NE FONCTIONNE PAS)
from game_logic import selected_piece, legal_moves, dragged_pos, drag_mode, white_timer, black_timer

# APRÈS (DEVRAIT FONCTIONNER)
import game_logic
# Puis utiliser : game_logic.selected_piece, game_logic.legal_moves, etc.
```

### 7. ✅ Suppression du cache Python

Commande : `rm -rf __pycache__`
Pour s'assurer que les anciennes versions ne sont pas utilisées.

## Pourquoi ça ne fonctionne TOUJOURS PAS ?

### Hypothèse 1 : Le cache n'est pas réellement vidé

Les fichiers `.pyc` sont peut-être recréés avec l'ancienne version.

**Test à faire** :

```bash
cd python_gui
find . -name "*.pyc" -delete
find . -name "__pycache__" -type d -exec rm -rf {} +
python main.py
```

### Hypothèse 2 : Import circulaire ou problème de module

`render_game()` dans `gui.py` importe `game_logic`, qui pourrait créer un import circulaire si `game_logic` importe aussi `gui`.

**Vérification dans le code** :

- `game_logic.py` ligne 178 : `from gui import promote_pawn, images`
- C'est un **import circulaire** !

**Solution possible** : Déplacer `promote_pawn` hors de `gui.py` ou faire l'import à l'intérieur de la fonction.

### Hypothèse 3 : render_game() est appelé avec l'ancien module

Si `game_modes.py` importe `gui` au début du fichier, il pourrait avoir une référence à l'ancienne version de `render_game()`.

**Vérification** : `game_modes.py` ligne 6 : `import gui`

### Hypothèse 4 : Problème spécifique à la case a1

La case a1 est à la position (0, 7) en coordonnées chess. Peut-être y a-t-il une condition spéciale quelque part dans le code qui traite le 0 comme False ?

## Prochaines Étapes à Tester

### 1. Forcer le rechargement des modules

```python
# Dans game_modes.py, au début
import importlib
import gui
import game_logic
importlib.reload(gui)
importlib.reload(game_logic)
```

### 2. Résoudre l'import circulaire

Déplacer `promote_pawn` dans un fichier séparé (ex: `promotions.py`) ou faire l'import local :

```python
def execute_move(board, move):
    # ...
    if needs_promotion:
        from gui import promote_pawn, images  # Import local
        promote_pawn(...)
```

### 3. Debug avec print direct dans render_game

```python
def render_game(screen, board, font, player_is_white=True):
    import game_logic
    print(f"DEBUG render_game: id(game_logic.selected_piece) = {id(game_logic.selected_piece)}")
    print(f"DEBUG render_game: game_logic.selected_piece = {game_logic.selected_piece}")
```

Et dans handle_click :

```python
def handle_click(event, board):
    global selected_piece
    # ...
    selected_piece = square
    print(f"DEBUG handle_click: id(selected_piece) = {id(selected_piece)}")
    print(f"DEBUG handle_click: selected_piece = {selected_piece}")
```

Comparer les `id()` pour voir si c'est la même variable.

### 4. Tester en mode 2 joueurs (solo_game)

Au lieu du mode `chess_engine`, tester avec `solo_game` pour éliminer les problèmes liés au moteur.

### 5. Ajouter un log dans draw_pieces

```python
def draw_pieces(screen, board, images, selected_piece=None, player_is_white=True):
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            # Ne pas dessiner la pièce en cours de drag sur l'échiquier
            if selected_piece is not None and square == selected_piece:
                print(f"DEBUG draw_pieces: Skip drawing piece at {chess.square_name(square)}")
                continue
```

## Fichiers Modifiés

1. **python_gui/game_logic.py**

   - `handle_click()` : Ajout vérifications limites, séparation drag/click
   - `handle_drag()` : Limitation dragged_pos
   - `handle_drop()` : Vérifications multiples, protection hors limites

2. **python_gui/support.py**

   - `get_square_from_pos()` : Limitation col/row entre 0-7

3. **python_gui/gui.py**

   - `render_game()` : Changement import (`import game_logic` au lieu de `from game_logic import ...`)

4. **python_gui/game_modes.py**
   - Ajout logs EVENT (commentés maintenant)

## Conclusion

Le problème persiste malgré toutes les corrections. La cause est probablement liée à :

1. Un import circulaire entre `gui.py` et `game_logic.py`
2. Le cache Python qui n'est pas correctement vidé
3. Une référence à l'ancienne version du module quelque part

**Recommandation** : Tester les prochaines étapes ci-dessus, en particulier résoudre l'import circulaire et ajouter des logs avec `id()` pour tracer les références aux variables.
