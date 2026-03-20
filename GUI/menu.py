"""
Module contenant toutes les fonctions de menu
"""
import pygame
import time
import random
import os
from settings import *
from menu_renderer import (draw_menu_panel, draw_menu_title, draw_menu_option_box, 
                           calculate_menu_layout, check_menu_click, update_hover_selection)

def load_main_menu_background():
    """Charge l'image de fond principale avec fallback sur un fond uni."""
    try:
        current_dir = os.path.dirname(os.path.abspath(__file__))
        bg_path = os.path.join(current_dir, 'assets', 'background6.png')
        bg = pygame.image.load(bg_path).convert()
        screen = pygame.display.get_surface()
        if screen:
            bg = pygame.transform.smoothscale(bg, (screen.get_width(), screen.get_height()))
        return bg
    except Exception:
        # Fallback : dégradé simple sombre si l'image n'est pas trouvée
        screen = pygame.display.get_surface()
        if screen:
            surface = pygame.Surface((screen.get_width(), screen.get_height()))
        else:
            surface = pygame.Surface((800, 800))
        surface.fill(UI_BG_COLOR)
        return surface

def pause_menu():
    """Affiche le menu de pause pendant le jeu."""
    from game_logic import game_paused, current_turn_start_time
    from game_logic import game_paused as gp
    
    # Utilisation d'une variable locale pour éviter les conflits
    game_paused_state = True
    pause_start_time = time.time()
    
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    menu_options = ["Reprendre", "Retour au menu principal", "Quitter le jeu"]
    selected_index = 0

    running = True
    while running:
        # Draw panel
        panel_x, panel_y, panel_width, panel_height = draw_menu_panel(screen, 0.33, 0.5)
        
        # Title (with custom font size for PAUSED)
        paused_font = pygame.font.Font(None, 84)
        title_text = paused_font.render("PAUSED", True, UI_ACCENT)
        title_shadow = paused_font.render("PAUSED", True, (0, 0, 0))
        title_rect = title_text.get_rect(center=(screen.get_width() // 2, panel_y + 60))
        screen.blit(title_shadow, title_rect.move(3, 3))
        screen.blit(title_text, title_rect)

        # Calculate layout
        box_w, box_h, box_x, start_y = calculate_menu_layout(
            panel_x, panel_y, panel_width, panel_height, len(menu_options)
        )
        box_h = 56  # Custom height for pause menu
        box_w = panel_width - 80
        box_x = panel_x + 40
        
        # Draw options
        for i, option in enumerate(menu_options):
            box_y = start_y + i * (box_h + 18)
            draw_menu_option_box(screen, box_x, box_y, box_w, box_h, option, i == selected_index)

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                hovered = update_hover_selection(event.pos, box_x, start_y, box_w, box_h, len(menu_options))
                if hovered is not None:
                    selected_index = hovered
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si une option a été cliquée
                clicked = check_menu_click(event.pos, box_x, start_y, box_w, box_h, len(menu_options))
                if clicked is not None:
                    selected_index = clicked
                    if selected_index == 0:  # Reprendre
                        pause_duration = time.time() - pause_start_time
                        import game_logic
                        if game_logic.current_turn_start_time:
                            game_logic.current_turn_start_time += pause_duration
                        game_logic.game_paused = False
                        return "resume"
                    elif selected_index == 1:  # Retour au menu principal
                        return "main_menu"
                    elif selected_index == 2:  # Quitter le jeu
                        pygame.quit()
                        exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(menu_options)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(menu_options)
                elif event.key == pygame.K_RETURN or event.key == pygame.K_SPACE:
                    if selected_index == 0:  # Reprendre
                        pause_duration = time.time() - pause_start_time
                        import game_logic
                        if game_logic.current_turn_start_time:
                            game_logic.current_turn_start_time += pause_duration
                        game_logic.game_paused = False
                        return "resume"
                    elif selected_index == 1:  # Retour au menu principal
                        return "main_menu"
                    elif selected_index == 2:  # Quitter le jeu
                        pygame.quit()
                        exit()
                elif event.key == pygame.K_ESCAPE:
                    pause_duration = time.time() - pause_start_time
                    import game_logic
                    if game_logic.current_turn_start_time:
                        game_logic.current_turn_start_time += pause_duration
                    game_logic.game_paused = False
                    return "resume"

def main_menu():
    """Affiche le menu principal avec une DA sombre / indie."""
    screen = pygame.display.get_surface()
    pygame.display.set_caption("Menu Principal")

    background_image = load_main_menu_background()

    # Fonts
    title_font = pygame.font.Font(None, 96)
    # Main title text — use explicit string "Chess Engine"
    title_text = "Chess Engine"
    # Render title with shadow and accent
    title_shadow = title_font.render(title_text, True, (0, 0, 0))
    title_surface = title_font.render(title_text, True, UI_ACCENT)
    font = pygame.font.Font(None, 48)
    small_font = pygame.font.Font(None, 28)

    menu_options = ["Jouer", "Réglages", "Logs", "Quitter"]
    selected_index = 0

    running = True
    clock = pygame.time.Clock()
    while running:
        clock.tick(60)

        # Fond
        screen.blit(background_image, (0, 0))
        # Draw main title with shadow and subtle panel
        screen = pygame.display.get_surface()
        screen_width, screen_height = screen.get_width(), screen.get_height()
        base_y = screen_height // 2
        base_x = screen_width // 2
        title_shadow_rect = title_shadow.get_rect(center=(base_x + 3, int(base_y - screen_height * 0.25) + 3))
        title_rect = title_surface.get_rect(center=(base_x, int(base_y - screen_height * 0.25)))

        # subtle glowing panel behind title
        panel_w = int(screen_width * 0.6)
        panel_h = title_rect.height + 30
        panel_x = (screen_width - panel_w) // 2
        panel_y = title_rect.top - 15
        glow_rect = pygame.Rect(panel_x, panel_y, panel_w, panel_h)
        s = pygame.Surface((glow_rect.width, glow_rect.height), pygame.SRCALPHA)
        s.fill((UI_ACCENT[0], UI_ACCENT[1], UI_ACCENT[2], 30))
        screen.blit(s, (glow_rect.x, glow_rect.y))

        # shadow then title
        screen.blit(title_shadow, title_shadow_rect)
        screen.blit(title_surface, title_rect)

        # Menu options (boutons centrés)
        spacing = int(screen_height * 0.09)
        panel_width = screen_width * 0.35
        panel_height = screen_height * 0.5
        btn_width = int(panel_width * 0.8)
        btn_height = max(int(panel_height * 0.08), 44)
        for i, option in enumerate(menu_options):
            is_selected = (i == selected_index)
            btn_x = base_x - (btn_width // 2)
            btn_y = base_y + i * spacing
            rect = pygame.Rect(btn_x, btn_y, btn_width, btn_height)
            bg_color = UI_BUTTON_HOVER_BG if is_selected else UI_BUTTON_BG
            pygame.draw.rect(screen, bg_color, rect, border_radius=10)
            border_color = UI_ACCENT if is_selected else UI_BUTTON_BORDER
            pygame.draw.rect(screen, border_color, rect, width=2, border_radius=10)
            # Texte
            color = UI_ACCENT if is_selected else UI_TEXT_PRIMARY
            label = font.render(option, True, color)
            label_rect = label.get_rect(center=rect.center)
            screen.blit(label, label_rect)

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                mouse_pos = event.pos
                for i, option in enumerate(menu_options):
                    btn_x = base_x - (btn_width // 2)
                    btn_y = base_y + i * spacing
                    rect = pygame.Rect(btn_x, btn_y, btn_width, btn_height)
                    if rect.collidepoint(mouse_pos):
                        selected_index = i
                        break
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si un bouton a été cliqué
                mouse_pos = event.pos
                for i, option in enumerate(menu_options):
                    btn_x = base_x - (btn_width // 2)
                    btn_y = base_y + i * spacing
                    rect = pygame.Rect(btn_x, btn_y, btn_width, btn_height)
                    if rect.collidepoint(mouse_pos):
                        selected_index = i
                        # Exécuter l'action correspondante
                        if selected_index == 0:  # Jouer
                            game_mode_menu()
                        elif selected_index == 1:  # Réglages
                            settings_menu()
                        elif selected_index == 2:  # Logs
                            from logs_menu import logs_menu
                            logs_menu()
                        elif selected_index == 3:  # Quitter
                            pygame.quit()
                            exit()
                        break
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(menu_options)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(menu_options)
                elif event.key == pygame.K_RETURN:
                    if selected_index == 0:  # Jouer
                        game_mode_menu()
                    elif selected_index == 1:  # Réglages
                        settings_menu()
                    elif selected_index == 2:  # Logs
                        from logs_menu import logs_menu
                        logs_menu()
                    elif selected_index == 3:  # Quitter
                        pygame.quit()
                        exit()

def game_mode_menu():
    """Menu de sélection du mode de jeu (DA pause_menu)."""
    screen = pygame.display.get_surface()
    pygame.display.set_caption("Sélection du mode de jeu")
    font = pygame.font.Font(None, 48)
    menu_options = ["Mode 2 joueurs", "Mode Chess Engine (bot)", "Mode Bot vs Bot", "Retour"]
    selected_index = 0
    clock = pygame.time.Clock()
    running = True
    
    while running:
        clock.tick(60)
        
        # Draw panel
        panel_x, panel_y, panel_width, panel_height = draw_menu_panel(screen, 0.35, 0.5)
        
        # Draw title
        draw_menu_title(screen, "Modes de jeu", panel_y, 60)
        
        # Calculate layout
        box_w, box_h, box_x, start_y = calculate_menu_layout(
            panel_x, panel_y, panel_width, panel_height, len(menu_options)
        )
        
        # Draw options
        for i, option in enumerate(menu_options):
            box_y = start_y + i * (box_h + 18)
            draw_menu_option_box(screen, box_x, box_y, box_w, box_h, option, i == selected_index)

        pygame.display.flip()
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                hovered = update_hover_selection(event.pos, box_x, start_y, box_w, box_h, len(menu_options))
                if hovered is not None:
                    selected_index = hovered
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si une option a été cliquée
                clicked = check_menu_click(event.pos, box_x, start_y, box_w, box_h, len(menu_options))
                if clicked is not None:
                    selected_index = clicked
                    # Exécuter l'action correspondante
                    if selected_index == 0:
                        timer_menu("solo")
                        return
                    elif selected_index == 1:
                        timer_menu("bot")
                        return
                    elif selected_index == 2:
                        timer_menu("bot_vs_bot")
                        return
                    elif selected_index == 3:
                        main_menu()
                        return
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(menu_options)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(menu_options)
                elif event.key == pygame.K_RETURN:
                    if selected_index == 0:
                        timer_menu("solo")
                        return
                    elif selected_index == 1:
                        timer_menu("bot")
                        return
                    elif selected_index == 2:
                        timer_menu("bot_vs_bot")
                        return
                    elif selected_index == 3:
                        main_menu()
                        return
                elif event.key == pygame.K_ESCAPE:
                    main_menu()
                    return

def timer_menu(mode):
    """Choix du timer avant de commencer le jeu (DA pause_menu)."""
    pygame.display.set_caption("Sélection du timer")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    options = ["10 min", "5 min"]
    selected_index = 0
    clock = pygame.time.Clock()
    running = True
    
    while running:
        clock.tick(60)
        
        # Draw panel
        panel_x, panel_y, panel_width, panel_height = draw_menu_panel(screen, 0.35, 0.5)
        
        # Draw title
        draw_menu_title(screen, "Durée de la partie", panel_y, 60)
        
        # Calculate layout
        box_w, box_h, box_x, start_y = calculate_menu_layout(
            panel_x, panel_y, panel_width, panel_height, len(options)
        )
        
        # Draw options
        for i, option in enumerate(options):
            box_y = start_y + i * (box_h + 18)
            draw_menu_option_box(screen, box_x, box_y, box_w, box_h, option, i == selected_index)

        pygame.display.flip()
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                hovered = update_hover_selection(event.pos, box_x, start_y, box_w, box_h, len(options))
                if hovered is not None:
                    selected_index = hovered
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si une option a été cliquée
                clicked = check_menu_click(event.pos, box_x, start_y, box_w, box_h, len(options))
                if clicked is not None:
                    selected_index = clicked
                    timer = 600 if selected_index == 0 else 300
                    if mode == "solo":
                        from game_modes import solo_game
                        solo_game(timer, True)
                        return
                    elif mode == "bot_vs_bot":
                        from game_modes import bot_vs_bot
                        bot_vs_bot(timer)
                        return
                    else:
                        color_choice_menu("bot", timer)
                        return
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP or event.key == pygame.K_DOWN:
                    selected_index = 1 - selected_index
                elif event.key == pygame.K_RETURN:
                    timer = 600 if selected_index == 0 else 300
                    if mode == "solo":
                        from game_modes import solo_game
                        solo_game(timer, True)
                        return
                    elif mode == "bot_vs_bot":
                        from game_modes import bot_vs_bot
                        bot_vs_bot(timer)
                        return
                    else:
                        color_choice_menu("bot", timer)
                        return
                elif event.key == pygame.K_ESCAPE:
                    game_mode_menu()
                    return

def color_choice_menu(mode, timer):
    """Menu pour choisir la couleur des pièces (DA pause_menu)."""
    pygame.display.set_caption("Sélection des couleurs")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    options = ["Jouer Blancs", "Jouer Noirs", "Aléatoire"]
    selected_index = 0
    clock = pygame.time.Clock()
    running = True
    
    while running:
        clock.tick(60)
        
        # Draw panel
        panel_x, panel_y, panel_width, panel_height = draw_menu_panel(screen, 0.35, 0.5)
        
        # Draw title (custom font size)
        title_font = pygame.font.Font(None, 64)
        title_text = "Choisissez votre couleur"
        title_shadow = title_font.render(title_text, True, (0, 0, 0))
        title_render = title_font.render(title_text, True, UI_ACCENT)
        title_rect = title_render.get_rect(center=(screen.get_width() // 2, panel_y + 60))
        screen.blit(title_shadow, title_rect.move(3, 3))
        screen.blit(title_render, title_rect)
        
        # Calculate layout
        box_w, box_h, box_x, start_y = calculate_menu_layout(
            panel_x, panel_y, panel_width, panel_height, len(options)
        )
        
        # Draw options
        for i, option in enumerate(options):
            box_y = start_y + i * (box_h + 18)
            draw_menu_option_box(screen, box_x, box_y, box_w, box_h, option, i == selected_index)

        pygame.display.flip()
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                hovered = update_hover_selection(event.pos, box_x, start_y, box_w, box_h, len(options))
                if hovered is not None:
                    selected_index = hovered
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si une option a été cliquée
                clicked = check_menu_click(event.pos, box_x, start_y, box_w, box_h, len(options))
                if clicked is not None:
                    selected_index = clicked
                    if selected_index == 2:
                        player_is_white = random.choice([True, False])
                    else:
                        player_is_white = (selected_index == 0)
                    from game_modes import chess_engine
                    chess_engine(timer, player_is_white)
                    return
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(options)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(options)
                elif event.key == pygame.K_RETURN:
                    if selected_index == 2:
                        player_is_white = random.choice([True, False])
                    else:
                        player_is_white = (selected_index == 0)
                    from game_modes import chess_engine
                    chess_engine(timer, player_is_white)
                    return
                elif event.key == pygame.K_ESCAPE:
                    timer_menu(mode)
                    return

def settings_menu():
    """Affiche le menu des réglages (DA pause_menu)."""
    pygame.display.set_caption("Paramètres")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    import game_logic
    selected_index = 0
    clock = pygame.time.Clock()
    running = True
    while running:
        clock.tick(60)
        screen_width, screen_height = screen.get_width(), screen.get_height()
        overlay = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA)
        overlay.fill(UI_OVERLAY_BG)
        screen.blit(overlay, (0, 0))

        panel_width = int(screen_width * 0.35)
        panel_height = int(screen_height * 0.5 + 40)
        panel_x = (screen_width - panel_width) // 2
        panel_y = (screen_height - panel_height) // 2
        panel_rect = pygame.Rect(panel_x, panel_y, panel_width, panel_height)
        panel_surf = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel_surf.fill((30, 24, 38, 220))
        screen.blit(panel_surf, (panel_x, panel_y))
        pygame.draw.rect(screen, UI_ACCENT_DARK, panel_rect, width=6)
        inner_rect = panel_rect.inflate(-12, -12)
        pygame.draw.rect(screen, UI_ACCENT, inner_rect, width=2)

        # Title with shadow
        title_font = pygame.font.Font(None, 72)
        title_text = "Paramètres"
        title_shadow = title_font.render(title_text, True, (0,0,0))
        title_render = title_font.render(title_text, True, UI_ACCENT)
        title_rect = title_render.get_rect(center=(screen.get_width() // 2, panel_y + 60))
        screen.blit(title_shadow, title_rect.move(3, 3))
        screen.blit(title_render, title_rect)

        from settings import current_board_theme
        options = [
            f"Drag & Drop {'(actif)' if game_logic.drag_mode else ''}",
            f"Mode Clic {'(actif)' if not game_logic.drag_mode else ''}",
            f"Couleur échiquier: {current_board_theme}",
            "Choisir Bot 1 vs Player",
            "Choisir Bot 2 vs Bot",
            "Retour"
        ]
        # Buttons
        box_w = int(panel_width * 0.8)
        box_h = max(int(panel_height * 0.08), 44)
        box_x = panel_x + int(panel_width * 0.1)
        for i, option in enumerate(options):
            box_y = panel_y + 120 + i * (box_h + 18)
            box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
            if i == selected_index:
                hover_s = pygame.Surface((box_w, box_h), pygame.SRCALPHA)
                hover_s.fill((UI_ACCENT[0], UI_ACCENT[1], UI_ACCENT[2], 35))
                screen.blit(hover_s, (box_x, box_y))
            pygame.draw.rect(screen, UI_ACCENT, box_rect, width=2)
            corner_size = 8
            pygame.draw.rect(screen, UI_ACCENT, (box_x - corner_size // 2, box_y + box_h // 2 - corner_size // 2, corner_size, corner_size))
            pygame.draw.rect(screen, UI_ACCENT, (box_x + box_w - corner_size // 2, box_y + box_h // 2 - corner_size // 2, corner_size, corner_size))
            opt_font = pygame.font.Font(None, 36)
            if i == selected_index:
                text = opt_font.render(option, True, UI_ACCENT)
            else:
                text = opt_font.render(option, True, UI_TEXT_SECONDARY)
            text_rect = text.get_rect(center=box_rect.center)
            screen.blit(text, text_rect)

        pygame.display.flip()
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                mouse_pos = event.pos
                for i, option in enumerate(options):
                    box_y = panel_y + 120 + i * (box_h + 18)
                    box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
                    if box_rect.collidepoint(mouse_pos):
                        selected_index = i
                        break
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si une option a été cliquée
                mouse_pos = event.pos
                for i, option in enumerate(options):
                    box_y = panel_y + 120 + i * (box_h + 18)
                    box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
                    if box_rect.collidepoint(mouse_pos):
                        selected_index = i
                        # Exécuter l'action correspondante
                        if selected_index == 0:
                            game_logic.drag_mode = True
                        elif selected_index == 1:
                            game_logic.drag_mode = False
                        elif selected_index == 2:
                            board_color_menu()
                            return
                        elif selected_index == 3:
                            bot_selection_menu(1)
                            return
                        elif selected_index == 4:
                            bot_selection_menu(2)
                            return
                        elif selected_index == 5:
                            main_menu()
                            return
                        break
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(options)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(options)
                elif event.key == pygame.K_RETURN:
                    if selected_index == 0:
                        game_logic.drag_mode = True
                    elif selected_index == 1:
                        game_logic.drag_mode = False
                    elif selected_index == 2:
                        board_color_menu()
                        return
                    elif selected_index == 3:
                        bot_selection_menu(1)
                        return
                    elif selected_index == 4:
                        bot_selection_menu(2)
                        return
                    elif selected_index == 5:
                        main_menu()
                        return
                elif event.key == pygame.K_ESCAPE:
                    main_menu()
                    return

def board_color_menu():
    """Menu pour choisir la couleur de l'échiquier (DA pause_menu)."""
    import settings
    pygame.display.set_caption("Couleur de l'échiquier")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    theme_list = list(settings.BOARD_COLORS.keys())
    selected_index = 0
    
    # Trouver l'index du thème actuel
    if settings.current_board_theme in theme_list:
        selected_index = theme_list.index(settings.current_board_theme)
    
    clock = pygame.time.Clock()
    running = True
    while running:
        clock.tick(60)
        screen_width, screen_height = screen.get_width(), screen.get_height()
        overlay = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA)
        overlay.fill(UI_OVERLAY_BG)
        screen.blit(overlay, (0, 0))

        # Panel
        panel_width = int(screen_width * 0.5)
        panel_height = min(screen_height - 120, 120 + len(theme_list) * 90 + 40)
        panel_x = (screen_width - panel_width) // 2
        panel_y = (screen_height - panel_height) // 2
        panel_rect = pygame.Rect(panel_x, panel_y, panel_width, panel_height)
        panel_surf = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel_surf.fill((30, 24, 38, 220))
        screen.blit(panel_surf, (panel_x, panel_y))
        pygame.draw.rect(screen, UI_ACCENT_DARK, panel_rect, width=6)
        inner_rect = panel_rect.inflate(-12, -12)
        pygame.draw.rect(screen, UI_ACCENT, inner_rect, width=2)

        # Title with shadow
        title_font = pygame.font.Font(None, 64)
        title_text = "Couleur de l'échiquier"
        title_shadow = title_font.render(title_text, True, (0,0,0))
        title_render = title_font.render(title_text, True, UI_ACCENT)
        title_rect = title_render.get_rect(center=(screen.get_width() // 2, panel_y + 50))
        screen.blit(title_shadow, title_rect.move(3, 3))
        screen.blit(title_render, title_rect)

        # Buttons (theme names)
        box_w = int(panel_width * 0.8)
        box_h = max(int(panel_height * 0.08), 44)
        box_x = panel_x + int(panel_width * 0.1)
        for i, theme_name in enumerate(theme_list):
            box_y = panel_y + 100 + i * 90
            box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
            if i == selected_index:
                hover_s = pygame.Surface((box_w, box_h), pygame.SRCALPHA)
                hover_s.fill((UI_ACCENT[0], UI_ACCENT[1], UI_ACCENT[2], 35))
                screen.blit(hover_s, (box_x, box_y))
            pygame.draw.rect(screen, UI_ACCENT, box_rect, width=2)
            corner_size = 8
            pygame.draw.rect(screen, UI_ACCENT, (box_x - corner_size // 2, box_y + box_h // 2 - corner_size // 2, corner_size, corner_size))
            pygame.draw.rect(screen, UI_ACCENT, (box_x + box_w - corner_size // 2, box_y + box_h // 2 - corner_size // 2, corner_size, corner_size))
            
            # Theme name
            opt_font = pygame.font.Font(None, 36)
            if i == selected_index:
                text = opt_font.render(theme_name.capitalize(), True, UI_ACCENT)
            else:
                text = opt_font.render(theme_name.capitalize(), True, UI_TEXT_SECONDARY)
            text_rect = text.get_rect(midleft=(box_x + 18, box_y + box_h // 2))
            screen.blit(text, text_rect)

        pygame.display.flip()
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                mouse_pos = event.pos
                for i, theme_name in enumerate(theme_list):
                    box_y = panel_y + 100 + i * 90
                    box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
                    if box_rect.collidepoint(mouse_pos):
                        selected_index = i
                        break
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si une option a été cliquée
                mouse_pos = event.pos
                for i, theme_name in enumerate(theme_list):
                    box_y = panel_y + 100 + i * 90
                    box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
                    if box_rect.collidepoint(mouse_pos):
                        selected_index = i
                        settings.current_board_theme = theme_list[selected_index]
                        settings_menu()
                        return
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(theme_list)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(theme_list)
                elif event.key == pygame.K_RETURN:
                    settings.current_board_theme = theme_list[selected_index]
                    settings_menu()
                    return
                elif event.key == pygame.K_ESCAPE:
                    settings_menu()
                    return

def bot_selection_menu(bot_number):
    """Menu pour choisir le bot (DA pause_menu)."""
    import config
    pygame.display.set_caption(f"Sélection Bot {bot_number}")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    small_font = pygame.font.Font(None, 32)
    bot_list = list(config.bot_engines.keys())
    selected_index = 0
    current_bot = config.selected_bot1 if bot_number == 1 else config.selected_bot2
    if current_bot in bot_list:
        selected_index = bot_list.index(current_bot)
    clock = pygame.time.Clock()
    running = True
    while running:
        clock.tick(60)
        screen_width, screen_height = screen.get_width(), screen.get_height()
        overlay = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA)
        overlay.fill(UI_OVERLAY_BG)
        screen.blit(overlay, (0, 0))

        # Panel
        panel_width = int(screen_width * 0.5)
        panel_height = min(screen_height - 120, 120 + len(bot_list) * 90 + 40)
        panel_x = (screen_width - panel_width) // 2
        panel_y = (screen_height - panel_height) // 2
        panel_rect = pygame.Rect(panel_x, panel_y, panel_width, panel_height)
        panel_surf = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel_surf.fill((30, 24, 38, 220))
        screen.blit(panel_surf, (panel_x, panel_y))
        pygame.draw.rect(screen, UI_ACCENT_DARK, panel_rect, width=6)
        inner_rect = panel_rect.inflate(-12, -12)
        pygame.draw.rect(screen, UI_ACCENT, inner_rect, width=2)

        # Title with shadow
        title_font = pygame.font.Font(None, 64)
        title_text = f"Choisir Bot {bot_number}"
        title_shadow = title_font.render(title_text, True, (0,0,0))
        title_render = title_font.render(title_text, True, UI_ACCENT)
        title_rect = title_render.get_rect(center=(screen.get_width() // 2, panel_y + 50))
        screen.blit(title_shadow, title_rect.move(3, 3))
        screen.blit(title_render, title_rect)

        # Buttons (bot names)
        box_w = int(panel_width * 0.8)
        box_h = max(int(panel_height * 0.08), 44)
        box_x = panel_x + int(panel_width * 0.1)
        for i, bot_name in enumerate(bot_list):
            box_y = panel_y + 100 + i * 90
            box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
            if i == selected_index:
                hover_s = pygame.Surface((box_w, box_h), pygame.SRCALPHA)
                hover_s.fill((UI_ACCENT[0], UI_ACCENT[1], UI_ACCENT[2], 35))
                screen.blit(hover_s, (box_x, box_y))
            pygame.draw.rect(screen, UI_ACCENT, box_rect, width=2)
            corner_size = 8
            pygame.draw.rect(screen, UI_ACCENT, (box_x - corner_size // 2, box_y + box_h // 2 - corner_size // 2, corner_size, corner_size))
            pygame.draw.rect(screen, UI_ACCENT, (box_x + box_w - corner_size // 2, box_y + box_h // 2 - corner_size // 2, corner_size, corner_size))
            # Bot name
            opt_font = pygame.font.Font(None, 36)
            if i == selected_index:
                text = opt_font.render(bot_name, True, UI_ACCENT)
            else:
                text = opt_font.render(bot_name, True, UI_TEXT_SECONDARY)
            text_rect = text.get_rect(midleft=(box_x + 18, box_y + box_h // 2))
            screen.blit(text, text_rect)

        pygame.display.flip()
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEMOTION:
                # Mettre à jour la sélection basée sur le survol de la souris
                mouse_pos = event.pos
                for i, bot_name in enumerate(bot_list):
                    box_y = panel_y + 100 + i * 90
                    box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
                    if box_rect.collidepoint(mouse_pos):
                        selected_index = i
                        break
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Vérifier si une option a été cliquée
                mouse_pos = event.pos
                for i, bot_name in enumerate(bot_list):
                    box_y = panel_y + 100 + i * 90
                    box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
                    if box_rect.collidepoint(mouse_pos):
                        selected_index = i
                        if bot_number == 1:
                            config.set_bot1(bot_list[selected_index])
                        else:
                            config.set_bot2(bot_list[selected_index])
                        settings_menu()
                        return
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(bot_list)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(bot_list)
                elif event.key == pygame.K_RETURN:
                    if bot_number == 1:
                        config.set_bot1(bot_list[selected_index])
                    else:
                        config.set_bot2(bot_list[selected_index])
                    settings_menu()
                    return
                elif event.key == pygame.K_ESCAPE:
                    settings_menu()
                    return

import game_logger

def game_history_menu():
    """Affiche l'historique des 5 dernières parties."""
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 36)
    small_font = pygame.font.Font(None, 28)
    
    games = game_logger.get_recent_games()
    
    running = True
    while running: 
        screen.fill(UI_BG_COLOR)
        
        # Titre
        title = font.render("Historique des parties", True, UI_ACCENT)
        screen.blit(title, (50, 50))
        
        # Afficher les parties
        y = 120
        for i, game_info in enumerate(games, 1):
            summary = game_logger.format_game_summary(game_info)
            text = small_font.render(f"{i}. {summary}", True, UI_TEXT_PRIMARY)
            screen.blit(text, (50, y))
            y += 40
        
        # Instructions
        instruction = small_font.render("Appuyez sur ESC pour revenir", True, UI_TEXT_SECONDARY)
        screen.blit(instruction, (50, screen.get_height() - 60))
        
        pygame.display.flip()
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False