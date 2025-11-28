"""
Main qui sert de boucle pour le jeu
"""
import pygame
from menu import main_menu

def main():
    """Fonction principale qui lance le jeu en plein écran."""
    pygame.init()
    
    # Mode plein écran natif, résolution de l'écran
    screen = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)
    info = pygame.display.Info()
    screen_width, screen_height = info.current_w, info.current_h
    print(f"Résolution plein écran : {screen_width}x{screen_height}")
    
    pygame.display.set_caption("Chess Engine")
    
    # Lancer le menu principal (qui utilisera la surface actuelle)
    main_menu()

if __name__ == "__main__":
    main()