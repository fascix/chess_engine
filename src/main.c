
#include "uci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Main complet pour lancer un perft interactif
int main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    printf("Mode test - TODO: Lancer tests\n");
    return 0;
  }

  // Boucle UCI principale
  uci_loop();

  return 0;
}