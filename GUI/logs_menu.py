"""
Module pour afficher le menu des logs de parties
"""
import pygame
from settings import *
import game_logger


def logs_menu():
    """Affiche le menu des logs avec l'historique des 5 dernières parties."""
    screen = pygame.display.get_surface()
    pygame.display.set_caption("Historique des parties")
    
    # Récupérer les parties récentes
    games = game_logger.get_recent_games()
    selected_index = 0
    
    clock = pygame.time.Clock()
    running = True
    
    while running:
        clock.tick(60)
        screen_width, screen_height = screen.get_width(), screen.get_height()
        
        # Overlay
        overlay = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA)
        overlay.fill(UI_OVERLAY_BG)
        screen.blit(overlay, (0, 0))
        
        # Panel dimensions
        panel_width = int(screen_width * 0.7)
        panel_height = int(screen_height * 0.8)
        panel_x = (screen_width - panel_width) // 2
        panel_y = (screen_height - panel_height) // 2
        
        # Panel background
        panel_rect = pygame.Rect(panel_x, panel_y, panel_width, panel_height)
        panel_surf = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel_surf.fill((30, 24, 38, 220))
        screen.blit(panel_surf, (panel_x, panel_y))
        
        # Panel borders
        pygame.draw.rect(screen, UI_ACCENT_DARK, panel_rect, width=6)
        inner_rect = panel_rect.inflate(-12, -12)
        pygame.draw.rect(screen, UI_ACCENT, inner_rect, width=2)
        
        # Title
        title_font = pygame.font.Font(None, 72)
        title_text = "Historique des parties"
        title_shadow = title_font.render(title_text, True, (0, 0, 0))
        title_render = title_font.render(title_text, True, UI_ACCENT)
        title_rect = title_render.get_rect(center=(screen_width // 2, panel_y + 60))
        screen.blit(title_shadow, title_rect.move(3, 3))
        screen.blit(title_render, title_rect)
        
        current_y = panel_y + 120
        
        # Display games
        if not games:
            no_games_font = pygame.font.Font(None, 36)
            no_games_text = no_games_font.render("Aucune partie sauvegardée", True, UI_TEXT_SECONDARY)
            no_games_rect = no_games_text.get_rect(center=(screen_width // 2, screen_height // 2))
            screen.blit(no_games_text, no_games_rect)
        else:
            game_font = pygame.font.Font(None, 32)
            info_font = pygame.font.Font(None, 24)
            
            for i, game_info in enumerate(games):
                # Game box
                box_w = int(panel_width * 0.85)
                box_h = 90
                box_x = panel_x + int(panel_width * 0.075)
                box_y = current_y + i * (box_h + 15)
                
                # Skip if outside panel
                if box_y + box_h > panel_y + panel_height - 100:
                    break
                
                box_rect = pygame.Rect(box_x, box_y, box_w, box_h)
                
                # Highlight selected
                if i == selected_index:
                    hover_s = pygame.Surface((box_w, box_h), pygame.SRCALPHA)
                    hover_s.fill((UI_ACCENT[0], UI_ACCENT[1], UI_ACCENT[2], 35))
                    screen.blit(hover_s, (box_x, box_y))
                
                # Border
                pygame.draw.rect(screen, UI_ACCENT if i == selected_index else UI_PANEL_BORDER, box_rect, width=2)
                
                # Game number
                number_text = game_font.render(f"#{i+1}", True, UI_ACCENT if i == selected_index else UI_TEXT_SECONDARY)
                screen.blit(number_text, (box_x + 15, box_y + 10))
                
                # Game details
                details_x = box_x + 70
                
                # Date
                date_text = info_font.render(f"Date: {game_info['date']}", True, UI_TEXT_PRIMARY)
                screen.blit(date_text, (details_x, box_y + 10))
                
                # Players
                players_text = info_font.render(
                    f"{game_info['white']} vs {game_info['black']}", 
                    True, UI_TEXT_PRIMARY
                )
                screen.blit(players_text, (details_x, box_y + 35))
                
                # Result
                result_color = UI_ACCENT if game_info['result'] != '*' else UI_TEXT_SECONDARY
                result_text = info_font.render(f"Résultat: {game_info['result']}", True, result_color)
                screen.blit(result_text, (details_x, box_y + 60))
                
                # Time control (right side)
                time_text = info_font.render(f"Temps: {game_info['time_control']}", True, UI_TEXT_SECONDARY)
                time_rect = time_text.get_rect(right=box_x + box_w - 15, centery=box_y + box_h // 2)
                screen.blit(time_text, time_rect)
        
        # Instructions at bottom
        instructions_y = panel_y + panel_height - 60
        instructions_font = pygame.font.Font(None, 28)
        
        if games:
            instruction1 = instructions_font.render("↑↓ Naviguer   ENTRÉE Voir la partie   ESC Retour", True, UI_TEXT_SECONDARY)
        else:
            instruction1 = instructions_font.render("ESC Retour au menu", True, UI_TEXT_SECONDARY)
        
        instruction_rect = instruction1.get_rect(center=(screen_width // 2, instructions_y))
        screen.blit(instruction1, instruction_rect)
        
        pygame.display.flip()
        
        # Event handling
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif games:
                    if event.key == pygame.K_UP:
                        selected_index = (selected_index - 1) % len(games)
                    elif event.key == pygame.K_DOWN:
                        selected_index = (selected_index + 1) % len(games)
                    elif event.key == pygame.K_RETURN:
                        # View game details
                        view_game_details(games[selected_index])


def view_game_details(game_info):
    """Affiche les détails d'une partie spécifique."""
    screen = pygame.display.get_surface()
    
    # Load the game from file
    game = game_logger.load_game_pgn(game_info['filepath'])
    if not game:
        return
    
    # Get all moves
    moves = []
    node = game
    temp_board = game.board()
    while node.variations:
        next_node = node.variation(0)
        move = next_node.move
        san_move = temp_board.san(move)
        moves.append(san_move)
        temp_board.push(move)
        node = next_node
    
    clock = pygame.time.Clock()
    running = True
    scroll_offset = 0
    
    while running:
        clock.tick(60)
        screen_width, screen_height = screen.get_width(), screen.get_height()
        
        # Overlay
        overlay = pygame.Surface((screen_width, screen_height), pygame.SRCALPHA)
        overlay.fill(UI_OVERLAY_BG)
        screen.blit(overlay, (0, 0))
        
        # Panel
        panel_width = int(screen_width * 0.6)
        panel_height = int(screen_height * 0.8)
        panel_x = (screen_width - panel_width) // 2
        panel_y = (screen_height - panel_height) // 2
        
        panel_rect = pygame.Rect(panel_x, panel_y, panel_width, panel_height)
        panel_surf = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel_surf.fill((30, 24, 38, 220))
        screen.blit(panel_surf, (panel_x, panel_y))
        
        pygame.draw.rect(screen, UI_ACCENT_DARK, panel_rect, width=6)
        inner_rect = panel_rect.inflate(-12, -12)
        pygame.draw.rect(screen, UI_ACCENT, inner_rect, width=2)
        
        # Title
        title_font = pygame.font.Font(None, 48)
        title_text = "Détails de la partie"
        title_shadow = title_font.render(title_text, True, (0, 0, 0))
        title_render = title_font.render(title_text, True, UI_ACCENT)
        title_rect = title_render.get_rect(center=(screen_width // 2, panel_y + 40))
        screen.blit(title_shadow, title_rect.move(2, 2))
        screen.blit(title_render, title_rect)
        
        # Game info
        info_font = pygame.font.Font(None, 28)
        move_font = pygame.font.Font(None, 24)
        current_y = panel_y + 80
        
        # Display headers
        info_lines = [
            f"Date: {game_info['date']}",
            f"Blancs: {game_info['white']}",
            f"Noirs: {game_info['black']}",
            f"Résultat: {game_info['result']}",
            f"Contrôle du temps: {game_info['time_control']}"
        ]
        
        for line in info_lines:
            text = info_font.render(line, True, UI_TEXT_PRIMARY)
            screen.blit(text, (panel_x + 30, current_y))
            current_y += 30
        
        # Separator
        pygame.draw.line(screen, UI_PANEL_BORDER, 
                        (panel_x + 20, current_y + 10), 
                        (panel_x + panel_width - 20, current_y + 10), 2)
        current_y += 25
        
        # Moves title
        moves_title = info_font.render("Coups:", True, UI_ACCENT)
        screen.blit(moves_title, (panel_x + 30, current_y))
        current_y += 35
        
        # Display moves in two columns
        moves_display_start_y = current_y
        moves_per_column = 15
        col_width = (panel_width - 60) // 2
        
        for i in range(0, len(moves), 2):
            move_num = (i // 2) + 1
            white_move = moves[i] if i < len(moves) else ""
            black_move = moves[i + 1] if i + 1 < len(moves) else ""
            
            row_in_display = i // 2
            if row_in_display < scroll_offset:
                continue
            if row_in_display - scroll_offset >= moves_per_column:
                break
            
            y_pos = moves_display_start_y + (row_in_display - scroll_offset) * 25
            
            move_line = f"{move_num}. {white_move}"
            if black_move:
                move_line += f"  {black_move}"
            
            move_text = move_font.render(move_line, True, UI_TEXT_SECONDARY)
            screen.blit(move_text, (panel_x + 40, y_pos))
        
        # Instructions
        instruction_font = pygame.font.Font(None, 24)
        instruction_text = "↑↓ Défiler   ESC Retour"
        instruction_render = instruction_font.render(instruction_text, True, UI_TEXT_SECONDARY)
        instruction_rect = instruction_render.get_rect(center=(screen_width // 2, panel_y + panel_height - 30))
        screen.blit(instruction_render, instruction_rect)
        
        pygame.display.flip()
        
        # Event handling
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_UP:
                    scroll_offset = max(0, scroll_offset - 1)
                elif event.key == pygame.K_DOWN:
                    max_scroll = max(0, (len(moves) + 1) // 2 - moves_per_column)
                    scroll_offset = min(max_scroll, scroll_offset + 1)
