import pygame

Window_Height = 800
Window_Width = 850
WINDOW_SIZE = min(Window_Height, Window_Width)

TILE_SIZE = WINDOW_SIZE // 8

# Couleurs du plateau
WHITE = (238, 238, 210)
BLACK = (118, 150, 86)

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