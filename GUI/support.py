import pygame
import chess
from settings import *

# Fonctions utilitaires :

def get_display_coords(square, player_is_white=True, tile_size=80):
    """Convertit les coordonnées d'échiquier en coordonnées d'affichage selon l'orientation."""
    col, row = chess.square_file(square), chess.square_rank(square)
    if player_is_white:
        return col * tile_size, (7 - row) * tile_size
    else:
        return (7 - col) * tile_size, row * tile_size

def get_square_from_pos(mouse_x, mouse_y, player_is_white=True, tile_size=80):
    """Convertit les coordonnées de souris en case d'échiquier selon l'orientation."""
    # Calculer col et row avec division entière
    col, row = mouse_x // tile_size, mouse_y // tile_size
    
    # Limiter col et row entre 0 et 7 pour éviter les débordements
    col = max(0, min(col, 7))
    row = max(0, min(row, 7))
    
    if player_is_white:
        return chess.square(col, 7 - row)
    else:
        return chess.square(7 - col, row)

def get_tile_size(screen):
    screen_width, screen_height = screen.get_size()
    # Utiliser une logique similaire à gui.py ou une valeur par défaut
    return min(screen_width, screen_height) // 8

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

# Permet de dessiner le plateau d'échecs (Legacy / Fallback)
def draw_board(screen):
    """Dessine l'échiquier sans les pièces."""
    tile_size = get_tile_size(screen)
    for row in range(8):
        for col in range(8):
            color = (238, 238, 210) if (row + col) % 2 == 0 else (118, 150, 86)
            pygame.draw.rect(screen, color, (col * tile_size, row * tile_size, tile_size, tile_size))

# Permet de dessiner les pièces (Legacy / Fallback)
def draw_pieces(screen, board, images, selected_piece=None, player_is_white=True):
    tile_size = get_tile_size(screen)
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            piece_color = 'w' if piece.color == chess.WHITE else 'b'
            piece_type = piece.symbol().lower()
            piece_image = pygame.transform.scale(images[piece_color + piece_type], (tile_size, tile_size))

            # Ne pas dessiner la pièce en cours de drag sur l'échiquier
            if selected_piece is not None and square == selected_piece:
                continue

            x, y = get_display_coords(square, player_is_white, tile_size)
            screen.blit(piece_image, (x, y))

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
        tile_size = get_tile_size(screen)
        
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

# Permet d'afficher l'échecs et mat
def display_checkmate(screen, winner):
    """ Affiche un message d'échec et mat """

    font = pygame.font.Font(None, 72)
    message = "Échec et mat ! Blanc gagne" if winner == chess.BLACK else "Échec et mat ! Noir gagne"
    text_surface = font.render(message, True, (255, 0, 0))

    text_rect = text_surface.get_rect(center=(screen.get_width() // 2, screen.get_height() // 2))
    screen.blit(text_surface, text_rect)
    pygame.display.flip()

    pygame.time.delay(3000)
    from menu import main_menu
    main_menu()
