"""
Main qui sert de boucle pour le jeu
"""
import pygame
from settings import *
from menu import main_menu

def main():
    """Fonction principale qui lance le jeu."""
    # Initialiser Pygame
    pygame.init()
    
    # Configuration de la fenêtre
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption("Chess Engine")
    
    # Lancer le menu principal
    main_menu()

if __name__ == "__main__":
    main()
