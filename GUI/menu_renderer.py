"""
Module pour les fonctions de rendu de menu réutilisables (principe DRY)
"""
import pygame
from settings import *


def draw_menu_panel(screen, panel_width_ratio=0.35, panel_height_ratio=0.5):
    """
    Dessine un panneau de menu standard avec overlay.
    
    Args:
        screen: Surface pygame
        panel_width_ratio: Ratio de largeur par rapport à l'écran
        panel_height_ratio: Ratio de hauteur par rapport à l'écran
        
    Returns:
        tuple: (panel_x, panel_y, panel_width, panel_height)
    """
    screen_width, screen_height = screen.get_width(), screen.get_height()
    
    # Overlay
    overlay = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA)
    overlay.fill(UI_OVERLAY_BG)
    screen.blit(overlay, (0, 0))
    
    # Panel dimensions
    panel_width = int(screen_width * panel_width_ratio)
    panel_height = int(screen_height * panel_height_ratio)
    panel_x = (screen_width - panel_width) // 2
    panel_y = (screen_height - panel_height) // 2
    
    # Panel background
    panel_rect = pygame.Rect(panel_x, panel_y, panel_width, panel_height)
    panel_surf = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
    panel_surf.fill(MENU_PANEL_BG_COLOR)
    screen.blit(panel_surf, (panel_x, panel_y))
    
    # Panel borders
    pygame.draw.rect(screen, UI_ACCENT_DARK, panel_rect, width=6)
    inner_rect = panel_rect.inflate(-12, -12)
    pygame.draw.rect(screen, UI_ACCENT, inner_rect, width=2)
    
    return panel_x, panel_y, panel_width, panel_height


def draw_menu_title(screen, title_text, panel_y, y_offset=None):
    """
    Dessine un titre de menu avec ombre.
    
    Args:
        screen: Surface pygame
        title_text: Texte du titre
        panel_y: Position Y du panneau
        y_offset: Décalage depuis le haut du panneau (utilise MENU_TITLE_Y_OFFSET par défaut)
    """
    if y_offset is None:
        y_offset = MENU_TITLE_Y_OFFSET
        
    screen_width = screen.get_width()
    title_font = pygame.font.Font(None, 72)
    
    title_shadow = title_font.render(title_text, True, (0, 0, 0))
    title_render = title_font.render(title_text, True, UI_ACCENT)
    title_rect = title_render.get_rect(center=(screen_width // 2, panel_y + y_offset))
    
    screen.blit(title_shadow, title_rect.move(3, 3))
    screen.blit(title_render, title_rect)


def draw_menu_option_box(screen, box_x, box_y, box_w, box_h, option_text, 
                          is_selected, font_size=36):
    """
    Dessine une option de menu dans un style cohérent.
    
    Args:
        screen: Surface pygame
        box_x, box_y: Position de la boîte
        box_w, box_h: Dimensions de la boîte
        option_text: Texte de l'option
        is_selected: Si l'option est sélectionnée
        font_size: Taille de la police
    """
    box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
    
    # Highlight if selected
    if is_selected:
        hover_s = pygame.Surface((box_w, box_h), pygame.SRCALPHA)
        hover_s.fill((UI_ACCENT[0], UI_ACCENT[1], UI_ACCENT[2], 35))
        screen.blit(hover_s, (box_x, box_y))
    
    # Border
    pygame.draw.rect(screen, UI_ACCENT, box_rect, width=2)
    
    # Corner ornaments
    pygame.draw.rect(screen, UI_ACCENT, 
                    (box_x - MENU_CORNER_SIZE // 2, box_y + box_h // 2 - MENU_CORNER_SIZE // 2, 
                     MENU_CORNER_SIZE, MENU_CORNER_SIZE))
    pygame.draw.rect(screen, UI_ACCENT, 
                    (box_x + box_w - MENU_CORNER_SIZE // 2, box_y + box_h // 2 - MENU_CORNER_SIZE // 2, 
                     MENU_CORNER_SIZE, MENU_CORNER_SIZE))
    
    # Text
    opt_font = pygame.font.Font(None, font_size)
    text_color = UI_ACCENT if is_selected else UI_TEXT_SECONDARY
    text = opt_font.render(option_text, True, text_color)
    text_rect = text.get_rect(center=box_rect.center)
    screen.blit(text, text_rect)


def calculate_menu_layout(panel_x, panel_y, panel_width, panel_height, 
                          num_options, start_y_offset=None, spacing=None):
    """
    Calcule les dimensions et positions pour les options de menu.
    
    Args:
        panel_x, panel_y: Position du panneau
        panel_width, panel_height: Dimensions du panneau
        num_options: Nombre d'options
        start_y_offset: Décalage du début des options (utilise MENU_OPTIONS_START_Y par défaut)
        spacing: Espacement entre les options (utilise MENU_BOX_SPACING par défaut)
        
    Returns:
        tuple: (box_w, box_h, box_x, start_y)
    """
    if start_y_offset is None:
        start_y_offset = MENU_OPTIONS_START_Y
    if spacing is None:
        spacing = MENU_BOX_SPACING
        
    box_w = int(panel_width * 0.8)
    box_h = max(int(panel_height * 0.08), 44)
    box_x = panel_x + int(panel_width * 0.1)
    start_y = panel_y + start_y_offset
    
    return box_w, box_h, box_x, start_y


def handle_menu_navigation(event, selected_index, num_options):
    """
    Gère la navigation au clavier dans un menu.
    
    Args:
        event: Event pygame
        selected_index: Index actuel
        num_options: Nombre total d'options
        
    Returns:
        int: Nouvel index sélectionné
    """
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_UP:
            return (selected_index - 1) % num_options
        elif event.key == pygame.K_DOWN:
            return (selected_index + 1) % num_options
    
    return selected_index


def check_menu_click(mouse_pos, box_x, box_y, box_w, box_h, num_options, spacing=None):
    """
    Vérifie si un clic de souris est sur une option de menu.
    
    Args:
        mouse_pos: Position de la souris (x, y)
        box_x, box_y: Position de la première option
        box_w, box_h: Dimensions d'une option
        num_options: Nombre d'options
        spacing: Espacement entre les options (utilise MENU_BOX_SPACING par défaut)
        
    Returns:
        int: Index de l'option cliquée, ou None si aucune option n'est cliquée
    """
    if spacing is None:
        spacing = MENU_BOX_SPACING
        
    mouse_x, mouse_y = mouse_pos
    
    for i in range(num_options):
        option_y = box_y + i * (box_h + spacing)
        option_rect = pygame.Rect(box_x, option_y, box_w, box_h)
        if option_rect.collidepoint(mouse_x, mouse_y):
            return i
    
    return None


def update_hover_selection(mouse_pos, box_x, box_y, box_w, box_h, num_options, spacing=None):
    """
    Met à jour la sélection basée sur la position de la souris (hover).
    
    Args:
        mouse_pos: Position de la souris (x, y)
        box_x, box_y: Position de la première option
        box_w, box_h: Dimensions d'une option
        num_options: Nombre d'options
        spacing: Espacement entre les options (utilise MENU_BOX_SPACING par défaut)
        
    Returns:
        int: Index de l'option survolée, ou None si aucune option n'est survolée
    """
    return check_menu_click(mouse_pos, box_x, box_y, box_w, box_h, num_options, spacing)
