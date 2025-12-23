"""
Module pour gérer la sauvegarde des parties au format PGN
"""
import chess. pgn
import os
from datetime import datetime
import io

# Dossier de sauvegarde des logs
LOGS_DIR = "./GUI/logs"
MAX_GAMES = 5  # Nombre de parties à conserver

def ensure_logs_directory():
    """Crée le dossier logs s'il n'existe pas."""
    if not os.path. exists(LOGS_DIR):
        os.makedirs(LOGS_DIR)

def get_log_files():
    """Retourne la liste des fichiers PGN triés par date (plus récent en premier)."""
    ensure_logs_directory()
    files = [f for f in os.listdir(LOGS_DIR) if f.endswith('.pgn')]
    files.sort(reverse=True)  # Tri par nom (format timestamp)
    return files

def cleanup_old_logs():
    """Supprime les anciennes parties si on dépasse MAX_GAMES."""
    files = get_log_files()
    
    # Supprimer les fichiers au-delà de MAX_GAMES
    for old_file in files[MAX_GAMES:]:
        try:
            os.remove(os.path.join(LOGS_DIR, old_file))
            print(f"Ancien log supprimé: {old_file}")
        except Exception as e:
            print(f"Erreur lors de la suppression de {old_file}: {e}")

def save_game_pgn(board, result, white_player="Human", black_player="Engine", time_control="10+0"):
    """
    Sauvegarde la partie au format PGN. 
    
    Args:
        board: L'objet chess.Board contenant l'historique des coups
        result: Résultat de la partie ("1-0", "0-1", "1/2-1/2", "*")
        white_player:  Nom du joueur blanc
        black_player: Nom du joueur noir
        time_control: Contrôle du temps utilisé
    """
    ensure_logs_directory()
    
    # Créer un objet Game PGN
    game = chess.pgn.Game()
    
    # Ajouter les métadonnées (headers PGN standard)
    game.headers["Event"] = "Chess Engine Game"
    game.headers["Site"] = "Local"
    game.headers["Date"] = datetime.now().strftime("%Y. %m.%d")
    game.headers["Round"] = "1"
    game.headers["White"] = white_player
    game.headers["Black"] = black_player
    game.headers["Result"] = result
    game.headers["TimeControl"] = time_control
    
    # Reconstruire les coups depuis le board
    node = game
    temp_board = chess.Board()
    
    for move in board.move_stack:
        node = node.add_variation(move)
        temp_board.push(move)
    
    # Nom du fichier avec timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"game_{timestamp}.pgn"
    filepath = os.path.join(LOGS_DIR, filename)
    
    # Sauvegarder le PGN
    try:
        with open(filepath, "w") as pgn_file:
            exporter = chess.pgn.FileExporter(pgn_file)
            game.accept(exporter)
        
        print(f"✅ Partie sauvegardée: {filepath}")
        
        # Nettoyer les anciens logs
        cleanup_old_logs()
        
        return filepath
    except Exception as e:
        print(f"❌ Erreur lors de la sauvegarde: {e}")
        return None

def load_game_pgn(filepath):
    """
    Charge une partie depuis un fichier PGN. 
    
    Args:
        filepath: Chemin du fichier PGN
        
    Returns:
        Un objet chess.pgn.Game ou None en cas d'erreur
    """
    try:
        with open(filepath) as pgn_file:
            game = chess.pgn.read_game(pgn_file)
        return game
    except Exception as e:
        print(f"❌ Erreur lors du chargement: {e}")
        return None

def get_recent_games():
    """
    Retourne les informations des 5 dernières parties. 
    
    Returns:
        Liste de dictionnaires contenant les métadonnées des parties
    """
    files = get_log_files()[:MAX_GAMES]
    games_info = []
    
    for filename in files:
        filepath = os.path.join(LOGS_DIR, filename)
        game = load_game_pgn(filepath)
        
        if game:
            games_info.append({
                'filename': filename,
                'filepath': filepath,
                'date':  game.headers.get("Date", "N/A"),
                'white': game.headers.get("White", "N/A"),
                'black': game.headers.get("Black", "N/A"),
                'result': game.headers.get("Result", "N/A"),
                'time_control': game.headers.get("TimeControl", "N/A")
            })
    
    return games_info

def format_game_summary(game_info):
    """Formate un résumé lisible d'une partie."""
    return f"{game_info['date']} - {game_info['white']} vs {game_info['black']} :  {game_info['result']}"