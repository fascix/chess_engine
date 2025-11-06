"""
Module contenant les différents modes de jeu
"""
import pygame
import chess
import time
from settings import *
import game_logic
import gui

def solo_game(timer, player_is_white=True):
    """Mode de jeu à 2 joueurs."""
    pygame.display.set_caption("Chess Engine - Mode 2 joueurs")
    
    # Réinitialiser le jeu
    game_logic.reset_game()
    game_logic.player_is_white = player_is_white
    
    # Initialiser les timers
    game_logic.current_turn_start_time = time.time()
    game_logic.white_timer = game_logic.black_timer = timer

    screen = pygame.display.set_mode((WINDOW_SIZE + 300, WINDOW_SIZE))
    font = pygame.font.Font(None, 48)

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    from menu import pause_menu, main_menu
                    result = pause_menu()
                    if result == "main_menu":
                        main_menu()
                        return

            elif event.type == pygame.MOUSEBUTTONDOWN and not game_logic.game_paused:
                print(f"\n===== EVENT: MOUSEBUTTONDOWN at {event.pos} =====")
                game_logic.handle_click(event, game_logic.board)

            elif event.type == pygame.MOUSEMOTION and not game_logic.game_paused:
                game_logic.handle_drag(event, game_logic.board)

            elif event.type == pygame.MOUSEBUTTONUP and not game_logic.game_paused:
                print(f"\n===== EVENT: MOUSEBUTTONUP at {event.pos} =====")
                game_logic.handle_drop(event, game_logic.board)

        # Mise à jour du timer
        game_logic.update_timers()

        # Vérification si un joueur a épuisé son temps
        if game_logic.is_time_up():
            winner = game_logic.get_time_winner()
            if winner:
                print(f"Temps écoulé ! {winner}")
                running = False

        # Rendu du jeu
        gui.render_game(screen, game_logic.board, font, player_is_white)
        pygame.display.flip()
        
        # Check for checkmate AFTER rendering so the last move is visible
        if game_logic.board.is_checkmate():
            pygame.time.wait(500)  # Brief pause to see the final position
            gui.display_checkmate(screen, game_logic.board.turn)
            running = False

    pygame.quit()

def chess_engine(timer, player_is_white=True):
    """Mode de jeu contre l'ordinateur."""
    pygame.display.set_caption("Chess Engine - Contre l'ordinateur")
    
    # Réinitialiser le jeu
    game_logic.reset_game()
    game_logic.player_is_white = player_is_white
    
    # Initialiser les timers
    game_logic.current_turn_start_time = time.time()
    game_logic.white_timer = game_logic.black_timer = timer

    screen = pygame.display.set_mode((WINDOW_SIZE + 300, WINDOW_SIZE))
    font = pygame.font.Font(None, 48)
    
    # Si le joueur joue les noirs et c'est le tour des blancs, démarrer le calcul du bot
    if not player_is_white and game_logic.board.turn == chess.WHITE:
        game_logic.start_engine_calculation(game_logic.board)

    try:
        running = True
        while running:
            # Vérifier si le moteur a terminé son calcul
            if game_logic.engine_thinking:
                game_logic.check_engine_result()
            
            # Si le coup du moteur est prêt, l'exécuter
            if game_logic.engine_move_ready and game_logic.pending_engine_move and not game_logic.game_paused:
                if game_logic.pending_engine_move in game_logic.board.legal_moves:
                    game_logic.execute_move(game_logic.board, game_logic.pending_engine_move)
                        
                game_logic.engine_move_ready = False
                game_logic.pending_engine_move = None
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        from menu import pause_menu, main_menu
                        result = pause_menu()
                        if result == "main_menu":
                            main_menu()
                            return

                elif event.type == pygame.MOUSEBUTTONDOWN and not game_logic.game_paused and not game_logic.engine_thinking:
                    # Le joueur peut seulement jouer si c'est son tour
                    if (player_is_white and game_logic.board.turn == chess.WHITE) or (not player_is_white and game_logic.board.turn == chess.BLACK):
                        game_logic.handle_click(event, game_logic.board)

                elif event.type == pygame.MOUSEMOTION and not game_logic.game_paused and not game_logic.engine_thinking:
                    game_logic.handle_drag(event, game_logic.board)

                elif event.type == pygame.MOUSEBUTTONUP and not game_logic.game_paused and not game_logic.engine_thinking:
                    if (player_is_white and game_logic.board.turn == chess.WHITE) or (not player_is_white and game_logic.board.turn == chess.BLACK):
                        game_logic.handle_drop(event, game_logic.board)
                        # Après le coup du joueur, démarrer le calcul du bot si c'est son tour
                        if (player_is_white and game_logic.board.turn == chess.BLACK) or (not player_is_white and game_logic.board.turn == chess.WHITE):
                            if not game_logic.board.is_game_over():
                                game_logic.start_engine_calculation(game_logic.board)

            # Mise à jour du timer
            game_logic.update_timers()

            # Vérification si un joueur a épuisé son temps
            if game_logic.is_time_up():
                winner = game_logic.get_time_winner()
                if winner:
                    print(f"Temps écoulé ! {winner}")
                    running = False

            # Rendu du jeu
            gui.render_game(screen, game_logic.board, font, player_is_white)
            pygame.display.flip()
            
            # Check for checkmate AFTER rendering so the last move is visible
            if game_logic.board.is_checkmate():
                pygame.time.wait(500)  # Brief pause to see the final position
                gui.display_checkmate(screen, game_logic.board.turn)
                running = False
    finally:
        # engine.quit() # Commenté car on utilise un moteur simulé
        pass

def bot_vs_bot(timer):
    """Mode de jeu bot contre bot."""
    pygame.display.set_caption("Chess Engine - Bot vs Bot")
    
    # Réinitialiser le jeu
    game_logic.reset_game()
    game_logic.player_is_white = True  # Always view from white's perspective
    
    # Initialiser les timers
    game_logic.current_turn_start_time = time.time()
    game_logic.white_timer = game_logic.black_timer = timer

    screen = pygame.display.set_mode((WINDOW_SIZE + 300, WINDOW_SIZE))
    font = pygame.font.Font(None, 48)
    
    # Démarrer le calcul du premier bot (blancs)
    game_logic.start_engine_calculation(game_logic.board)

    try:
        running = True
        while running:
            # Vérifier si le moteur 1 (bot blanc) a terminé son calcul
            if game_logic.engine_thinking:
                game_logic.check_engine_result()
            
            # Vérifier si le moteur 2 (bot noir) a terminé son calcul
            if game_logic.engine2_thinking:
                game_logic.check_engine2_result()
            
            # Si le coup du bot blanc est prêt, l'exécuter
            if game_logic.engine_move_ready and game_logic.pending_engine_move and not game_logic.game_paused:
                if game_logic.pending_engine_move in game_logic.board.legal_moves:
                    game_logic.execute_move(game_logic.board, game_logic.pending_engine_move)
                    # Démarrer le bot noir si la partie n'est pas terminée
                    if not game_logic.board.is_game_over():
                        game_logic.start_engine2_calculation(game_logic.board)
                        
                game_logic.engine_move_ready = False
                game_logic.pending_engine_move = None
            
            # Si le coup du bot noir est prêt, l'exécuter
            if game_logic.engine2_move_ready and game_logic.pending_engine2_move and not game_logic.game_paused:
                if game_logic.pending_engine2_move in game_logic.board.legal_moves:
                    game_logic.execute_move(game_logic.board, game_logic.pending_engine2_move)
                    # Démarrer le bot blanc si la partie n'est pas terminée
                    if not game_logic.board.is_game_over():
                        game_logic.start_engine_calculation(game_logic.board)
                        
                game_logic.engine2_move_ready = False
                game_logic.pending_engine2_move = None
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        from menu import pause_menu, main_menu
                        result = pause_menu()
                        if result == "main_menu":
                            main_menu()
                            return

            # Mise à jour du timer
            game_logic.update_timers()

            # Vérification si un joueur a épuisé son temps
            if game_logic.is_time_up():
                winner = game_logic.get_time_winner()
                if winner:
                    print(f"Temps écoulé ! {winner}")
                    running = False

            # Rendu du jeu
            gui.render_game(screen, game_logic.board, font, True)
            pygame.display.flip()
            
            # Check for checkmate AFTER rendering so the last move is visible
            if game_logic.board.is_checkmate():
                pygame.time.wait(500)  # Brief pause to see the final position
                gui.display_checkmate(screen, game_logic.board.turn)
                running = False
    finally:
        pass
