#include "logger.h"
#include "search.h"
#include "uci.h"
#include <stdio.h>

// main.c
int main() {
  // Désactiver le buffering pour une communication UCI fluide
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  // Initialize the logging system
  logger_init();

  init_zobrist();
  initialize_engine();
  uci_loop();
  return 0;
}
