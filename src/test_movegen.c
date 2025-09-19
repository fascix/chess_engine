// Test simple des utilitaires movegen
#include "movegen.h"
#include <stdio.h>

int main() {
  printf("=== Test Utilitaires Movegen ===\n");

  // Test création de coups
  Move move1 = create_move(12, 28, MOVE_NORMAL);         // e2e4
  Move move2 = create_move(12, 20, MOVE_NORMAL);         // e2e3
  Move promotion = create_promotion_move(48, 56, QUEEN); // a7a8=Q

  printf("Coup 1: ");
  print_move(&move1);
  printf("\n");

  printf("Coup 2: ");
  print_move(&move2);
  printf("\n");

  printf("Promotion: ");
  print_move(&promotion);
  printf("\n");

  // Test MoveList
  MoveList list;
  movelist_init(&list);
  movelist_add(&list, move1);
  movelist_add(&list, move2);
  movelist_add(&list, promotion);

  printf("\n");
  print_movelist(&list);

  // Test génération de coups de pions
  printf("\n=== Test Génération Pions ===\n");
  Board board;
  board_init(&board); // Position initiale

  MoveList pawn_moves;
  generate_pawn_moves(&board, WHITE, &pawn_moves);

  printf("Coups de pions blancs en position initiale:\n");
  print_movelist(&pawn_moves);

  // Test avec captures - Position avec pièces adverses
  printf("\n=== Test avec captures ===\n");
  Board test_board;
  board_from_fen(
      &test_board,
      "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
  print_board(&test_board);

  MoveList capture_moves;
  generate_pawn_moves(&test_board, WHITE, &capture_moves);

  printf("Coups de pions blancs (avec captures possibles):\n");
  print_movelist(&capture_moves);

  return 0;
}
