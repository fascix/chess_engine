// board.c
#include "board.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

void board_init(Board *board) {
  // Initialise position de départ
  memset(board, 0, sizeof(Board));
  // Met tous les champs de Board à zéro avant initialisation

  // Position initiale des pièces blanches
  // ULL = Unsigned Long Long
  board->pieces[WHITE][ROOK] = 0x81ULL;   // a1, h1
  board->pieces[WHITE][KNIGHT] = 0x42ULL; // b1, g1
  board->pieces[WHITE][BISHOP] = 0x24ULL; // c1, f1
  board->pieces[WHITE][QUEEN] = 0x8ULL;   // d1
  board->pieces[WHITE][KING] = 0x10ULL;   // e1
  board->pieces[WHITE][PAWN] = 0xFF00ULL; // rangée 2

  // Position initiale des pièces noires
  board->pieces[BLACK][ROOK] = 0x8100000000000000ULL;
  board->pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;
  board->pieces[BLACK][BISHOP] = 0x2400000000000000ULL;
  board->pieces[BLACK][QUEEN] = 0x800000000000000ULL;
  board->pieces[BLACK][KING] = 0x1000000000000000ULL;
  board->pieces[BLACK][PAWN] = 0xFF000000000000ULL;

  // Calcul des bitboards dérivés
  board->occupied[WHITE] =
      board->pieces[WHITE][PAWN] | board->pieces[WHITE][KNIGHT] |
      board->pieces[WHITE][BISHOP] | board->pieces[WHITE][ROOK] |
      board->pieces[WHITE][QUEEN] | board->pieces[WHITE][KING];

  board->occupied[BLACK] =
      board->pieces[BLACK][PAWN] | board->pieces[BLACK][KNIGHT] |
      board->pieces[BLACK][BISHOP] | board->pieces[BLACK][ROOK] |
      board->pieces[BLACK][QUEEN] | board->pieces[BLACK][KING];

  board->all_pieces = board->occupied[WHITE] | board->occupied[BLACK];

  // État initial
  board->to_move = WHITE;
  board->castle_rights = 0xF; // Tous les roques possibles
  board->en_passant = -1;
  board->halfmove_clock = 0;
  board->move_number = 1;
}

bool is_square_occupied(const Board *board, Square square) {
  return GET_BIT(board->all_pieces, square);
}

Couleur get_piece_color(const Board *board, Square square) {

  if (!is_square_occupied(board, square)) {
    return NO_COLOR; // Retourne NO_COLOR pour case vide
  }
  if (GET_BIT(board->occupied[WHITE], square)) {
    return WHITE;
  } else {
    return BLACK;
  }
}
PieceType get_piece_type(const Board *board, Square square) {

  // 1. Vérification de sécurité
  if (!is_square_occupied(board, square))
    return EMPTY;

  // 2. Déterminer la couleur (cela permet de réduire la boucle du test de type
  // à 6 au lieu de 12)
  Couleur couleur = get_piece_color(board, square);

  // 3. Boucle sur les types
  for (PieceType type = PAWN; type <= KING; type++) {
    if (GET_BIT(board->pieces[couleur][type], square))
      return type;
  }
  return EMPTY; // Impossible mais petite sécurité au cas où
}

char get_piece_char(const Board *board, Square square) {
  if (!is_square_occupied(board, square))
    return '.';

  static const char pieces[] = "PNBRQK"; // Indices = enum values
  PieceType type = get_piece_type(board, square);
  Couleur couleur = get_piece_color(board, square);

  char c = pieces[type];
  return (couleur == BLACK) ? tolower(c) : c;
}

void print_board(const Board *board) {

  for (int ligne = 0; ligne < 8; ligne++) {
    printf("%d  ", 8 - ligne);
    for (int colonne = 0; colonne < 8; colonne++) {
      Square square = (7 - ligne) * 8 + colonne;
      printf("%c ", get_piece_char(board, square));
    }
    printf("\n");
  }
  printf("   a b c d e f g h\n");
}

void fen_char_to_piece_info(char c, PieceType *type, Couleur *couleur) {
  *couleur = isupper(c) ? WHITE : BLACK;

  switch (tolower(c)) {
  case 'p':
    *type = PAWN;
    break;
  case 'n':
    *type = KNIGHT;
    break;
  case 'b':
    *type = BISHOP;
    break;
  case 'r':
    *type = ROOK;
    break;
  case 'q':
    *type = QUEEN;
    break;
  case 'k':
    *type = KING;
    break;
  default:
    *type = EMPTY;
    break; // Sécurité
  }
}

void board_from_fen(Board *board, const char *fen) {

  memset(board, 0, sizeof(Board));

  int square = 56; // Commence par A8

  // Parser la partie pièces jusqu'au premier espace
  for (const char *p = fen; *p && *p != ' '; p++) {
    char c = *p;

    if (c == '/') {
      // Aller au début de la rangée précédente
      square = ((square - 1) / 8 - 1) * 8;
    } else if (isdigit(c)) {
      // Sauter des cases vides
      square += (c - '0');
    } else {
      // Placer une pièce
      PieceType type;
      Couleur couleur;
      fen_char_to_piece_info(c, &type, &couleur);
      SET_BIT(board->pieces[couleur][type], square);
      square++;
    }
  }

  // Recalculer les bitboards dérivés
  for (Couleur couleur = WHITE; couleur <= BLACK; couleur++) {
    board->occupied[couleur] = 0;
    for (PieceType type = PAWN; type <= KING; type++) {
      board->occupied[couleur] |= board->pieces[couleur][type];
    }
  }
  board->all_pieces = board->occupied[WHITE] | board->occupied[BLACK];

  // Parser le reste du FEN : joueur actuel, droits de roque, en passant, etc.
  const char *current = fen;

  // Aller au premier espace (fin des pièces)
  while (*current && *current != ' ')
    current++;
  if (!*current)
    return; // FEN incomplet, arrêter ici

  current++; // Passer l'espace

  // Parser le joueur actuel
  if (*current == 'w') {
    board->to_move = WHITE;
  } else if (*current == 'b') {
    board->to_move = BLACK;
  }
  current++;

  if (!*current || *current != ' ')
    return;  // FEN incomplet
  current++; // Passer l'espace

  // Parser les droits de roque
  board->castle_rights = 0;
  if (*current != '-') {
    while (*current && *current != ' ') {
      switch (*current) {
      case 'K':
        board->castle_rights |= WHITE_KINGSIDE;
        break;
      case 'Q':
        board->castle_rights |= WHITE_QUEENSIDE;
        break;
      case 'k':
        board->castle_rights |= BLACK_KINGSIDE;
        break;
      case 'q':
        board->castle_rights |= BLACK_QUEENSIDE;
        break;
      }
      current++;
    }
  } else {
    current++; // Passer le '-'
  }

  if (!*current || *current != ' ')
    return;  // FEN incomplet
  current++; // Passer l'espace

  // Parser en passant (simple pour l'instant)
  if (*current == '-') {
    board->en_passant = -1;
  } else {
    // Conversion notation algebrique -> Square (ex: "e3" -> E3)
    if (*current >= 'a' && *current <= 'h' && *(current + 1) >= '1' &&
        *(current + 1) <= '8') {
      int file = *current - 'a';
      int rank = *(current + 1) - '1';
      board->en_passant = rank * 8 + file;
    } else {
      board->en_passant = -1;
    }
  }

  // Valeurs par défaut pour les compteurs
  board->halfmove_clock = 0;
  board->move_number = 1;
}