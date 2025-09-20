#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "search.h"

// Variables globales pour l'état UCI
extern int uci_debug;

// Boucle principale UCI
void uci_loop();

// Parser de commandes
void parse_uci_command(char *line, Board *board);

// Gestionnaires de commandes spécifiques
void handle_uci();
void handle_isready();
void handle_position(Board *board, char *params);
void handle_go(Board *board, char *params);
void handle_stop();
void handle_quit();

// Fonctions utilitaires pour positions
void setup_startpos(Board *board);
void setup_from_fen(Board *board, char *params);
void apply_uci_moves(Board *board, char *moves_str);
Move parse_uci_move(const char *uci_str);

// Fonctions utilitaires
char **split_string(char *str, char *delimiter, int *count);
void free_split_result(char **tokens, int count);

#endif // UCI_H
