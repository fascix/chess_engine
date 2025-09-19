// Test simple pour vérifier la compilation
#include "board.h"
#include <stdio.h>

int main() {
  Board board;
  board_init(&board);

  printf("Chess engine - Bitboard test\n");
  printf("Position initiale chargée avec succès!\n");
  printf("Pièces blanches: 0x%llx\n", board.occupied[WHITE]);
  printf("Pièces noires: 0x%llx\n", board.occupied[BLACK]);

  // Test de nos nouvelles fonctions
  printf("\n--- Tests des fonctions ---\n");

  // Test case E1 (roi blanc) = case 4
  Square e1 = 4; // E1 dans votre énumération
  printf("Case E1 occupée ? %s\n",
         is_square_occupied(&board, e1) ? "Oui" : "Non");

  if (is_square_occupied(&board, e1)) {
    Couleur color = get_piece_color(&board, e1);
    printf("Couleur sur E1: %s\n", color == WHITE ? "Blanc" : "Noir");

    PieceType type = get_piece_type(&board, e1);
    const char *piece_names[] = {"EMPTY", "PAWN",  "KNIGHT", "BISHOP",
                                 "ROOK",  "QUEEN", "KING"};
    printf("Type sur E1: %s\n", piece_names[type + 1]); // +1 car EMPTY = -1
  }

  // Test case E4 (case vide) = case 28
  Square e4 = 28;
  printf("Case E4 occupée ? %s\n",
         is_square_occupied(&board, e4) ? "Oui" : "Non");

  // Test get_piece_type sur case vide
  PieceType empty_type = get_piece_type(&board, e4);
  printf("Type sur E4 (vide): %s\n", empty_type == EMPTY ? "EMPTY" : "ERREUR!");

  printf("\n--- Affichage du plateau initial ---\n");
  print_board(&board);

  // Test du parser FEN
  printf("\n--- Test de fen_char_to_piece_info ---\n");
  PieceType type;
  Couleur couleur;

  fen_char_to_piece_info('r', &type, &couleur);
  printf(
      "'r' → Type: %d, Couleur: %d (attendu: type=3/ROOK, couleur=1/BLACK)\n",
      type, couleur);

  fen_char_to_piece_info('R', &type, &couleur);
  printf(
      "'R' → Type: %d, Couleur: %d (attendu: type=3/ROOK, couleur=0/WHITE)\n",
      type, couleur);

  printf("\n--- Test board_from_fen ---\n");
  const char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
  board_from_fen(&board, fen);

  // Diagnostic: vérifier les bitboards après parsing
  printf("Après parsing FEN:\n");
  printf("Tours noires: 0x%llx (attendu: cases A8=56 et H8=63)\n",
         board.pieces[BLACK][ROOK]);
  printf("Pions blancs: 0x%llx\n", board.pieces[WHITE][PAWN]);

  // DIAGNOSTIC CRITIQUE: les bitboards occupied[]
  printf("occupied[WHITE]: 0x%llx\n", board.occupied[WHITE]);
  printf("occupied[BLACK]: 0x%llx\n", board.occupied[BLACK]);
  printf("all_pieces: 0x%llx\n", board.all_pieces);

  printf("Case A8 (56) occupée ? %s\n",
         is_square_occupied(&board, 56) ? "Oui" : "Non");

  if (is_square_occupied(&board, 56)) {
    printf("Type sur A8: %d (attendu: 3 pour ROOK)\n",
           get_piece_type(&board, 56));
    printf("Couleur sur A8: %d (attendu: 1 pour BLACK)\n",
           get_piece_color(&board, 56));
  }

  printf("Position depuis FEN: %s\n", fen);
  print_board(&board);

  // Test avec une position différente
  printf("\n--- Test avec position personnalisée ---\n");
  const char *fen2 = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R";
  board_from_fen(&board, fen2);

  printf("Position depuis FEN: %s\n", fen2);
  print_board(&board);

  return 0;
}
