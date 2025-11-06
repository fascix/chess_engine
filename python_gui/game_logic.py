"""
Module contenant toute la logique de jeu d'échecs
"""
import pygame
import chess
import chess.engine
import time
import threading
import queue
from settings import *
import config

# Variables globales pour le jeu
board = chess.Board()
selected_piece = None
legal_moves = []
dragged_pos = (0, 0)
dragging = False
drag_mode = True

# Variables pour le timer
current_turn_start_time = None
white_timer = 0
black_timer = 0

# Variables pour les pièces capturées
captured_pieces = {'white': [], 'black': []}

# Variables pour le threading du moteur
engine_queue = queue.Queue()
engine_queue2 = queue.Queue()  # Queue for second engine in bot vs bot
engine_thinking = False
engine_move_ready = False
pending_engine_move = None
game_paused = False

# Variables pour bot vs bot
engine2_thinking = False
engine2_move_ready = False
pending_engine2_move = None

# Variable pour l'orientation du plateau
player_is_white = True

# Variable pour le dernier coup joué
last_move = None

def reset_game():
    """Remet à zéro toutes les variables de jeu."""
    global board, selected_piece, legal_moves, dragging, captured_pieces
    global engine_thinking, engine_move_ready, pending_engine_move
    global engine2_thinking, engine2_move_ready, pending_engine2_move, last_move
    
    board = chess.Board()
    selected_piece = None
    legal_moves = []
    dragging = False
    captured_pieces = {'white': [], 'black': []}
    engine_thinking = False
    engine_move_ready = False
    pending_engine_move = None
    engine2_thinking = False
    engine2_move_ready = False
    pending_engine2_move = None
    last_move = None

def engine_worker(board_copy, result_queue, engine_path=None):
    """Worker thread pour calculer le coup du moteur sans bloquer l'interface."""
    engine = None
    try:
        if engine_path is None:
            engine_path = config.get_bot1_path()
        engine = chess.engine.SimpleEngine.popen_uci(engine_path)
        result = engine.play(board_copy, chess.engine.Limit(time=1.0))
        result_queue.put(result.move)
    except Exception as e:
        print(f"Erreur dans le moteur: {e}")
        result_queue.put(None)
    finally:
        if engine is not None:
            try:
                engine.quit()
            except:
                pass

def start_engine_calculation(board, engine_path=None):
    """Démarre le calcul du moteur dans un thread séparé."""
    global engine_thinking, engine_move_ready, pending_engine_move
    engine_thinking = True
    engine_move_ready = False
    pending_engine_move = None
    
    board_copy = board.copy()
    thread = threading.Thread(target=engine_worker, args=(board_copy, engine_queue, engine_path))
    thread.daemon = True
    thread.start()

def start_engine2_calculation(board):
    """Démarre le calcul du second moteur (bot vs bot)."""
    global engine2_thinking, engine2_move_ready, pending_engine2_move
    engine2_thinking = True
    engine2_move_ready = False
    pending_engine2_move = None
    
    board_copy = board.copy()
    engine2_path = config.get_bot2_path()
    thread = threading.Thread(target=engine_worker, args=(board_copy, engine_queue2, engine2_path))
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

def check_engine2_result():
    """Vérifie si le second moteur a terminé son calcul."""
    global engine2_thinking, engine2_move_ready, pending_engine2_move
    
    try:
        move = engine_queue2.get_nowait()
        engine2_thinking = False
        engine2_move_ready = True
        pending_engine2_move = move
        return True
    except queue.Empty:
        return False

def handle_click(event, board):
    """Gère les clics de souris pour sélectionner et déplacer les pièces."""
    global selected_piece, legal_moves, dragged_pos, dragging, current_turn_start_time
    from support import get_square_from_pos

    mouse_x, mouse_y = event.pos
    
    # Vérifier que le clic est bien dans les limites de l'échiquier
    if not (0 <= mouse_x < WINDOW_SIZE and 0 <= mouse_y < WINDOW_SIZE):
        return
    
    square = get_square_from_pos(mouse_x, mouse_y, player_is_white)

    if selected_piece is None:
        piece = board.piece_at(square)
        if piece and piece.color == board.turn and current_turn_start_time is not None:
            selected_piece = square
            legal_moves = [move for move in board.legal_moves if move.from_square == square]
            # En mode drag, on active le dragging. En mode click, non.
            dragging = drag_mode
            dragged_pos = (mouse_x - TILE_SIZE // 2, mouse_y - TILE_SIZE // 2)
    else:
        # En mode drag, on ne gère PAS la désélection ici (c'est handle_drop qui s'en charge)
        # En mode click, on gère le deuxième clic pour déplacer la pièce
        if not drag_mode:
            if square in [move.to_square for move in legal_moves]:
                move = chess.Move(selected_piece, square)
                if move in board.legal_moves:
                    execute_move(board, move)
            selected_piece = None
            legal_moves = []
            dragging = False

def handle_drag(event, board):
    """Gère le drag des pièces."""
    global dragged_pos, dragging

    if drag_mode and selected_piece is not None and dragging:
        mouse_x, mouse_y = event.pos
        # Calculer la position de la pièce draggée (centrée sur la souris)
        drag_x = mouse_x - TILE_SIZE // 2
        drag_y = mouse_y - TILE_SIZE // 2
        
        # Limiter la position pour que la pièce reste visible à l'écran
        # (éviter qu'elle ne sorte complètement de l'échiquier)
        drag_x = max(-TILE_SIZE // 2, min(drag_x, WINDOW_SIZE - TILE_SIZE // 2))
        drag_y = max(-TILE_SIZE // 2, min(drag_y, WINDOW_SIZE - TILE_SIZE // 2))
        
        dragged_pos = (drag_x, drag_y)

def handle_drop(event, board):
    """Gère le drop des pièces."""
    global selected_piece, legal_moves, dragging
    from support import get_square_from_pos
    
    # Si on n'est pas en mode drag OU si aucune pièce n'est sélectionnée OU si dragging est False, on ignore
    if not drag_mode:
        return
    
    if not dragging:
        return
        
    if selected_piece is None:  # Utiliser is None au lieu de not
        # Réinitialiser dragging au cas où
        dragging = False
        return
    
    # À ce stade, on a drag_mode=True, dragging=True et selected_piece est défini
    mouse_x, mouse_y = event.pos
    
    # Vérifier que le drop est dans les limites de l'échiquier
    # Si on drop en dehors, on annule simplement la sélection
    if not (0 <= mouse_x < WINDOW_SIZE and 0 <= mouse_y < WINDOW_SIZE):
        selected_piece = None
        legal_moves = []
        dragging = False
        return
    
    square = get_square_from_pos(mouse_x, mouse_y, player_is_white)

    if square in [move.to_square for move in legal_moves]:
        move = chess.Move(selected_piece, square)
        
        # Vérification de la promotion
        if board.piece_at(move.from_square) and board.piece_at(move.from_square).piece_type == chess.PAWN:
            if ((chess.square_rank(move.to_square) == 7 and board.turn == chess.WHITE) or
                    (chess.square_rank(move.to_square) == 0 and board.turn == chess.BLACK)):
                # Import local pour éviter l'import circulaire
                from support import promote_pawn, load_images
                promote_pawn(pygame.display.get_surface(), board, move, load_images())
                
        if move in board.legal_moves:
            execute_move(board, move)

    selected_piece = None
    legal_moves = []
    dragging = False

def execute_move(board, move):
    """Exécute un mouvement et gère les captures."""
    global last_move
    captured_piece = None
    captured_color = None
    
    if board.is_capture(move):
        if board.is_en_passant(move):
            direction = -8 if board.turn == chess.WHITE else 8
            captured_square = move.to_square + direction
        else:
            captured_square = move.to_square

        captured_piece = board.piece_at(captured_square)
        if captured_piece:
            captured_color = 'white' if captured_piece.color == chess.WHITE else 'black'

    board.push(move)
    last_move = move  # Store the last move made

    # Ajouter la pièce capturée si elle existe
    if captured_piece and captured_color:
        captured_pieces[captured_color].append(captured_piece)
        
    # Don't display checkmate here - let the game loop handle it
    # so the board can be rendered first with the last move visible

def update_timers():
    """Met à jour les timers des joueurs."""
    global white_timer, black_timer, current_turn_start_time
    
    if current_turn_start_time is not None and not game_paused:
        elapsed_time = time.time() - current_turn_start_time
        if board.turn == chess.WHITE:
            white_timer -= elapsed_time
        else:
            black_timer -= elapsed_time
        current_turn_start_time = time.time()

def is_time_up():
    """Vérifie si le temps d'un joueur est écoulé."""
    return white_timer <= 0 or black_timer <= 0

def get_time_winner():
    """Retourne le gagnant par temps écoulé."""
    if white_timer <= 0:
        return "Les noirs gagnent !"
    elif black_timer <= 0:
        return "Les blancs gagnent !"
    return None