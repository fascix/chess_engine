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

def draw_timer(screen, white_timer, black_timer, font, player_is_white=True):
    """Affiche les timers des deux joueurs selon l'orientation."""
    white_time_text = font.render(f"{int(white_timer // 60)}:{int(white_timer % 60):02d}", True, (255, 255, 255))
    black_time_text = font.render(f"{int(black_timer // 60)}:{int(black_timer % 60):02d}", True, (255, 255, 255))

    if player_is_white:
        # Blancs en bas, noirs en haut
        pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE+60, WINDOW_SIZE-150, 200, 50))  # Timer blanc
        pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE + 60, 50, 200, 50))  # Timer noir
        screen.blit(white_time_text, (WINDOW_SIZE+ 60, WINDOW_SIZE-150))  # Timer blanc en bas
        screen.blit(black_time_text, (WINDOW_SIZE+ 60, 50))  # Timer noir en haut
    else:
        # Noirs en bas, blancs en haut (timers inversés)
        pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE+60, WINDOW_SIZE-150, 200, 50))  # Timer noir
        pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE + 60, 50, 200, 50))  # Timer blanc
        screen.blit(black_time_text, (WINDOW_SIZE+ 60, WINDOW_SIZE-150))  # Timer noir en bas
        screen.blit(white_time_text, (WINDOW_SIZE+ 60, 50))  # Timer blanc en haut

def draw_engine_status(screen, font):
    """Affiche le statut du moteur quand il réfléchit."""
    # Import local pour éviter les imports circulaires
    import game_logic
    
    # Effacer d'abord la zone du statut (plus grande pour couvrir les 2 lignes)
    pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE + 60, WINDOW_SIZE//2 - 30, 200, 80))
    
    if game_logic.engine_thinking:
        status_text = font.render("L'ordinateur", True, (255, 255, 0))
        status_text2 = font.render("réfléchit...", True, (255, 255, 0))
        screen.blit(status_text, (WINDOW_SIZE + 60, WINDOW_SIZE//2 - 15))
        screen.blit(status_text2, (WINDOW_SIZE + 60, WINDOW_SIZE//2 + 15))
    elif game_logic.game_paused:
        status_text = font.render("PAUSE", True, (255, 0, 0))
        screen.blit(status_text, (WINDOW_SIZE + 60, WINDOW_SIZE//2))

def draw_captured_pieces(screen, player_is_white=True):
    """Affiche les pièces capturées avec un système de grille selon l'orientation."""
    # Import local pour éviter les imports circulaires
    import game_logic
    
    piece_size = 25  # Taille réduite des pièces
    pieces_per_row = 6  # Nombre de pièces par ligne
    spacing = 5  # Espacement entre les pièces
    start_x = WINDOW_SIZE + 60
    
    if player_is_white:
        # Pièces capturées par les blancs (noires) = en bas
        start_y = WINDOW_SIZE - 100
        for i, piece in enumerate(game_logic.captured_pieces['black']):
            row = i // pieces_per_row
            col = i % pieces_per_row
            x_pos = start_x + col * (piece_size + spacing)
            y_pos = start_y + row * (piece_size + spacing)
            piece_color = 'b'
            piece_type = piece.symbol().lower()
            mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
            screen.blit(mini_piece, (x_pos, y_pos))

        # Pièces capturées par les noirs (blanches) = en haut
        start_y = 110
        for i, piece in enumerate(game_logic.captured_pieces['white']):
            row = i // pieces_per_row
            col = i % pieces_per_row
            x_pos = start_x + col * (piece_size + spacing)
            y_pos = start_y + row * (piece_size + spacing)
            piece_color = 'w'
            piece_type = piece.symbol().lower()
            mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
            screen.blit(mini_piece, (x_pos, y_pos))
    else:
        # Inversion pour quand le joueur joue les noirs
        # Pièces capturées par les noirs (blanches) = en bas
        start_y = WINDOW_SIZE - 100
        for i, piece in enumerate(game_logic.captured_pieces['white']):
            row = i // pieces_per_row
            col = i % pieces_per_row
            x_pos = start_x + col * (piece_size + spacing)
            y_pos = start_y + row * (piece_size + spacing)
            piece_color = 'w'
            piece_type = piece.symbol().lower()
            mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
            screen.blit(mini_piece, (x_pos, y_pos))

        # Pièces capturées par les blancs (noires) = en haut
        start_y = 110
        for i, piece in enumerate(game_logic.captured_pieces['black']):
            row = i // pieces_per_row
            col = i % pieces_per_row
            x_pos = start_x + col * (piece_size + spacing)
            y_pos = start_y + row * (piece_size + spacing)
            piece_color = 'b'
            piece_type = piece.symbol().lower()
            mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
            screen.blit(mini_piece, (x_pos, y_pos))

def highlight_en_passant_moves(screen, legal_moves, player_is_white=True):
    """Met en surbrillance les coups en passant."""
    for move in legal_moves:
        # Vérification manuelle de l'en passant
        if move.promotion == chess.PAWN and abs(move.from_square - move.to_square) == 16:
            x, y = get_display_coords(move.to_square, player_is_white)
            pygame.draw.circle(screen, (255, 0, 0), 
                             (x + TILE_SIZE // 2, y + TILE_SIZE // 2), 10)

def display_checkmate(screen, loser_color):
    """Affiche le message d'échec et mat."""
    font = pygame.font.Font(None, 64)
    overlay = pygame.Surface((WINDOW_SIZE + 300, WINDOW_SIZE))
    overlay.set_alpha(200)
    overlay.fill((0, 0, 0))

    winner = "Les Blancs" if loser_color == chess.BLACK else "Les Noirs"
    message = f"Échec et mat ! {winner} gagnent."

    text_surface = font.render(message, True, (255, 0, 0))
    text_rect = text_surface.get_rect(center=(WINDOW_SIZE // 2 + 150, WINDOW_SIZE // 2))

    screen.blit(overlay, (0, 0))
    screen.blit(text_surface, text_rect)
    pygame.display.flip()

    # Attendre quelques secondes avant de retourner au menu
    pygame.time.wait(4000)
    from menu import main_menu
    main_menu()

def render_game(screen, board, font, player_is_white=True):
    """Rend l'interface complète du jeu."""
    # Import local pour éviter les imports circulaires au niveau du module
    import game_logic
    
    # Dessiner l'échiquier
    draw_board(screen)
    highlight_selected_piece(screen, game_logic.selected_piece, player_is_white)
    draw_pieces(screen, board, images, game_logic.selected_piece if game_logic.drag_mode else None, player_is_white)
    highlight_legal_moves(screen, game_logic.legal_moves, player_is_white)
    
    # Afficher les timers et statuts
    draw_timer(screen, game_logic.white_timer, game_logic.black_timer, font, player_is_white)
    draw_engine_status(screen, pygame.font.Font(None, 36))
    
    # Afficher les éléments de jeu
    highlight_en_passant_moves(screen, game_logic.legal_moves, player_is_white)
    draw_captured_pieces(screen, player_is_white)
    
    # Afficher la pièce sélectionnée si en mode drag
    if game_logic.drag_mode and game_logic.selected_piece is not None:
        piece = board.piece_at(game_logic.selected_piece)
        if piece:
            piece_color = 'w' if piece.color == chess.WHITE else 'b'
            piece_type = piece.symbol().lower()
            dragged_piece = images[piece_color + piece_type]
            screen.blit(dragged_piece, game_logic.dragged_pos)