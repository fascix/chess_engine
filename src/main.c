#include "search.h"
#include "uci.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Initialiser les tables Zobrist
  init_zobrist();

  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    printf("Mode test - TODO: Lancer tests\n");
    return 0;
  } else {
    // Mode UCI par défaut
    uci_loop();
  }

  return 0;
}
