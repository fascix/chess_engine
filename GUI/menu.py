"""
Module contenant toutes les fonctions de menu
"""
import pygame
import time
import random
from settings import *

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

    overlay = pygame.Surface((WINDOW_SIZE + 300, WINDOW_SIZE))
    overlay.set_alpha(200)
    overlay.fill((0, 0, 0))

    running = True
    while running:
        screen.blit(overlay, (0, 0))
        
        title_text = font.render("PAUSE", True, (255, 255, 255))
        screen.blit(title_text, (WINDOW_SIZE // 2 - 50, 100))
        
        for i, option in enumerate(menu_options):
            color = (255, 255, 255)
            if i == selected_index:
                text = font.render(f"> {option}", True, (255, 255, 0))
            else:
                text = font.render(option, True, color)
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
                        # Ajuster le timer pour exclure le temps de pause
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
    """Affiche le menu principal."""
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Menu Principal")

    font = pygame.font.Font(None, 48)
    menu_options = ["Jouer", "Réglages", "Quitter"]
    selected_index = 0

    running = True
    while running:
        # Charger l'image de fond
        background_image = pygame.image.load('./assets/background.jpg')
        background_image = pygame.transform.scale(background_image, (WINDOW_SIZE, WINDOW_SIZE))
        screen.blit(background_image, (0, 0))

        for i, option in enumerate(menu_options):
            color = pygame.Color("white")
            if i == selected_index:
                text = font.render(f"> {option}", True, (255, 255, 0))
            else:
                text = font.render(option, True, color)
            screen.blit(text, (WINDOW_SIZE // 3, 150 + i * 60))
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
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Sélection du mode de jeu")

    font = pygame.font.Font(None, 48)
    menu_options = ["Mode 2 joueurs", "Mode Chess Engine (bot)", "Retour"]
    selected_index = 0

    running = True
    while running:
        screen.fill((0, 0, 0))

        for i, option in enumerate(menu_options):
            color = pygame.Color("white")
            if i == selected_index:
                text = font.render(f"> {option}", True, (255, 255, 0))
            else:
                text = font.render(option, True, color)
            screen.blit(text, (WINDOW_SIZE // 4, 150 + i * 60))

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
                    elif selected_index == 2:  # Retour
                        main_menu()

def timer_menu(mode):
    """Choix du timer avant de commencer le jeu."""
    pygame.display.set_caption("Sélection du timer")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    options = ["10 min", "5 min"]
    selected_index = 0

    running = True
    while running:
        screen.fill((0, 0, 0))

        for i, option in enumerate(options):
            color = (255, 255, 255)
            if i == selected_index:
                text = font.render(f"> {option}", True, (255, 255, 0))
            else:
                text = font.render(option, True, color)

            screen.blit(text, (WINDOW_SIZE // 3, 150 + i * 60))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_UP or event.key == pygame.K_DOWN:
                    selected_index = 1 - selected_index  # Alterner entre 5 et 10 min
                elif event.key == pygame.K_RETURN:
                    timer = 600 if selected_index == 0 else 300  # 10 min = 600 sec, 5 min = 300 sec
                    if mode == "solo":
                        from game_modes import solo_game
                        solo_game(timer, True)  # Mode 2 joueurs, les blancs commencent toujours
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

    running = True
    while running:
        screen.fill((0, 0, 0))

        title_text = font.render("Choisissez votre couleur:", True, (255, 255, 255))
        screen.blit(title_text, (WINDOW_SIZE // 4, 100))

        for i, option in enumerate(options):
            color = (255, 255, 255)
            if i == selected_index:
                text = font.render(f"> {option}", True, (255, 255, 0))
            else:
                text = font.render(option, True, color)

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
                        player_is_white = selected_index == 0  # Blancs = True, Noirs = False
                    
                    from game_modes import chess_engine
                    chess_engine(timer, player_is_white)
                    return
                elif event.key == pygame.K_ESCAPE:
                    timer_menu(mode)
                    return

def settings_menu():
    """Affiche le menu des réglages (choix entre clic et drag & drop)."""
    pygame.display.set_caption("Paramètres")
    screen = pygame.display.get_surface()
    font = pygame.font.Font(None, 48)
    
    import game_logic
    selected_index = 0

    running = True
    while running:
        screen.fill((0, 0, 0))

        # Mise à jour des options en fonction du mode actif
        options = [
            f"Drag & Drop {'(actif)' if game_logic.drag_mode else ''}",
            f"Mode Clic {'(actif)' if not game_logic.drag_mode else ''}",
            "Retour"
        ]

        for i, option in enumerate(options):
            color = (255, 255, 255)
            if i == selected_index:
                text = font.render(f"> {option}", True, (255, 255, 0))
            else:
                text = font.render(option, True, color)

            screen.blit(text, (WINDOW_SIZE // 4, 150 + i * 60))

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
                        main_menu()
                        return
