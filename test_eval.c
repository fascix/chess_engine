#include "src/board.h"
#include "src/evaluation.h"
#include "src/movegen.h"
#include <stdio.h>

int main() {
  Board board;
  board_init(&board);

  printf("Position de départ:\n");
  print_board(&board);

  int eval = evaluate_position(&board);
  printf("\nÉvaluation: %d\n", eval);

  printf("\nDétails:\n");
  printf("- Matériel: %d\n", evaluate_material(&board));
  printf("- PST: %d\n", evaluate_piece_square_tables(&board));
  printf("- Contrôle centre: %d\n", evaluate_center_control(&board));
  printf("- Développement sûr: %d\n", evaluate_safe_development(&board));

  // Tester après e2e4
  Move move;
  move.from = E2;
  move.to = E4;
  move.type = MOVE_NORMAL;
  move.promotion = EMPTY;
  move.captured_piece = EMPTY;

  Board backup;
  make_move_temp(&board, &move, &backup);

  printf("\n\nAprès e2e4:\n");
  print_board(&board);

  eval = evaluate_position(&board);
  printf("\nÉvaluation: %d\n", eval);
  printf("(Du point de vue des NOIRS car c'est à leur tour)\n");

  printf("\nDétails:\n");
  printf("- Matériel: %d\n", evaluate_material(&board));
  printf("- PST: %d\n", evaluate_piece_square_tables(&board));
  printf("- Contrôle centre: %d\n", evaluate_center_control(&board));

  return 0;
}
