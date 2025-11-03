#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "movegen.h"

// Variables globales pour l'état UCI
extern volatile int uci_debug;

// Structure pour les options UCI configurables
typedef struct {
  int hash_size_mb; // Taille de la table de transposition (MB)
  int ponder;       // Pondering activé (0/1)
  int own_book;     // Utiliser le livre d'ouvertures (0/1)
  int analyse_mode; // Mode analyse UCI (0/1)
} UCIOptions;

extern UCIOptions uci_options;

// Boucle principale UCI
void uci_loop();

// Parser de commandes
void parse_uci_command(char *line, Board *board);

// Gestionnaires de commandes spécifiques
void handle_uci();
void handle_isready();
void handle_debug(char *params);
void handle_setoption(char *params);
void handle_register(char *params);
void handle_ucinewgame();
void handle_position(Board *board, char *params);
void handle_go(Board *board, char *params);
void handle_ponderhit();
void handle_stop();
void handle_quit();

// Fonctions utilitaires pour positions
void setup_startpos(Board *board);
void setup_from_fen(Board *board, char *params);
void apply_uci_moves(Board *board, char *moves_str);
Move parse_uci_move(const char *uci_str);

#endif // UCI_H
