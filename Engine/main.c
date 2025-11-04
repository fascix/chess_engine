#include "search.h"
#include "uci.h"

// main.c
int main() {
  initialize_engine();
  uci_loop();
  return 0;
}
