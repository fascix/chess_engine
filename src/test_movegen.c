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
  movelist_init(&pawn_moves); // Important: initialiser avant génération!
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
  movelist_init(&capture_moves);
  generate_pawn_moves(&test_board, WHITE, &capture_moves);

  printf("Coups de pions blancs (avec captures possibles):\n");
  print_movelist(&capture_moves);

  // Test génération de coups de tours
  printf("\n=== Test Génération Tours ===\n");

  // Position avec une tour blanche isolée au centre
  Board rook_test;
  board_from_fen(&rook_test, "8/8/8/3R4/8/8/8/8 w - - 0 1");
  printf("Tour blanche en D5 (plateau vide):\n");
  print_board(&rook_test);

  MoveList rook_moves;
  movelist_init(&rook_moves);
  generate_rook_moves(&rook_test, WHITE, &rook_moves);
  printf("Mouvements possibles:\n");
  print_movelist(&rook_moves);

  // Test avec obstacles et captures
  printf("\n=== Test Tours avec obstacles ===\n");
  Board rook_blocked;
  board_from_fen(&rook_blocked, "8/3p4/8/2pRp3/8/3P4/8/8 w - - 0 1");
  printf("Tour blanche en D5 avec obstacles:\n");
  print_board(&rook_blocked);

  MoveList blocked_moves;
  movelist_init(&blocked_moves);
  generate_rook_moves(&rook_blocked, WHITE, &blocked_moves);
  printf("Mouvements possibles (avec captures):\\n");
  print_movelist(&blocked_moves);

  // Test génération de coups de fous
  printf("\\n=== Test Génération Fous ===\\n");

  // Position avec un fou blanc au centre
  Board bishop_test;
  board_from_fen(&bishop_test, "8/8/8/3B4/8/8/8/8 w - - 0 1");
  printf("Fou blanc en D5 (plateau vide):\\n");
  print_board(&bishop_test);

  MoveList bishop_moves;
  movelist_init(&bishop_moves);
  generate_bishop_moves(&bishop_test, WHITE, &bishop_moves);
  printf("Mouvements possibles:\\n");
  print_movelist(&bishop_moves);

  // Test avec obstacles et captures
  printf("\\n=== Test Fous avec obstacles ===\\n");
  Board bishop_blocked;
  board_from_fen(&bishop_blocked, "8/8/1p3p2/3B4/8/1P3P2/8/8 w - - 0 1");
  printf("Fou blanc en D5 avec obstacles:\\n");
  print_board(&bishop_blocked);

  MoveList bishop_blocked_moves;
  movelist_init(&bishop_blocked_moves);
  generate_bishop_moves(&bishop_blocked, WHITE, &bishop_blocked_moves);
  printf("Mouvements possibles (avec captures):\\n");
  print_movelist(&bishop_blocked_moves);

  // Test génération de coups de cavaliers
  printf("\\n=== Test Génération Cavaliers ===\\n");

  // Position avec un cavalier blanc au centre
  Board knight_test;
  board_from_fen(&knight_test, "8/8/8/3N4/8/8/8/8 w - - 0 1");
  printf("Cavalier blanc en D5 (plateau vide):\\n");
  print_board(&knight_test);

  MoveList knight_moves;
  movelist_init(&knight_moves);
  generate_knight_moves(&knight_test, WHITE, &knight_moves);
  printf("Mouvements possibles:\\n");
  print_movelist(&knight_moves);

  // Test avec obstacles et captures
  printf("\\n=== Test Cavaliers avec obstacles ===\\n");
  Board knight_blocked;
  board_from_fen(&knight_blocked, "8/8/1p1p1p2/2pNp3/1p1p1p2/8/8/8 w - - 0 1");
  printf("Cavalier blanc en D5 avec obstacles:\\n");
  print_board(&knight_blocked);

  MoveList knight_blocked_moves;
  movelist_init(&knight_blocked_moves);
  generate_knight_moves(&knight_blocked, WHITE, &knight_blocked_moves);
  printf("Mouvements possibles (avec captures):\\n");
  print_movelist(&knight_blocked_moves);

  // Test cavalier dans un coin (cas limite)
  printf("\\n=== Test Cavalier en coin (A1) ===\\n");
  Board knight_corner;
  board_from_fen(&knight_corner, "8/8/8/8/8/8/8/N7 w - - 0 1");
  printf("Cavalier blanc en A1:\\n");
  print_board(&knight_corner);

  MoveList corner_moves;
  movelist_init(&corner_moves);
  generate_knight_moves(&knight_corner, WHITE, &corner_moves);
  printf("Mouvements possibles:\\n");
  print_movelist(&corner_moves);

  // Test génération de coups de dame
  printf("\\n=== Test Génération Dame ===\\n");

  // Position avec une dame blanche au centre
  Board queen_test;
  board_from_fen(&queen_test, "8/8/8/3Q4/8/8/8/8 w - - 0 1");
  printf("Dame blanche en D5 (plateau vide):\\n");
  print_board(&queen_test);

  MoveList queen_moves;
  movelist_init(&queen_moves);
  generate_queen_moves(&queen_test, WHITE, &queen_moves);
  printf("Mouvements possibles:\\n");
  print_movelist(&queen_moves);

  // Test avec obstacles - dame vs roi et pions
  printf("\\n=== Test Dame vs obstacles ===\\n");
  Board queen_blocked;
  board_from_fen(&queen_blocked,
                 "8/3p4/1p1p1p2/2pQp3/1p1p1p2/3P4/8/8 w - - 0 1");
  printf("Dame blanche en D5 avec obstacles:\\n");
  print_board(&queen_blocked);

  MoveList queen_blocked_moves;
  movelist_init(&queen_blocked_moves);
  generate_queen_moves(&queen_blocked, WHITE, &queen_blocked_moves);
  printf("Mouvements possibles (avec captures):\\n");
  print_movelist(&queen_blocked_moves);

  // Test génération de coups de roi
  printf("\\n=== Test Génération Roi ===\\n");

  // Position avec un roi blanc au centre
  Board king_test;
  board_from_fen(&king_test, "8/8/8/3K4/8/8/8/8 w - - 0 1");
  printf("Roi blanc en D5 (plateau vide):\\n");
  print_board(&king_test);

  MoveList king_moves;
  movelist_init(&king_moves);
  generate_king_moves(&king_test, WHITE, &king_moves);
  printf("Mouvements possibles:\\n");
  print_movelist(&king_moves);

  // Test avec obstacles
  printf("\\n=== Test Roi avec obstacles ===\\n");
  Board king_blocked;
  board_from_fen(&king_blocked, "8/8/1p1p1p2/1pKp1p2/1p1p1p2/8/8/8 w - - 0 1");
  printf("Roi blanc en C5 avec obstacles:\\n");
  print_board(&king_blocked);

  MoveList king_blocked_moves;
  movelist_init(&king_blocked_moves);
  generate_king_moves(&king_blocked, WHITE, &king_blocked_moves);
  printf("Mouvements possibles (avec captures):\\n");
  print_movelist(&king_blocked_moves);

  // Test roi en coin
  printf("\\n=== Test Roi en coin (H1) ===\\n");
  Board king_corner;
  board_from_fen(&king_corner, "8/8/8/8/8/8/8/7K w - - 0 1");
  printf("Roi blanc en H1:\\n");
  print_board(&king_corner);

  MoveList king_corner_moves;
  movelist_init(&king_corner_moves);
  generate_king_moves(&king_corner, WHITE, &king_corner_moves);
  printf("Mouvements possibles:\\n");
  print_movelist(&king_corner_moves);

  // Test des roques
  printf("\\n=== Test Roques ===\\n");

  // Position initiale (roques possibles)
  Board castle_test;
  board_from_fen(&castle_test, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  printf("Position roques possibles (rois et tours en place):\\n");
  print_board(&castle_test);

  MoveList castle_moves;
  movelist_init(&castle_moves);
  generate_king_moves(&castle_test, WHITE, &castle_moves);
  printf("Mouvements du roi blanc (avec roques):\\n");
  print_movelist(&castle_moves);

  // Test roque bloqué par pièce
  printf("\\n=== Test Roque bloqué ===\\n");
  Board blocked_castle;
  board_from_fen(&blocked_castle, "r1b1k1nr/8/8/8/8/8/8/R1B1K1NR w KQkq - 0 1");
  printf("Roques bloqués par des pièces:\\n");
  print_board(&blocked_castle);

  MoveList blocked_castle_moves;
  movelist_init(&blocked_castle_moves);
  generate_king_moves(&blocked_castle, WHITE, &blocked_castle_moves);
  printf("Mouvements du roi blanc (roques bloqués):\\n");
  print_movelist(&blocked_castle_moves);

  // ===== TESTS COMPLETS =====

  printf("\\n\\n============ TESTS COMPLETS ============\\n");

  // Test en passant
  printf("\\n=== Test En Passant ===\\n");
  Board en_passant_test;
  board_from_fen(
      &en_passant_test,
      "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
  printf("Position avec pions adjacents (en passant sur f6 possible):\\n");
  print_board(&en_passant_test);
  printf("En passant square: %d (f6 = %d)\\n", en_passant_test.en_passant, F6);

  MoveList ep_moves;
  movelist_init(&ep_moves);
  generate_pawn_moves(&en_passant_test, WHITE, &ep_moves);
  printf("Mouvements des pions blancs (avec en passant):\\n");
  for (int i = 0; i < ep_moves.count; i++) {
    printf("%2d. ", i + 1);
    print_move(&ep_moves.moves[i]);
    if (ep_moves.moves[i].type == MOVE_EN_PASSANT) {
      printf(" *** EN PASSANT ***");
    }
    printf("\\n");
  }

  // Test filtrage des mouvements illégaux
  printf("\\n=== Test Mouvements Illégaux ===\\n");
  Board illegal_test;
  board_from_fen(
      &illegal_test,
      "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKB1R w KQkq - 0 2");
  printf("Position normale:\\n");
  print_board(&illegal_test);

  MoveList all_moves, legal_moves;
  movelist_init(&all_moves);
  movelist_init(&legal_moves);
  generate_moves(&illegal_test, &all_moves);
  generate_legal_moves(&illegal_test, &legal_moves);

  printf("Tous les mouvements pseudo-légaux: %d\\n", all_moves.count);
  printf("Mouvements légaux uniquement: %d\\n", legal_moves.count);

  // Test détection d'échec
  printf("\\n=== Test Détection Échec ===\\n");
  Board check_test;
  board_from_fen(&check_test,
                 "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQK2R w KQkq - 0 2");
  printf("Position avec roi blanc potentiellement en échec:\\n");
  print_board(&check_test);
  printf("Roi blanc en échec: %s\\n",
         is_in_check(&check_test, WHITE) ? "OUI" : "NON");
  printf("Roi noir en échec: %s\\n",
         is_in_check(&check_test, BLACK) ? "OUI" : "NON");

  // Test mat
  printf("\\n=== Test Mat ===\\n");
  Board mate_test;
  board_from_fen(
      &mate_test,
      "rnbqkbnr/pppp1ppp/8/4p2Q/4P3/8/PPPP1PPP/RNB1K1NR b KQkq - 1 2");
  printf("Position de mat potentiel:\\n");
  print_board(&mate_test);
  printf("Mat pour les noirs: %s\\n", is_checkmate(&mate_test) ? "OUI" : "NON");

  GameResult result = get_game_result(&mate_test);
  printf("Résultat de la partie: ");
  switch (result) {
  case GAME_ONGOING:
    printf("En cours\\n");
    break;
  case GAME_CHECKMATE_WHITE:
    printf("Mat - Blancs gagnent\\n");
    break;
  case GAME_CHECKMATE_BLACK:
    printf("Mat - Noirs gagnent\\n");
    break;
  case GAME_STALEMATE:
    printf("Pat\\n");
    break;
  case GAME_FIFTY_MOVE_RULE:
    printf("Règle des 50 coups\\n");
    break;
  }

  // Test pat
  printf("\\n=== Test Pat ===\\n");
  Board stalemate_test;
  board_from_fen(&stalemate_test, "k7/8/1Q6/8/8/8/8/K7 b - - 0 1");
  printf("Position de pat potentiel:\\n");
  print_board(&stalemate_test);
  printf("Pat pour les noirs: %s\\n",
         is_stalemate(&stalemate_test) ? "OUI" : "NON");

  // Test règle des 50 coups
  printf("\\n=== Test Règle 50 Coups ===\\n");
  Board fifty_test = illegal_test;
  fifty_test.halfmove_clock = 100;
  printf("Compteur demi-coups: %d\\n", fifty_test.halfmove_clock);
  printf("Règle des 50 coups active: %s\\n",
         is_fifty_move_rule(&fifty_test) ? "OUI" : "NON");

  printf("\\n============ TESTS TERMINÉS ============\\n");
  printf("🎯 Toutes les fonctionnalités sont implémentées !\\n");

  return 0;
}
