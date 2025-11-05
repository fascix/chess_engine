#include "search.h"
#include "uci.h"

// main.c
int main() {
  init_zobrist();
  initialize_engine();
  uci_loop();
  return 0;
}
