"""
Module contenant toute la logique de jeu d'échecs
"""
import pygame
import chess
import chess.engine
import time
import threading
import queue
import config
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
engine_queue2 = queue.Queue()
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

# Variables pour les infos du moteur
engine_info = {'depth': 0, 'nodes': 0, 'score': 0, 'pv': ''}

# Variables pour le render (stockage des infos de positionnement)
board_render_info = {}
navbar_buttons = (None, None)
undo_button = None

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
    global engine_info
    engine = None
    
    def info_handler(info):
        """Handler pour récupérer les infos du moteur pendant la recherche."""
        global engine_info
        try:
            engine_info = {
                'depth': info.get('depth', engine_info.get('depth', 0)),
                'nodes': info.get('nodes', engine_info.get('nodes', 0)),
                'score': str(info.get('score', engine_info.get('score', 'N/A'))),
                'pv': ' '.join([str(move) for move in info.get('pv', [])[:3]]) if 'pv' in info else engine_info.get('pv', '')
            }
        except Exception:
            pass
    
    try:
        if engine_path is None:
            engine_path = config.get_bot1_path()
        engine = chess.engine.SimpleEngine.popen_uci(engine_path)
        
        # Utiliser analyse + stop pour avoir les infos en temps réel
        with engine.analysis(board_copy, chess.engine.Limit(time=1.0), info=chess.engine.INFO_ALL) as analysis:
            for info in analysis:
                info_handler(info)
            
            # Récupérer le meilleur coup
            best_move = analysis.info.get('pv', [None])[0] if 'pv' in analysis.info else None
            
        result_queue.put(best_move)
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

def handle_ui_click(event):
    """Gestion des clics de souris pour l'interface utilisateur (Navbar, etc.)."""
    from gui import check_navbar_clicks
    from menu import main_menu

    mouse_x, mouse_y = event.pos
    
    # Vérifier la navbar
    if navbar_buttons[0] and navbar_buttons[1] and mouse_y < NAVBAR_HEIGHT:
        navbar_action = check_navbar_clicks(event.pos, *navbar_buttons)
        if navbar_action == "new_game":
            main_menu()
            return "menu"
        elif navbar_action == "resign":
            main_menu()
            return "menu"
            
    return None

def handle_click(event, board):
    """Gère les clics de souris pour sélectionner et déplacer les pièces."""
    global selected_piece, legal_moves, dragged_pos, dragging, current_turn_start_time
    from gui import get_square_from_mouse_centered

    mouse_x, mouse_y = event.pos
    
    # Récupérer les infos de rendu de l'échiquier
    if not board_render_info:
        return
    
    tile_size = board_render_info['tile_size']
    board_offset_x = board_render_info['board_offset_x']
    board_offset_y = board_render_info['board_offset_y']
    
    square = get_square_from_mouse_centered(mouse_x, mouse_y, player_is_white, tile_size, board_offset_x, board_offset_y)
    
    # Si le clic est hors de l'échiquier
    if square is None:
        # Si on clique ailleurs et qu'on n'est pas en drag mode, on désélectionne
        if not drag_mode:
            selected_piece = None
            legal_moves = []
        return

    if selected_piece is None:
        piece = board.piece_at(square)
        if piece and piece.color == board.turn and current_turn_start_time is not None:
            selected_piece = square
            legal_moves = [move for move in board.legal_moves if move.from_square == square]
            # En mode drag, on active le dragging. En mode click, non.
            dragging = drag_mode
            dragged_pos = (mouse_x - tile_size // 2, mouse_y - tile_size // 2)
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

    if drag_mode and selected_piece is not None and dragging and board_render_info:
        tile_size = board_render_info['tile_size']
        screen_width, screen_height = pygame.display.get_surface().get_size()
        
        mouse_x, mouse_y = event.pos
        # Calculer la position de la pièce draggée (centrée sur la souris)
        drag_x = mouse_x - tile_size // 2
        drag_y = mouse_y - tile_size // 2
        
        # Limiter la position pour que la pièce reste visible à l'écran
        drag_x = max(-tile_size // 2, min(drag_x, screen_width - tile_size // 2))
        drag_y = max(-tile_size // 2, min(drag_y, screen_height - tile_size // 2))
        
        dragged_pos = (drag_x, drag_y)

def handle_drop(event, board):
    """Gère le drop des pièces."""
    global selected_piece, legal_moves, dragging
    from gui import get_square_from_mouse_centered
    
    # Si on n'est pas en mode drag OU si aucune pièce n'est sélectionnée OU si dragging est False, on ignore
    if not drag_mode:
        return
    
    if not dragging:
        return
        
    if selected_piece is None:
        # Réinitialiser dragging au cas où
        dragging = False
        return
    
    # À ce stade, on a drag_mode=True, dragging=True et selected_piece est défini
    mouse_x, mouse_y = event.pos
    
    # Récupérer les infos de rendu de l'échiquier
    if not board_render_info:
        selected_piece = None
        legal_moves = []
        dragging = False
        return
    
    tile_size = board_render_info['tile_size']
    board_offset_x = board_render_info['board_offset_x']
    board_offset_y = board_render_info['board_offset_y']
    
    square = get_square_from_mouse_centered(mouse_x, mouse_y, player_is_white, tile_size, board_offset_x, board_offset_y)
    
    # Si on drop en dehors de l'échiquier, on annule simplement la sélection
    if square is None:
        selected_piece = None
        legal_moves = []
        dragging = False
        return

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
    
    # Vérifier la règle des 50 coups
    if board.is_fifty_moves():
        from gui import display_draw
        display_draw(pygame.display.get_surface(), "Règle des 50 coups")
        return
    
    # Vérifier le pat (stalemate)
    if board.is_stalemate():
        from gui import display_draw
        display_draw(pygame.display.get_surface(), "Pat (Stalemate)")
        return
    
    # Vérifier matériel insuffisant
    if board.is_insufficient_material():
        from gui import display_draw
        display_draw(pygame.display.get_surface(), "Matériel insuffisant")
        return
        
    

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