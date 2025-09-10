import pygame
import chess
import time
from settings import *
from support import *

# Initialiser Pygame
pygame.init()

# Configuration de la fenêtre
screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
pygame.display.set_caption("Chess Engine")

# Charger les images des pièces
images = load_images()

# Créer un nouvel échiquier
board = chess.Board()

# Variables pour la gestion des clics et du drag
selected_piece = None
legal_moves = []
dragged_pos = (0, 0)  # Position de la pièce pendant le drag
dragging = False  # Flag pour savoir si une pièce est en train d'être déplacée

# Mode actuel : True pour drag, False pour clic
drag_mode = True

# Ajout des variables globales pour le timer

current_turn_start_time = None  # Démarre le timer à None

captured_pieces = {'white': [], 'black': []}  # Liste des pièces capturées

def draw_timer(screen, white_timer, black_timer, font):
    white_time_text = font.render(f"{int(white_timer // 60)}:{int(white_timer % 60):02d}", True, (255, 255, 255))
    black_time_text = font.render(f"{int(black_timer // 60)}:{int(black_timer % 60):02d}", True, (255, 255, 255))

    # Efface la zone du timer avant de le redessiner pour éviter les superpositions
    pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE+60, WINDOW_SIZE-120, 200, 50))
    pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE + 60, 80, 200, 50))
    screen.blit(white_time_text, (WINDOW_SIZE+ 60, WINDOW_SIZE-120))  # Timer blanc en haut
    screen.blit(black_time_text, (WINDOW_SIZE+ 60, 80))  # Timer noir en bas

def main_menu():
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Menu Principal")

    font = pygame.font.Font(None, 48)
    menu_options = ["Jouer", "Réglages", "Quitter"]
    selected_index = 0

    running = True
    while running:
        # Charger l'image de fond
        background_image = pygame.image.load('background.jpg')  # Remplace par le chemin de ton image
        background_image = pygame.transform.scale(background_image, (
        WINDOW_SIZE, WINDOW_SIZE))  # Redimensionner l'image pour remplir la fenêtre

        # Afficher l'image de fond à chaque frame
        screen.blit(background_image, (0, 0))  # Dessine l'image en haut à gauche de l'écran

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
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Menu Principal")

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
                        solo_game(timer)
                    else:
                        chess_engine(timer)
                    return

def settings_menu():
    """Affiche le menu des réglages (choix entre clic et drag & drop)."""
    global drag_mode
    drag_mode = True

    font = pygame.font.Font(None, 48)
    options = ["Drag & Drop (actif)", "Mode Clic", "Retour"]
    selected_index = 0

    running = True
    while running:
        screen.fill((0, 0, 0))

        # Mise à jour des options en fonction du mode actif
        options = [
            f"Drag & Drop {'(actif)' if drag_mode else ''}",
            f"Mode Clic {'(actif)' if not drag_mode else ''}",
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
                        drag_mode = True
                    elif selected_index == 1:
                        drag_mode = False
                    elif selected_index == 2:
                        main_menu()

def handle_click(event, board):
    global selected_piece, legal_moves, dragged_pos, dragging, current_turn_start_time

    mouse_x, mouse_y = event.pos
    if 0 <= mouse_x < WINDOW_SIZE and 0 <= mouse_y < WINDOW_SIZE:
        col, row = mouse_x // TILE_SIZE, mouse_y // TILE_SIZE
        square = chess.square(col, 7 - row)

        if not selected_piece:
            piece = board.piece_at(square)
            if piece and piece.color == board.turn and current_turn_start_time is not None:  # Vérifie si c'est une pièce du joueur et que le timer est actif
                selected_piece = square
                legal_moves = [move for move in board.legal_moves if move.from_square == square]  # Utilisation des objets Move
                dragging = True
                dragged_pos = (mouse_x - TILE_SIZE // 2, mouse_y - TILE_SIZE // 2)
        else:
                if square in [move.to_square for move in legal_moves]:  # Vérification avec les destinations des coups légaux
                    move = chess.Move(selected_piece, square)
                    if move in board.legal_moves:
                        if board.is_capture(move):
                            if board.is_en_passant(move):
                                direction = 8 if board.turn == chess.WHITE else -8
                                captured_square = move.to_square + direction
                            else:
                                captured_square = move.to_square

                            captured_piece = board.piece_at(captured_square)
                            if captured_piece:
                                captured_color = 'white' if captured_piece.color == chess.WHITE else 'black'
                                captured_pieces[captured_color].append(captured_piece)
                        board.push(move)
                if board.is_checkmate():
                    display_checkmate(screen, board.turn)
                selected_piece = None  # Réinitialiser après le déplacement
                legal_moves = []  # Effacer les coups légaux
                dragging = False

def handle_drag(event, board):
    global dragged_pos, dragging

    if drag_mode and selected_piece and dragging:
        mouse_x, mouse_y = event.pos
        dragged_pos = (mouse_x - TILE_SIZE // 2, mouse_y - TILE_SIZE // 2)

def handle_drop(event, board):
    global selected_piece, legal_moves, dragging
    if drag_mode and selected_piece and dragging:
        mouse_x, mouse_y = event.pos
        col, row = mouse_x // TILE_SIZE, mouse_y // TILE_SIZE
        square = chess.square(col, 7 - row)  # Calculer la case cible

        if square in [move.to_square for move in legal_moves]:  # Vérifier si le mouvement est légal
            move = chess.Move(selected_piece, square)

            # Vérification de la promotion
            if board.piece_at(move.from_square) and board.piece_at(move.from_square).piece_type == chess.PAWN:
                if (chess.square_rank(move.to_square) == 7 and board.turn == chess.WHITE) or \
                        (chess.square_rank(move.to_square) == 0 and board.turn == chess.BLACK):
                    promote_pawn(screen, board, move, images)  # Promotion
            if move in board.legal_moves:
                captured_piece = None
                captured_color = None
                if board.is_capture(move):
                    if board.is_en_passant(move):
                        # La case du pion capturé en passant dépend du joueur
                        direction = -8 if board.turn == chess.WHITE else 8
                        captured_square = move.to_square + direction
                    else:
                        captured_square = move.to_square

                    captured_piece = board.piece_at(captured_square)
                    if captured_piece:
                        captured_color = 'white' if captured_piece.color == chess.WHITE else 'black'

                board.push(move)

                # Ajouter la pièce capturée si elle existe
                if captured_piece and captured_color:
                    captured_pieces[captured_color].append(captured_piece)
            if board.is_checkmate():
                display_checkmate(screen, board.turn)

        # Réinitialiser l'état après le "drop"
        selected_piece = None
        legal_moves = []
        dragging = False

def highlight_en_passant_moves(screen, legal_moves):
    for move in legal_moves:
        # Vérification manuelle de l'en passant
        if move.promotion == chess.PAWN and abs(move.from_square - move.to_square) == 16:  # Si le déplacement est un double saut de pion
            col, row = chess.square_file(move.to_square), chess.square_rank(move.to_square)
            pygame.draw.circle(screen, (255, 0, 0), (col * TILE_SIZE + TILE_SIZE // 2, (7 - row) * TILE_SIZE + TILE_SIZE // 2), 10)

def draw_captured_pieces(screen):
    # Pièces capturées par les blancs = pièces noires mangées
    x_offset = WINDOW_SIZE + 60
    y_offset = WINDOW_SIZE - 80
    for piece in captured_pieces['black']:
        piece_color = 'b'
        piece_type = piece.symbol().lower()
        mini_piece = pygame.transform.scale(images[piece_color + piece_type], (30, 30))
        screen.blit(mini_piece, (x_offset, y_offset))
        x_offset += 35

    # Pièces capturées par les noirs = pièces blanches mangées
    x_offset = WINDOW_SIZE + 60
    y_offset = 20
    for piece in captured_pieces['white']:
        piece_color = 'w'
        piece_type = piece.symbol().lower()
        mini_piece = pygame.transform.scale(images[piece_color + piece_type], (30, 30))
        screen.blit(mini_piece, (x_offset, y_offset))
        x_offset += 35

def display_checkmate(screen, loser_color):
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
    main_menu()

def solo_game(timer):
    global white_timer, black_timer, current_turn_start_time
    current_turn_start_time = time.time()  # Démarre le timer ici
    if timer == 600:
        white_timer = black_timer = 600;
    elif timer == 300:
        white_timer = black_timer = 300;

    screen = pygame.display.set_mode((WINDOW_SIZE + 300, WINDOW_SIZE))

    # Boucle principale
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.MOUSEBUTTONDOWN:
                handle_click(event, board)

            elif event.type == pygame.MOUSEMOTION:
                handle_drag(event, board)

            elif event.type == pygame.MOUSEBUTTONUP:
                handle_drop(event, board)

        # Mise à jour du timer
        if current_turn_start_time is not None:  # S'assurer que le timer est actif
            elapsed_time = time.time() - current_turn_start_time
            if board.turn == chess.WHITE:
                white_timer -= elapsed_time
            else:
                black_timer -= elapsed_time

            current_turn_start_time = time.time()

        # Vérification si un joueur a épuisé son temps
        if white_timer <= 0:
            print("Temps écoulé ! Les noirs gagnent !")
            running = False
        elif black_timer <= 0:
            print("Temps écoulé ! Les blancs gagnent !")
            running = False

        # Dessiner l'échiquier
        draw_board(screen)
        highlight_selected_piece(screen, selected_piece)
        draw_pieces(screen, board, images, selected_piece if drag_mode else None)
        highlight_legal_moves(screen, legal_moves)

        # Afficher le timer

        draw_timer(screen, white_timer, black_timer, pygame.font.Font(None, 48))

        # Afficher les coups en passant
        highlight_en_passant_moves(screen, legal_moves)

        # Afficher les pièces capturées
        draw_captured_pieces(screen)

        # Afficher la pièce sélectionnée uniquement si on est en mode "drag"
        if drag_mode and selected_piece:
            piece = board.piece_at(selected_piece)
            if piece:
                piece_color = 'w' if piece.color == chess.WHITE else 'b'
                piece_type = piece.symbol().lower()
                dragged_piece = images[piece_color + piece_type]
                screen.blit(dragged_piece, dragged_pos)

        pygame.display.flip()

    pygame.quit()

main_menu()