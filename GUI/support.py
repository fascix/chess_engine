import pygame
import chess
from settings import *

# Permet de charger les images des différentes pièces blanches et noires
def load_images():
    pieces = ['k', 'q', 'b', 'n', 'p', 'r']
    images = {}
    # Taille de base pour le chargement, sera redimensionné à l'affichage
    base_size = 100 
    for color in ['w', 'b']:
        for piece in pieces:
            piece_id = color + piece
            try:
                images[piece_id] = pygame.transform.scale(
                    pygame.image.load(f"./assets/pieces/{color}/{piece}.png"), (base_size, base_size)
                )
            except Exception as e:
                print(f"Erreur chargement image {piece_id}: {e}")
                # Fallback: carré coloré
                s = pygame.Surface((base_size, base_size))
                s.fill((255, 255, 255) if color == 'w' else (0, 0, 0))
                images[piece_id] = s
    return images

# Permet de faire la promotion d'un pion
def promote_pawn(screen, board, move, images):
    """Affiche un menu graphique pour choisir une promotion et applique la pièce choisie."""
    options = [chess.QUEEN, chess.ROOK, chess.BISHOP, chess.KNIGHT]
    option_names = ["q", "r", "b", "n"]
    color = 'w' if board.turn == chess.WHITE else 'b'

    # Essayer de récupérer tile_size depuis game_logic si possible, sinon calculer
    import game_logic
    if game_logic.board_render_info:
        tile_size = game_logic.board_render_info['tile_size']
    else:
        screen_width, screen_height = screen.get_size()
        tile_size = min(screen_width, screen_height) // 8
        
    size = int(tile_size * 0.9)  # Taille des images des pièces promotion
    padding = max(5, size // 10)  # Espacement entre les images
    menu_width = len(options) * (size + padding) + padding
    menu_height = size + 2 * padding
    menu_x = (screen.get_width() - menu_width) // 2
    menu_y = (screen.get_height() - menu_height) // 2
    menu_rect = pygame.Rect(menu_x, menu_y, menu_width, menu_height)

    selecting = True
    while selecting:
        # Dessiner le fond du menu
        pygame.draw.rect(screen, (50, 50, 50), menu_rect, border_radius=10)

        piece_positions = []
        for i, piece in enumerate(options):
            img_x = menu_x + padding + i * (size + padding)
            img_y = menu_y + padding
            piece_positions.append((img_x, img_y, size, size))
            screen.blit(pygame.transform.scale(images[color + option_names[i]], (size, size)), (img_x, img_y))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                exit()
            elif event.type == pygame.MOUSEBUTTONDOWN:
                mouse_x, mouse_y = event.pos
                for i, (img_x, img_y, img_w, img_h) in enumerate(piece_positions):
                    if img_x <= mouse_x <= img_x + img_w and img_y <= mouse_y <= img_y + img_h:
                        move.promotion = options[i]
                        board.push(move)
                        selecting = False