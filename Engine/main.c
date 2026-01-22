#include "search.h"
#include "uci.h"
#include "logger.h"

// main.c
int main() {
  // Initialize the logging system
  logger_init();
  
  init_zobrist();
  initialize_engine();
  uci_loop();
  return 0;
}
