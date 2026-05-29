#include "logger.h"
#include "search.h"
#include "uci.h"
#include <stdio.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

static Board wasm_board;
static int wasm_board_initialized = 0;

EMSCRIPTEN_KEEPALIVE
void pallas_init() {
  logger_init();
  init_zobrist();
  initialize_engine();
  board_from_fen(&wasm_board,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  wasm_board_initialized = 1;
}

EMSCRIPTEN_KEEPALIVE
void pallas_uci_command(const char* command) {
  if (!wasm_board_initialized) {
    pallas_init();
  }
  char line[4096];
  strncpy(line, command, sizeof(line) - 1);
  line[sizeof(line) - 1] = '\0';
  parse_uci_command(line, &wasm_board);
}

EMSCRIPTEN_KEEPALIVE
void pallas_reset() {
  initialize_engine();
  board_from_fen(&wasm_board,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  wasm_board_initialized = 1;
}

EMSCRIPTEN_KEEPALIVE
char* pallas_get_legal_moves(void) {
  if (!wasm_board_initialized) pallas_init();
  static char buffer[4096];
  MoveList moves;
  movelist_init(&moves);
  generate_legal_moves(&wasm_board, &moves);
  int pos = 0;
  for (int i = 0; i < moves.count; i++) {
    if (i > 0) buffer[pos++] = ' ';
    const char *str = move_to_string(&moves.moves[i]);
    for (int j = 0; str[j] && pos < (int)sizeof(buffer) - 2; j++)
      buffer[pos++] = str[j];
  }
  buffer[pos] = '\0';
  return buffer;
}

EMSCRIPTEN_KEEPALIVE
int pallas_is_legal_move(const char *move_str) {
  if (!wasm_board_initialized) pallas_init();
  Move move = parse_uci_move(move_str);
  return is_move_legal(&wasm_board, &move);
}

int main() {
  pallas_init();
  return 0;
}
#else
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
#endif
