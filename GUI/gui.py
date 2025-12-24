"""
Module contenant toutes les fonctions d'affichage et d'interface graphique
"""
import pygame
import chess
import time
from settings import *
from support import *

# Images des pièces (chargées une seule fois)
images = load_images()

def draw_navbar(screen, font):
    """Dessine la navbar en haut de l'écran."""
    screen_width = screen.get_width()
    
    # Fond de la navbar
    navbar_rect = pygame.Rect(0, 0, screen_width, NAVBAR_HEIGHT)
    pygame.draw.rect(screen, UI_PANEL_BG, navbar_rect)
    pygame.draw.line(screen, UI_PANEL_BORDER, (0, NAVBAR_HEIGHT), (screen_width, NAVBAR_HEIGHT), 2)
    
    # Titre "Pygame Chess"
    title_font = pygame.font.Font(None, 48)
    title_surface = title_font.render("Pygame Chess", True, UI_ACCENT)
    title_rect = title_surface.get_rect(midleft=(20, NAVBAR_HEIGHT // 2))
    screen.blit(title_surface, title_rect)
    
    # Bouton "New Game"
    new_game_btn = pygame.Rect(screen_width - 340, 10, 150, 40)
    pygame.draw.rect(screen, UI_BUTTON_BG, new_game_btn, border_radius=8)
    pygame.draw.rect(screen, UI_BUTTON_BORDER, new_game_btn, width=2, border_radius=8)
    new_game_text = font.render("New Game", True, UI_TEXT_PRIMARY)
    new_game_text_rect = new_game_text.get_rect(center=new_game_btn.center)
    screen.blit(new_game_text, new_game_text_rect)
    
    # Bouton "Resign"
    resign_btn = pygame.Rect(screen_width - 180, 10, 150, 40)
    pygame.draw.rect(screen, UI_BUTTON_BG, resign_btn, border_radius=8)
    pygame.draw.rect(screen, UI_DANGER, resign_btn, width=2, border_radius=8)
    resign_text = font.render("Resign", True, UI_TEXT_PRIMARY)
    resign_text_rect = resign_text.get_rect(center=resign_btn.center)
    screen.blit(resign_text, resign_text_rect)
    
    return new_game_btn, resign_btn

def check_navbar_clicks(mouse_pos, new_game_btn, resign_btn):
    """Vérifie si un bouton de la navbar a été cliqué."""
    if new_game_btn.collidepoint(mouse_pos):
        return "new_game"
    elif resign_btn.collidepoint(mouse_pos):
        return "resign"
    return None

def draw_board_centered(screen, board_size, board_offset_x, board_offset_y):
    """Dessine l'échiquier centré sur l'écran."""
    tile_size = board_size // 8
    
    for row in range(8):
        for col in range(8):
            # Utiliser les couleurs du thème actuel
            colors = BOARD_COLORS[current_board_theme]
            color = colors[0] if (row + col) % 2 == 0 else colors[1]
            pygame.draw.rect(screen, color, 
                           (board_offset_x + col * tile_size, 
                            board_offset_y + row * tile_size, 
                            tile_size, tile_size))
    
    return tile_size

def draw_pieces_centered(screen, board, images, selected_piece, player_is_white, tile_size, board_offset_x, board_offset_y):
    """Dessine les pièces sur l'échiquier centré."""
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            piece_color = 'w' if piece.color == chess.WHITE else 'b'
            piece_type = piece.symbol().lower()
            piece_image = pygame.transform.scale(images[piece_color + piece_type], (tile_size, tile_size))

            # Ne pas dessiner la pièce en cours de drag sur l'échiquier
            if selected_piece is not None and square == selected_piece:
                continue

            col, row = chess.square_file(square), chess.square_rank(square)
            if player_is_white:
                x = board_offset_x + col * tile_size
                y = board_offset_y + (7 - row) * tile_size
            else:
                x = board_offset_x + (7 - col) * tile_size
                y = board_offset_y + row * tile_size
            
            screen.blit(piece_image, (x, y))

def highlight_selected_piece_centered(screen, selected_piece, player_is_white, tile_size, board_offset_x, board_offset_y):
    """Met en surbrillance la pièce sélectionnée."""
    if selected_piece is not None:
        col, row = chess.square_file(selected_piece), chess.square_rank(selected_piece)
        if player_is_white:
            x = board_offset_x + col * tile_size
            y = board_offset_y + (7 - row) * tile_size
        else:
            x = board_offset_x + (7 - col) * tile_size
            y = board_offset_y + row * tile_size
        
        pygame.draw.rect(screen, (0, 0, 255), (x, y, tile_size, tile_size), max(1, tile_size // 15))

def highlight_legal_moves_centered(screen, legal_moves, player_is_white, tile_size, board_offset_x, board_offset_y):
    """Met en surbrillance les coups légaux."""
    overlay = pygame.Surface((screen.get_width(), screen.get_height()), pygame.SRCALPHA)
    radius = max(5, tile_size // 6)
    
    for move in legal_moves:
        col, row = chess.square_file(move.to_square), chess.square_rank(move.to_square)
        if player_is_white:
            x = board_offset_x + col * tile_size
            y = board_offset_y + (7 - row) * tile_size
        else:
            x = board_offset_x + (7 - col) * tile_size
            y = board_offset_y + row * tile_size
        
        pygame.draw.circle(overlay, (0, 255, 0, 180),
                         (x + tile_size // 2, y + tile_size // 2), radius)
    screen.blit(overlay, (0, 0))

def highlight_last_move_centered(screen, last_move, player_is_white, tile_size, board_offset_x, board_offset_y):
    """Highlights the last move played on the board."""
    if last_move is None:
        return
    
    from_square = last_move.from_square
    to_square = last_move.to_square
    
    for square in [from_square, to_square]:
        col, row = chess.square_file(square), chess.square_rank(square)
        if player_is_white:
            x = board_offset_x + col * tile_size
            y = board_offset_y + (7 - row) * tile_size
        else:
            x = board_offset_x + (7 - col) * tile_size
            y = board_offset_y + row * tile_size
        
        highlight_surface = pygame.Surface((tile_size, tile_size))
        highlight_surface.set_alpha(100)
        highlight_surface.fill((255, 255, 0))
        screen.blit(highlight_surface, (x, y))

def draw_right_panel(screen, board, font, player_is_white=True):
    """Dessine le panneau droit avec toutes les informations."""
    import game_logic
    
    screen_width = screen.get_width()
    screen_height = screen.get_height()
    panel_x = screen_width - RIGHT_PANEL_WIDTH
    panel_y = NAVBAR_HEIGHT
    panel_height = screen_height - NAVBAR_HEIGHT
    
    # Fond du panneau
    panel_rect = pygame.Rect(panel_x, panel_y, RIGHT_PANEL_WIDTH, panel_height)
    pygame.draw.rect(screen, UI_PANEL_BG, panel_rect)
    pygame.draw.line(screen, UI_PANEL_BORDER, (panel_x, panel_y), (panel_x, screen_height), 2)
    
    # Fonts
    title_font = pygame.font.Font(None, 36)
    text_font = pygame.font.Font(None, 28)
    small_font = pygame.font.Font(None, 24)
    
    current_y = panel_y + 20
    
    # Section "Game Info"
    game_info_title = title_font.render("Game Info", True, UI_ACCENT)
    screen.blit(game_info_title, (panel_x + 20, current_y))
    current_y += 35  # Reduced from 50
    
    # White to Move / Black to Move
    turn_text = "White to Move" if board.turn == chess.WHITE else "Black to Move"
    turn_color = UI_TEXT_PRIMARY if board.turn == chess.WHITE else UI_TEXT_SECONDARY
    turn_surface = text_font.render(turn_text, True, turn_color)
    screen.blit(turn_surface, (panel_x + 20, current_y))
    current_y += 30  # Reduced from 40
    
    # Timers
    white_time_text = f"White: {int(game_logic.white_timer // 60)}:{int(game_logic.white_timer % 60):02d}"
    black_time_text = f"Black: {int(game_logic.black_timer // 60)}:{int(game_logic.black_timer % 60):02d}"
    
    white_time_surface = text_font.render(white_time_text, True, UI_TEXT_PRIMARY)
    black_time_surface = text_font.render(black_time_text, True, UI_TEXT_PRIMARY)
    
    screen.blit(white_time_surface, (panel_x + 20, current_y))
    current_y += 25  # Reduced from 30
    screen.blit(black_time_surface, (panel_x + 20, current_y))
    current_y += 35  # Reduced from 50
    
    # Séparateur
    pygame.draw.line(screen, UI_PANEL_BORDER, (panel_x + 20, current_y), (panel_x + RIGHT_PANEL_WIDTH - 20, current_y), 1)
    current_y += 15  # Reduced from 20
    
    # Section "Move History"
    history_title = title_font.render("Move History", True, UI_ACCENT)
    screen.blit(history_title, (panel_x + 20, current_y))
    current_y += 30  # Reduced from 40
    
    # Afficher l'historique des coups
    move_history = get_move_history(board)
    history_display_height = 150  # Reduced from 200
    history_y_start = current_y
    
    for i, move_line in enumerate(move_history[-6:]):  # Reduced from 8 to 6 moves
        move_surface = small_font.render(move_line, True, UI_TEXT_SECONDARY)
        screen.blit(move_surface, (panel_x + 20, current_y))
        current_y += 22  # Reduced from 25
    
    current_y = history_y_start + history_display_height + 15  # Reduced from 20
    
    # Séparateur
    pygame.draw.line(screen, UI_PANEL_BORDER, (panel_x + 20, current_y), (panel_x + RIGHT_PANEL_WIDTH - 20, current_y), 1)
    current_y += 15  # Reduced from 20
    
    # Section "Captured Pieces"
    captured_title = title_font.render("Captured Pieces", True, UI_ACCENT)
    screen.blit(captured_title, (panel_x + 20, current_y))
    current_y += 30  # Reduced from 40
    
    # Captured by White
    white_cap_text = small_font.render("Captured by White:", True, UI_TEXT_PRIMARY)
    screen.blit(white_cap_text, (panel_x + 20, current_y))
    current_y += 25  # Reduced from 30
    
    # Afficher les pièces capturées par les blancs
    piece_size = 22  # Reduced from 25
    spacing = 4  # Reduced from 5
    pieces_per_row = 10  # Increased from 8
    
    for i, piece in enumerate(game_logic.captured_pieces['black']):
        row = i // pieces_per_row
        col = i % pieces_per_row
        x_pos = panel_x + 20 + col * (piece_size + spacing)
        y_pos = current_y + row * (piece_size + spacing)
        piece_color = 'b'
        piece_type = piece.symbol().lower()
        mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
        screen.blit(mini_piece, (x_pos, y_pos))
    
    current_y += 40  # Reduced from 60
    
    # Captured by Black
    black_cap_text = small_font.render("Captured by Black:", True, UI_TEXT_PRIMARY)
    screen.blit(black_cap_text, (panel_x + 20, current_y))
    current_y += 25  # Reduced from 30
    
    for i, piece in enumerate(game_logic.captured_pieces['white']):
        row = i // pieces_per_row
        col = i % pieces_per_row
        x_pos = panel_x + 20 + col * (piece_size + spacing)
        y_pos = current_y + row * (piece_size + spacing)
        piece_color = 'w'
        piece_type = piece.symbol().lower()
        mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
        screen.blit(mini_piece, (x_pos, y_pos))
    
    current_y += 50  # Reduced from 80
    
    # Séparateur
    pygame.draw.line(screen, UI_PANEL_BORDER, (panel_x + 20, current_y), (panel_x + RIGHT_PANEL_WIDTH - 20, current_y), 1)
    current_y += 20
    
    # Section "Engine Info"
    engine_title = title_font.render("Engine Info", True, UI_ACCENT)
    screen.blit(engine_title, (panel_x + 20, current_y))
    current_y += 40
    
    # Afficher les infos du moteur
    if game_logic.engine_thinking or game_logic.engine2_thinking:
        thinking_text = small_font.render("Engine is thinking...", True, UI_ACCENT)
        screen.blit(thinking_text, (panel_x + 20, current_y))
        current_y += 30
        
        # Afficher les infos du moteur
        if hasattr(game_logic, 'engine_info') and game_logic.engine_info:
            depth_text = small_font.render(f"Depth: {game_logic.engine_info.get('depth', 'N/A')}", True, UI_TEXT_SECONDARY)
            screen.blit(depth_text, (panel_x + 20, current_y))
            current_y += 25
            
            nodes_text = small_font.render(f"Nodes: {game_logic.engine_info.get('nodes', 'N/A')}", True, UI_TEXT_SECONDARY)
            screen.blit(nodes_text, (panel_x + 20, current_y))
            current_y += 25
            
            score = game_logic.engine_info.get('score', 'N/A')
            score_text = small_font.render(f"Score: {score}", True, UI_TEXT_SECONDARY)
            screen.blit(score_text, (panel_x + 20, current_y))
    else:
        idle_text = small_font.render("Engine idle", True, UI_TEXT_SECONDARY)
        screen.blit(idle_text, (panel_x + 20, current_y))

def get_move_history(board):
    """Retourne l'historique des coups au format lisible."""
    move_list = list(board.move_stack)
    history = []
    
    temp_board = chess.Board()
    for i, move in enumerate(move_list):
        if i % 2 == 0:
            move_number = (i // 2) + 1
            san_move = temp_board.san(move)
            history.append(f"{move_number}. {san_move}")
        else:
            san_move = temp_board.san(move)
            history[-1] += f" {san_move}"
        
        temp_board.push(move)
    
    return history

def get_square_from_mouse_centered(mouse_x, mouse_y, player_is_white, tile_size, board_offset_x, board_offset_y):
    """Convertit les coordonnées de souris en case d'échiquier pour l'échiquier centré."""
    # Vérifier que le clic est dans l'échiquier
    if not (board_offset_x <= mouse_x < board_offset_x + tile_size * 8 and 
            board_offset_y <= mouse_y < board_offset_y + tile_size * 8):
        return None
    
    col = (mouse_x - board_offset_x) // tile_size
    row = (mouse_y - board_offset_y) // tile_size
    
    # Limiter col et row entre 0 et 7
    col = max(0, min(col, 7))
    row = max(0, min(row, 7))
    
    if player_is_white:
        return chess.square(col, 7 - row)
    else:
        return chess.square(7 - col, row)

def render_game(screen, board, font, player_is_white=True):
    """Rend l'interface complète du jeu avec la nouvelle mise en page."""
    import game_logic
    
    # Fond de l'écran
    screen.fill(UI_BG_COLOR)
    
    # Calculer les dimensions de l'échiquier
    screen_width = screen.get_width()
    screen_height = screen.get_height()
    
    # Taille disponible pour l'échiquier (entre le bord gauche et le panneau droit)
    available_width = screen_width - RIGHT_PANEL_WIDTH - 100  # 100 = marges
    available_height = screen_height - NAVBAR_HEIGHT - 40  # 40 = marges
    board_size = min(available_width, available_height)
    
    # Centrer l'échiquier dans l'espace disponible
    board_offset_x = (available_width - board_size) // 2 + 50
    board_offset_y = NAVBAR_HEIGHT + (available_height - board_size) // 2 + 20
    
    # Dessiner la navbar
    new_game_btn, resign_btn = draw_navbar(screen, font)
    
    # Stocker les boutons pour le click handling
    game_logic.navbar_buttons = (new_game_btn, resign_btn)
    
    # Dessiner l'échiquier
    tile_size = draw_board_centered(screen, board_size, board_offset_x, board_offset_y)
    
    # Stocker les informations de l'échiquier pour le click handling
    game_logic.board_render_info = {
        'tile_size': tile_size,
        'board_offset_x': board_offset_x,
        'board_offset_y': board_offset_y
    }
    
    # Highlight last move
    highlight_last_move_centered(screen, game_logic.last_move, player_is_white, tile_size, board_offset_x, board_offset_y)
    
    # Highlight selected piece
    highlight_selected_piece_centered(screen, game_logic.selected_piece, player_is_white, tile_size, board_offset_x, board_offset_y)
    
    # Dessiner les pièces
    draw_pieces_centered(screen, board, images, game_logic.selected_piece if game_logic.drag_mode else None, 
                        player_is_white, tile_size, board_offset_x, board_offset_y)
    
    # Highlight legal moves
    highlight_legal_moves_centered(screen, game_logic.legal_moves, player_is_white, tile_size, board_offset_x, board_offset_y)
    
    # Afficher la pièce sélectionnée si en mode drag
    if game_logic.drag_mode and game_logic.selected_piece is not None and game_logic.dragging:
        piece = board.piece_at(game_logic.selected_piece)
        if piece:
            piece_color = 'w' if piece.color == chess.WHITE else 'b'
            piece_type = piece.symbol().lower()
            dragged_piece = pygame.transform.scale(images[piece_color + piece_type], (tile_size, tile_size))
            screen.blit(dragged_piece, game_logic.dragged_pos)
    
    # Dessiner le panneau droit
    draw_right_panel(screen, board, font, player_is_white)

def display_draw(screen, reason, white_player="Player 1", black_player="Player 2"):
    """Affiche un message de partie nulle."""
    import game_logic
    
    font = pygame.font.Font(None, 64)
    screen_width, screen_height = screen.get_width(), screen.get_height()
    overlay = pygame.Surface((screen_width, screen_height))
    overlay.set_alpha(200)
    overlay.fill((0, 0, 0))

    message = "Partie nulle !"
    reason_text = f"({reason})"
    
    # Sauvegarder la partie comme nulle
    game_logic.end_game_and_save("1/2-1/2", white_player, black_player)

    text_surface = font.render(message, True, (255, 165, 0))
    reason_surface = pygame.font.Font(None, 36).render(reason_text, True, (200, 200, 200))
    text_rect = text_surface.get_rect(center=(screen_width // 2, screen_height // 2 - 30))
    reason_rect = reason_surface.get_rect(center=(screen_width // 2, screen_height // 2 + 20))

    screen.blit(overlay, (0, 0))
    screen.blit(text_surface, text_rect)
    screen.blit(reason_surface, reason_rect)
    pygame.display.flip()

    pygame.time.wait(4000)
    from menu import main_menu
    main_menu()

def display_checkmate(screen, loser_color, white_player="Human", black_player="Engine"):
    """Affiche le message d'échec et mat."""
    import game_logic
    
    font = pygame.font.Font(None, 64)
    screen_width, screen_height = screen.get_width(), screen.get_height()
    overlay = pygame.Surface((screen_width, screen_height))
    overlay.set_alpha(200)
    overlay.fill((0, 0, 0))

    winner = "Les Blancs" if loser_color == chess.BLACK else "Les Noirs"
    message = f"Échec et mat ! {winner} gagnent."
    
    # Determine result for PGN
    result = "1-0" if loser_color == chess.BLACK else "0-1"
    
    # Sauvegarder la partie
    game_logic.end_game_and_save(result, white_player, black_player)

    text_surface = font.render(message, True, (255, 0, 0))
    text_rect = text_surface.get_rect(center=(screen_width // 2, screen_height // 2))

    screen.blit(overlay, (0, 0))
    screen.blit(text_surface, text_rect)
    pygame.display.flip()

    pygame.time.wait(4000)
    from menu import main_menu
    main_menu()