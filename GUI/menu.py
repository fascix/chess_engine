"""
Module contenant toutes les fonctions de menu
"""
import pygame
import time
import random
from settings import *

def load_main_menu_background():
    """Charge l'image de fond principale avec fallback sur un fond uni."""
    try:
        bg = pygame.image.load('./assets/background6.png').convert()
        bg = pygame.transform.scale(bg, (WINDOW_SIZE, WINDOW_SIZE))
        return bg
    except Exception:
        # Fallback : dégradé simple sombre si l'image n'est pas trouvée
        surface = pygame.Surface((WINDOW_SIZE, WINDOW_SIZE))
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

    overlay = pygame.Surface((WINDOW_SIZE + 300, WINDOW_SIZE), pygame.SRCALPHA)
    overlay.fill(UI_OVERLAY_BG)

    running = True
    while running:
        screen.blit(overlay, (0, 0))
        
        title_text = font.render("PAUSE", True, UI_TEXT_PRIMARY)
        screen.blit(title_text, (WINDOW_SIZE // 2 - 50, 100))
        
        for i, option in enumerate(menu_options):
            if i == selected_index:
                text = font.render(f"> {option}", True, UI_ACCENT)
            else:
                text = font.render(option, True, UI_TEXT_SECONDARY)
            screen.blit(text, (WINDOW_SIZE // 4, 200 + i * 60))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
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

    title_font = pygame.font.Font(None, 96)
    font = pygame.font.Font(None, 48)
    small_font = pygame.font.Font(None, 28)

    menu_options = ["Jouer", "Réglages", "Quitter"]
    selected_index = 0

    running = True
    clock = pygame.time.Clock()
    while running:
        clock.tick(60)

        # Fond
        screen.blit(background_image, (0, 0))

        # Légère vignette sombre autour des bords
        vignette = pygame.Surface((WINDOW_SIZE, WINDOW_SIZE), pygame.SRCALPHA)
        vignette.fill((0, 0, 0, 80))
        screen.blit(vignette, (0, 0))

        # Menu options (boutons minimalistes)
        base_y = WINDOW_SIZE // 2
        spacing = 70
        for i, option in enumerate(menu_options):
            is_selected = (i == selected_index)

            # Arrière-plan de bouton
            btn_width, btn_height = 320, 50
            btn_x = (WINDOW_SIZE - btn_width) // 2
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
                    elif selected_index == 2:  # Quitter
                        pygame.quit()
                        exit()

def game_mode_menu():
    """Menu de sélection du mode de jeu."""
    screen = pygame.display.get_surface()
    pygame.display.set_caption("Sélection du mode de jeu")

    font = pygame.font.Font(None, 48)
    menu_options = ["Mode 2 joueurs", "Mode Chess Engine (bot)", "Mode Bot vs Bot", "Retour"]    
    selected_index = 0

    running = True
    clock = pygame.time.Clock()
    while running:
        clock.tick(60)
        screen.fill(UI_BG_COLOR)

        title = font.render("Modes de jeu", True, UI_TEXT_PRIMARY)
        title_rect = title.get_rect(center=(WINDOW_SIZE // 2, 120))
        screen.blit(title, title_rect)

        for i, option in enumerate(menu_options):
            is_selected = (i == selected_index)
            color = UI_ACCENT if is_selected else UI_TEXT_PRIMARY
            text = font.render(option if not is_selected else f"> {option}", True, color)
            screen.blit(text, (WINDOW_SIZE // 4, 200 + i * 60))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(menu_options)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(menu_options)
                elif event.key == pygame.K_RETURN:
                    if selected_index == 0:  # Solo
                        timer_menu("solo")
                    elif selected_index == 1:  # Bot
                        timer_menu("bot")
                    elif selected_index == 2:  # Bot vs Bot
                        timer_menu("bot_vs_bot")
                    elif selected_index == 3:  # Retour
                        main_menu()

def timer_menu(mode):
    """Choix du timer avant de commencer le jeu."""
    pygame.display.set_caption("Sélection du timer")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    options = ["10 min", "5 min"]
    selected_index = 0

    running = True
    clock = pygame.time.Clock()
    while running:
        clock.tick(60)
        screen.fill(UI_BG_COLOR)

        title = font.render("Durée de la partie", True, UI_TEXT_PRIMARY)
        title_rect = title.get_rect(center=(WINDOW_SIZE // 2, 120))
        screen.blit(title, title_rect)

        for i, option in enumerate(options):
            is_selected = (i == selected_index)
            color = UI_ACCENT if is_selected else UI_TEXT_PRIMARY
            text = font.render(option if not is_selected else f"> {option}", True, color)
            screen.blit(text, (WINDOW_SIZE // 3, 200 + i * 60))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP or event.key == pygame.K_DOWN:
                    selected_index = 1 - selected_index  # Alterner entre 5 et 10 min
                elif event.key == pygame.K_RETURN:
                    timer = 600 if selected_index == 0 else 300
                    if mode == "solo":
                        from game_modes import solo_game
                        solo_game(timer, True)
                    elif mode == "bot_vs_bot":
                        from game_modes import bot_vs_bot
                        bot_vs_bot(timer)
                    else:
                        color_choice_menu("bot", timer)
                    return

def color_choice_menu(mode, timer):
    """Menu pour choisir la couleur des pièces (uniquement pour le mode bot)."""
    pygame.display.set_caption("Sélection des couleurs")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    
    options = ["Jouer Blancs", "Jouer Noirs", "Aléatoire"]
    
    selected_index = 0
    clock = pygame.time.Clock()

    running = True
    while running:
        clock.tick(60)
        screen.fill(UI_BG_COLOR)

        title_text = font.render("Choisissez votre couleur:", True, UI_TEXT_PRIMARY)
        screen.blit(title_text, (WINDOW_SIZE // 4, 100))

        for i, option in enumerate(options):
            is_selected = (i == selected_index)
            color = UI_ACCENT if is_selected else UI_TEXT_PRIMARY
            text = font.render(option if not is_selected else f"> {option}", True, color)
            screen.blit(text, (WINDOW_SIZE // 4, 200 + i * 60))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP:
                    selected_index = (selected_index - 1) % len(options)
                elif event.key == pygame.K_DOWN:
                    selected_index = (selected_index + 1) % len(options)
                elif event.key == pygame.K_RETURN:
                    if selected_index == 2:  # Aléatoire
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
    """Affiche le menu des réglages (choix entre clic et drag & drop + bots)."""
    pygame.display.set_caption("Paramètres")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    
    import game_logic
    selected_index = 0
    clock = pygame.time.Clock()

    running = True
    while running:
        clock.tick(60)
        screen.fill(UI_BG_COLOR)

        title = font.render("Paramètres", True, UI_TEXT_PRIMARY)
        title_rect = title.get_rect(center=(WINDOW_SIZE // 2, 120))
        screen.blit(title, title_rect)

        options = [
            f"Drag & Drop {'(actif)' if game_logic.drag_mode else ''}",
            f"Mode Clic {'(actif)' if not game_logic.drag_mode else ''}",
            "Choisir Bot 1 vs Player",
            "Choisir Bot 2 vs Bot",
            "Retour"
        ]

        for i, option in enumerate(options):
            is_selected = (i == selected_index)
            color = UI_ACCENT if is_selected else UI_TEXT_PRIMARY
            text = font.render(option if not is_selected else f"> {option}", True, color)
            screen.blit(text, (WINDOW_SIZE // 4, 200 + i * 60))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
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
                        bot_selection_menu(1)
                    elif selected_index == 3:
                        bot_selection_menu(2)
                    elif selected_index == 4:
                        main_menu()
                        return

def bot_selection_menu(bot_number):
    """Menu pour choisir le bot (1 ou 2)."""
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

    running = True
    clock = pygame.time.Clock()
    while running:
        clock.tick(60)
        screen.fill(UI_BG_COLOR)

        title_text = font.render(f"Choisir Bot {bot_number}:", True, UI_TEXT_PRIMARY)
        screen.blit(title_text, (WINDOW_SIZE // 4, 100))

        for i, bot_name in enumerate(bot_list):
            bot_path = config.bot_engines[bot_name]
            display_path = bot_path if bot_path else "(vide)"
            
            is_selected = (i == selected_index)
            color = UI_ACCENT if is_selected else UI_TEXT_PRIMARY
            path_color = UI_TEXT_SECONDARY

            text = font.render(bot_name if not is_selected else f"> {bot_name}", True, color)
            path_text = small_font.render(display_path, True, path_color)

            screen.blit(text, (WINDOW_SIZE // 4, 200 + i * 80))
            screen.blit(path_text, (WINDOW_SIZE // 4 + 20, 230 + i * 80))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
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