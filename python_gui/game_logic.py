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
engine_thinking = False
engine_move_ready = False
pending_engine_move = None
game_paused = False

# Variable pour l'orientation du plateau
player_is_white = True

def reset_game():
    """Remet à zéro toutes les variables de jeu."""
    global board, selected_piece, legal_moves, dragging, captured_pieces
    global engine_thinking, engine_move_ready, pending_engine_move
    
    board = chess.Board()
    selected_piece = None
    legal_moves = []
    dragging = False
    captured_pieces = {'white': [], 'black': []}
    engine_thinking = False
    engine_move_ready = False
    pending_engine_move = None

def engine_worker(board_copy, result_queue):
    """Worker thread pour calculer le coup du moteur sans bloquer l'interface."""
    engine = None
    try:
        print(f"🔍 DEBUG: Démarrage moteur avec position: {board_copy.fen()}")
        engine = chess.engine.SimpleEngine.popen_uci("../chess_uci")
        print(f"🔍 DEBUG: Moteur démarré, lancement recherche...")
        result = engine.play(board_copy, chess.engine.Limit(time=1.0))
        print(f"🔍 DEBUG: Coup reçu du moteur: {result.move}")
        result_queue.put(result.move)
    except Exception as e:
        print(f"❌ Erreur dans le moteur: {e}")
        import traceback
        traceback.print_exc()
        result_queue.put(None)
    finally:
        if engine is not None:
            try:
                engine.quit()
            except:
                pass
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

def handle_click(event, board):
    """Gère les clics de souris pour sélectionner et déplacer les pièces."""
    global selected_piece, legal_moves, dragged_pos, dragging, current_turn_start_time
    from support import get_square_from_pos

    mouse_x, mouse_y = event.pos
    if 0 <= mouse_x < WINDOW_SIZE and 0 <= mouse_y < WINDOW_SIZE:
        square = get_square_from_pos(mouse_x, mouse_y, player_is_white)

        if not selected_piece:
            piece = board.piece_at(square)
            if piece and piece.color == board.turn and current_turn_start_time is not None:
                selected_piece = square
                legal_moves = [move for move in board.legal_moves if move.from_square == square]
                dragging = True
                dragged_pos = (mouse_x - TILE_SIZE // 2, mouse_y - TILE_SIZE // 2)
        else:
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

    if drag_mode and selected_piece and dragging:
        mouse_x, mouse_y = event.pos
        dragged_pos = (mouse_x - TILE_SIZE // 2, mouse_y - TILE_SIZE // 2)

def handle_drop(event, board):
    """Gère le drop des pièces."""
    global selected_piece, legal_moves, dragging
    from support import get_square_from_pos
    
    if drag_mode and selected_piece and dragging:
        mouse_x, mouse_y = event.pos
        square = get_square_from_pos(mouse_x, mouse_y, player_is_white)

        if square in [move.to_square for move in legal_moves]:
            move = chess.Move(selected_piece, square)
            
            # Vérification de la promotion
            if board.piece_at(move.from_square) and board.piece_at(move.from_square).piece_type == chess.PAWN:
                if ((chess.square_rank(move.to_square) == 7 and board.turn == chess.WHITE) or
                        (chess.square_rank(move.to_square) == 0 and board.turn == chess.BLACK)):
                    from gui import promote_pawn, images
                    promote_pawn(pygame.display.get_surface(), board, move, images)
                    
            if move in board.legal_moves:
                execute_move(board, move)

        selected_piece = None
        legal_moves = []
        dragging = False

def execute_move(board, move):
    """Exécute un mouvement et gère les captures."""
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

    # Ajouter la pièce capturée si elle existe
    if captured_piece and captured_color:
        captured_pieces[captured_color].append(captured_piece)
        
    if board.is_checkmate():
        from gui import display_checkmate
        display_checkmate(pygame.display.get_surface(), board.turn)

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
