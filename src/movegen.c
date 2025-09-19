#include "movegen.h"
#include <stdio.h>
#include <string.h>

// Initialise une liste de coups vide
void movelist_init(MoveList *list) { list->count = 0; }

// Ajoute un coup à la liste
void movelist_add(MoveList *list, Move move) {
  if (list->count < 256) {
    list->moves[list->count] = move;
    list->count++;
  }
}

// Crée un coup simple (normal ou capture)
Move create_move(Square from, Square to, MoveType type) {
  Move move;
  move.from = from;
  move.to = to;
  move.type = type;
  move.promotion = EMPTY;
  move.captured_piece = EMPTY;
  return move;
}

// Crée un coup de promotion
Move create_promotion_move(Square from, Square to, PieceType promotion) {
  Move move;
  move.from = from;
  move.to = to;
  move.type = MOVE_PROMOTION;
  move.promotion = promotion;
  move.captured_piece = EMPTY;
  return move;
}

// Affiche un coup en notation lisible
void print_move(const Move *move) {
  // Conversion Square → coordonnées (A1, B2, etc.)
  char from_str[3] = {'a' + (move->from % 8), '1' + (move->from / 8), '\0'};
  char to_str[3] = {'a' + (move->to % 8), '1' + (move->to / 8), '\0'};

  printf("%s%s", from_str, to_str);

  // Affichage spécial selon le type
  switch (move->type) {
  case MOVE_PROMOTION: {
    const char pieces[] = "PNBRQK"; // Pas d'espace ! PAWN=0->P, QUEEN=4->Q
    printf("=%c", pieces[move->promotion]);
    break;
  }
  case MOVE_CASTLE:
    printf(" (roque)");
    break;
  case MOVE_EN_PASSANT:
    printf(" (en passant)");
    break;
  case MOVE_CAPTURE:
    printf(" (capture)");
    break;
  default:
    break;
  }
}

// Affiche toute la liste de coups
void print_movelist(const MoveList *list) {
  printf("Coups générés (%d) :\n", list->count);
  for (int i = 0; i < list->count; i++) {
    printf("%2d. ", i + 1);
    print_move(&list->moves[i]);
    printf("\n");
  }
}

// TODO: Implémentation des fonctions de génération
void generate_moves(const Board *board, MoveList *moves) {
  movelist_init(moves);

  // Pour l'instant, générer seulement les coups de pions
  generate_pawn_moves(board, board->to_move, moves);
}

void generate_pawn_moves(const Board *board, Couleur color, MoveList *moves) {
  // Récupérer le bitboard des pions de cette couleur
  Bitboard pawns = board->pieces[color][PAWN];

  // Direction des pions : blancs +8 (vers le haut), noirs -8 (vers le bas)
  int direction = (color == WHITE) ? 8 : -8;

  // Rangée de départ pour le double saut
  int start_rank =
      (color == WHITE) ? 1 : 6; // Rangée 2 pour blancs, 7 pour noirs

  // Parcourir tous les pions avec la méthode bitboard
  while (pawns != 0) {
    Square from = __builtin_ctzll(pawns); // Trouve le premier pion
    pawns &= (pawns - 1);                 // Efface ce bit

    // Vérifier la validité de la case
    if (from < A1 || from > H8) {
      continue;
    }

    // Case de destination pour avancer d'une case
    Square one_forward = from + direction;

    // Vérification que la case est sur le plateau
    if (one_forward >= A1 && one_forward <= H8) {

      // 1. MOUVEMENT SIMPLE - Avancer d'une case
      if (!is_square_occupied(board, one_forward)) {
        // Vérifier si promotion (pion atteint la dernière rangée)
        int rank = one_forward / 8;
        if ((color == WHITE && rank == 7) || (color == BLACK && rank == 0)) {
          // Générer les 4 promotions possibles
          movelist_add(moves, create_promotion_move(from, one_forward, QUEEN));
          movelist_add(moves, create_promotion_move(from, one_forward, ROOK));
          movelist_add(moves, create_promotion_move(from, one_forward, BISHOP));
          movelist_add(moves, create_promotion_move(from, one_forward, KNIGHT));
        } else {
          // Coup normal
          movelist_add(moves, create_move(from, one_forward, MOVE_NORMAL));
        }

        // 2. DOUBLE SAUT - Si première rangée et case libre
        Square two_forward = from + (2 * direction);
        if ((from / 8) == start_rank && two_forward >= A1 &&
            two_forward <= H8 && !is_square_occupied(board, two_forward)) {
          movelist_add(moves, create_move(from, two_forward, MOVE_NORMAL));
        }
      }

      // 3. CAPTURES EN DIAGONALE
      // Diagonale gauche et droite selon la couleur
      int left_capture = (color == WHITE) ? from + 7 : from - 9;
      int right_capture = (color == WHITE) ? from + 9 : from - 7;

      // Capture diagonale gauche
      if (left_capture >= A1 && left_capture <= H8 &&
          (from % 8) != 0) { // Vérifier qu'on ne déborde pas sur la colonne A
        if (is_square_occupied(board, left_capture) &&
            get_piece_color(board, left_capture) != color) {
          PieceType captured = get_piece_type(board, left_capture);

          // Vérifier promotion
          int rank = left_capture / 8;
          if ((color == WHITE && rank == 7) || (color == BLACK && rank == 0)) {
            Move capture = create_promotion_move(from, left_capture, QUEEN);
            capture.type = MOVE_PROMOTION;
            capture.captured_piece = captured;
            movelist_add(moves, capture);

            capture.promotion = ROOK;
            movelist_add(moves, capture);
            capture.promotion = BISHOP;
            movelist_add(moves, capture);
            capture.promotion = KNIGHT;
            movelist_add(moves, capture);
          } else {
            Move capture = create_move(from, left_capture, MOVE_CAPTURE);
            capture.captured_piece = captured;
            movelist_add(moves, capture);
          }
        }
      }

      // Capture diagonale droite
      if (right_capture >= A1 && right_capture <= H8 &&
          (from % 8) != 7) { // Vérifier qu'on ne déborde pas sur la colonne H
        if (is_square_occupied(board, right_capture) &&
            get_piece_color(board, right_capture) != color) {
          PieceType captured = get_piece_type(board, right_capture);

          // Vérifier promotion
          int rank = right_capture / 8;
          if ((color == WHITE && rank == 7) || (color == BLACK && rank == 0)) {
            Move capture = create_promotion_move(from, right_capture, QUEEN);
            capture.type = MOVE_PROMOTION;
            capture.captured_piece = captured;
            movelist_add(moves, capture);

            capture.promotion = ROOK;
            movelist_add(moves, capture);
            capture.promotion = BISHOP;
            movelist_add(moves, capture);
            capture.promotion = KNIGHT;
            movelist_add(moves, capture);
          } else {
            Move capture = create_move(from, right_capture, MOVE_CAPTURE);
            capture.captured_piece = captured;
            movelist_add(moves, capture);
          }
        }
      }
    }
  }
}
