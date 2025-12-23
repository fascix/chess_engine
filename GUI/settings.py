import pygame

# --- Layout constants pour l'écran de jeu ---
NAVBAR_HEIGHT = 60
RIGHT_PANEL_WIDTH = 400

# TILE_SIZE est maintenant calculé dynamiquement dans gui.py

# Couleurs du plateau
WHITE = (238, 238, 210)
BLACK = (118, 150, 86)

# Couleurs alternatives pour l'échiquier (customisables)
BOARD_COLORS = {
    'classic': [(238, 238, 210), (118, 150, 86)],
    'brown': [(240, 217, 181), (181, 136, 99)],
    'blue': [(222, 227, 230), (140, 162, 173)],
    'green': [(234, 240, 206), (119, 153, 84)],
    'gray': [(230, 230, 230), (100, 100, 100)]
}
current_board_theme = 'classic'

# --- Layout constants pour l'écran de jeu ---
NAVBAR_HEIGHT = 60
RIGHT_PANEL_WIDTH = 400
BOARD_OFFSET_X = 50  # Décalage depuis la gauche
BOARD_OFFSET_Y = NAVBAR_HEIGHT + 20  # Décalage depuis le haut (après la navbar)

# --- Direction artistique GUI (indie sombre / premium) ---

# Fond global très sombre, légèrement bleuté
UI_BG_COLOR = (5, 5, 10)          # #05050A

# Couleurs de texte
UI_TEXT_PRIMARY = (242, 233, 228)  # #F2E9E4 - blanc cassé
UI_TEXT_SECONDARY = (201, 173, 167)  # #C9ADA7 - gris rosé

# Couleurs d'accent
UI_ACCENT = (244, 162, 89)        # #F4A259 - orange doré
UI_ACCENT_DARK = (140, 82, 40)    # version plus sombre (hover / arrière plan)

# Accent froid complémentaire (bleu pétrole)
UI_ACCENT_COOL = (31, 79, 102)    # #1F4F66

# Pour boutons / overlays
UI_BUTTON_BG = (15, 15, 25)       # fond de boutons
UI_BUTTON_HOVER_BG = (40, 30, 50) # hover plus lumineux
UI_BUTTON_BORDER = UI_ACCENT
UI_BUTTON_DISABLED = (80, 80, 90)

# Pour fond d'overlay (pause, etc.)
UI_OVERLAY_BG = (0, 0, 0, 200)  # utilisé avec surface alpha

# Couleurs spécifiques (danger, etc.)
UI_DANGER = (215, 38, 61)        # #D7263D

# Panel background
UI_PANEL_BG = (20, 20, 30)        # fond des panneaux
UI_PANEL_BORDER = (50, 50, 60)    # bordure des panneaux

# --- Menu constants (DRY principle) ---
MENU_PANEL_BG_COLOR = (30, 24, 38, 220)  # Standard panel background with alpha
MENU_CORNER_SIZE = 8  # Size of corner ornaments on menu boxes
MENU_BOX_SPACING = 18  # Spacing between menu options
MENU_TITLE_Y_OFFSET = 60  # Default Y offset for menu titles
MENU_OPTIONS_START_Y = 120  # Default Y position for first option