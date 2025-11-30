import pygame
from menu import main_menu

def main():
    pygame.init()
    screen = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)
    pygame.display.set_caption("Chess Engine")
    main_menu()

if __name__ == "__main__":
    main()