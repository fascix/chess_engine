import pygame
import chess
import time
import chess.engine
import threading
import queue
import random
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

# Variables pour le threading du moteur
engine_queue = queue.Queue()
engine_thinking = False
engine_move_ready = False
pending_engine_move = None
game_paused = False

def engine_worker(board_copy, result_queue):
    """Worker thread pour calculer le coup du moteur sans bloquer l'interface."""
    try:
        # Simulation avec stockfish ou autre moteur
        # engine = chess.engine.SimpleEngine.popen_uci("/opt/homebrew/bin/stockfish")
        # result = engine.play(board_copy, chess.engine.Limit(time=1.0))
        # result_queue.put(result.move)
        # engine.quit()
        
        # Pour l'instant, coup aléatoire pour éviter les erreurs
        legal_moves = list(board_copy.legal_moves)
        if legal_moves:
            move = random.choice(legal_moves)
            time.sleep(1)  # Simule le temps de calcul
            result_queue.put(move)
        else:
            result_queue.put(None)
    except Exception as e:
        print(f"Erreur dans le moteur: {e}")
        result_queue.put(None)

def start_engine_calculation(board):
    """Démarre le calcul du moteur dans un thread séparé."""
    global engine_thinking, engine_move_ready, pending_engine_move
    engine_thinking = True
    engine_move_ready = False
    pending_engine_move = None
    
    board_copy = board.copy()
    thread = threading.Thread(target=engine_worker, args=(board_copy, engine_queue))
    thread.daemon = True
    thread.start()

def check_engine_result():
    """Vérifie si le moteur a terminé son calcul."""
    global engine_thinking, engine_move_ready, pending_engine_move
    
    try:
        move = engine_queue.get_nowait()
        engine_thinking = False
        engine_move_ready = True
        pending_engine_move = move
        return True
    except queue.Empty:
        return False

def draw_timer(screen, white_timer, black_timer, font):
    white_time_text = font.render(f"{int(white_timer // 60)}:{int(white_timer % 60):02d}", True, (255, 255, 255))
    black_time_text = font.render(f"{int(black_timer // 60)}:{int(black_timer % 60):02d}", True, (255, 255, 255))

    # Efface la zone du timer avant de le redessiner pour éviter les superpositions
    pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE+60, WINDOW_SIZE-120, 200, 50))
    pygame.draw.rect(screen, (0, 0, 0), (WINDOW_SIZE + 60, 80, 200, 50))
    screen.blit(white_time_text, (WINDOW_SIZE+ 60, WINDOW_SIZE-120))  # Timer blanc en haut
    screen.blit(black_time_text, (WINDOW_SIZE+ 60, 80))  # Timer noir en bas

def draw_engine_status(screen, font):
    """Affiche le statut du moteur quand il réfléchit."""
    if engine_thinking:
        status_text = font.render("L'ordinateur réfléchit...", True, (255, 255, 0))
        screen.blit(status_text, (WINDOW_SIZE + 60, WINDOW_SIZE//2))
    elif game_paused:
        status_text = font.render("PAUSE", True, (255, 0, 0))
        screen.blit(status_text, (WINDOW_SIZE + 60, WINDOW_SIZE//2))

def pause_menu():
    """Affiche le menu de pause pendant le jeu."""
    global game_paused, current_turn_start_time
    game_paused = True
    pause_start_time = time.time()
    
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
                        game_paused = False
                        # Ajuster le timer pour exclure le temps de pause
                        pause_duration = time.time() - pause_start_time
                        if current_turn_start_time:
                            current_turn_start_time += pause_duration
                        return "resume"
                    elif selected_index == 1:  # Retour au menu principal
                        return "main_menu"
                    elif selected_index == 2:  # Quitter le jeu
                        pygame.quit()
                        exit()
                elif event.key == pygame.K_ESCAPE:
                    game_paused = False
                    pause_duration = time.time() - pause_start_time
                    if current_turn_start_time:
                        current_turn_start_time += pause_duration
                    return "resume"

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
                        color_choice_menu("solo", timer)
                    else:
                        color_choice_menu("bot", timer)
                    return
def color_choice_menu(mode, timer):
    """Menu pour choisir la couleur des pièces."""
    pygame.display.set_caption("Sélection des couleurs")
    font = pygame.font.Font(None, 48)
    
    if mode == "solo":
        options = ["Blancs commencent", "Noirs commencent", "Aléatoire"]
    else:
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
                    
                    if mode == "solo":
                        solo_game(timer, player_is_white)
                    else:
                        chess_engine(timer, player_is_white)
                    return
                elif event.key == pygame.K_ESCAPE:
                    timer_menu(mode)
                    return

def settings_menu():
    """Affiche le menu des réglages (choix entre clic et drag & drop)."""
    pygame.display.set_caption("Paramètres")
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
    piece_size = 25  # Taille réduite des pièces
    pieces_per_row = 6  # Nombre de pièces par ligne
    spacing = 5  # Espacement entre les pièces
    
    # Pièces capturées par les blancs = pièces noires mangées
    start_x = WINDOW_SIZE + 60
    start_y = WINDOW_SIZE - 100
    
    for i, piece in enumerate(captured_pieces['black']):
        row = i // pieces_per_row
        col = i % pieces_per_row
        
        x_pos = start_x + col * (piece_size + spacing)
        y_pos = start_y + row * (piece_size + spacing)
        
        piece_color = 'b'
        piece_type = piece.symbol().lower()
        mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
        screen.blit(mini_piece, (x_pos, y_pos))

    # Pièces capturées par les noirs = pièces blanches mangées
    start_y = 20
    
    for i, piece in enumerate(captured_pieces['white']):
        row = i // pieces_per_row
        col = i % pieces_per_row
        
        x_pos = start_x + col * (piece_size + spacing)
        y_pos = start_y + row * (piece_size + spacing)
        
        piece_color = 'w'
        piece_type = piece.symbol().lower()
        mini_piece = pygame.transform.scale(images[piece_color + piece_type], (piece_size, piece_size))
        screen.blit(mini_piece, (x_pos, y_pos))

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

def solo_game(timer, player_is_white=True):
    global white_timer, black_timer, current_turn_start_time, board
    pygame.display.set_caption("Chess Engine - Mode 2 joueurs")
    
    # Réinitialiser le plateau pour une nouvelle partie
    board = chess.Board()
    
    current_turn_start_time = time.time()  # Démarre le timer ici
    if timer == 600:
        white_timer = black_timer = 600
    elif timer == 300:
        white_timer = black_timer = 300

    screen = pygame.display.set_mode((WINDOW_SIZE + 300, WINDOW_SIZE))

    # Boucle principale
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    result = pause_menu()
                    if result == "main_menu":
                        main_menu()
                        return

            elif event.type == pygame.MOUSEBUTTONDOWN and not game_paused:
                handle_click(event, board)

            elif event.type == pygame.MOUSEMOTION and not game_paused:
                handle_drag(event, board)

            elif event.type == pygame.MOUSEBUTTONUP and not game_paused:
                handle_drop(event, board)

        # Mise à jour du timer (seulement si le jeu n'est pas en pause)
        if current_turn_start_time is not None and not game_paused:  # S'assurer que le timer est actif
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
        
        # Afficher le statut du jeu
        draw_engine_status(screen, pygame.font.Font(None, 36))

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

def chess_engine(timer, player_is_white=True):
    global white_timer, black_timer, current_turn_start_time, board, engine_thinking, engine_move_ready, pending_engine_move
    pygame.display.set_caption("Chess Engine - Contre l'ordinateur")
    
    # Réinitialiser le plateau pour une nouvelle partie
    board = chess.Board()
    
    current_turn_start_time = time.time()
    if timer == 600:
        white_timer = black_timer = 600
    elif timer == 300:
        white_timer = black_timer = 300

    screen = pygame.display.set_mode((WINDOW_SIZE + 300, WINDOW_SIZE))
    
    # Si le joueur joue les noirs et c'est le tour des blancs, démarrer le calcul du bot
    if not player_is_white and board.turn == chess.WHITE:
        start_engine_calculation(board)
    # Initialiser le moteur UCI (à coder encore !)
    # test avec stockfish : engine = chess.engine.SimpleEngine.popen_uci("/opt/homebrew/bin/stockfish")
    #engine = chess.engine.SimpleEngine.popen_uci("?")
    try:
        running = True
        while running:
            # Vérifier si le moteur a terminé son calcul
            if engine_thinking:
                check_engine_result()
            
            # Si le coup du moteur est prêt, l'exécuter
            if engine_move_ready and pending_engine_move and not game_paused:
                global engine_move_ready, pending_engine_move
                if pending_engine_move in board.legal_moves:
                    # Gérer les captures pour les pièces capturées
                    if board.is_capture(pending_engine_move):
                        captured_piece = board.piece_at(pending_engine_move.to_square)
                        if captured_piece:
                            captured_color = 'white' if captured_piece.color == chess.WHITE else 'black'
                            captured_pieces[captured_color].append(captured_piece)
                    
                    board.push(pending_engine_move)
                    if board.is_checkmate():
                        display_checkmate(screen, board.turn)
                        
                engine_move_ready = False
                pending_engine_move = None
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        result = pause_menu()
                        if result == "main_menu":
                            main_menu()
                            return

                elif event.type == pygame.MOUSEBUTTONDOWN and not game_paused and not engine_thinking:
                    # Le joueur peut seulement jouer si c'est son tour
                    if (player_is_white and board.turn == chess.WHITE) or (not player_is_white and board.turn == chess.BLACK):
                        handle_click(event, board)

                elif event.type == pygame.MOUSEMOTION and not game_paused and not engine_thinking:
                    handle_drag(event, board)

                elif event.type == pygame.MOUSEBUTTONUP and not game_paused and not engine_thinking:
                    if (player_is_white and board.turn == chess.WHITE) or (not player_is_white and board.turn == chess.BLACK):
                        handle_drop(event, board)
                        # Après le coup du joueur, démarrer le calcul du bot si c'est son tour
                        if (player_is_white and board.turn == chess.BLACK) or (not player_is_white and board.turn == chess.WHITE):
                            if not board.is_game_over():
                                start_engine_calculation(board)

            # Mise à jour du timer (seulement si le jeu n'est pas en pause)
            if current_turn_start_time is not None and not game_paused:
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

            draw_board(screen)
            highlight_selected_piece(screen, selected_piece)
            draw_pieces(screen, board, images, selected_piece if drag_mode else None)
            highlight_legal_moves(screen, legal_moves)
            draw_timer(screen, white_timer, black_timer, pygame.font.Font(None, 48))
            draw_engine_status(screen, pygame.font.Font(None, 36))
            highlight_en_passant_moves(screen, legal_moves)
            draw_captured_pieces(screen)
            if drag_mode and selected_piece:
                piece = board.piece_at(selected_piece)
                if piece:
                    piece_color = 'w' if piece.color == chess.WHITE else 'b'
                    piece_type = piece.symbol().lower()
                    dragged_piece = images[piece_color + piece_type]
                    screen.blit(dragged_piece, dragged_pos)
            pygame.display.flip()
    finally:
        # engine.quit() # Commenté car on utilise un moteur simulé
        pass

main_menu()
