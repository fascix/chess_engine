#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// ============================================================================
// UTILITAIRES D'AFFICHAGE ET CONVERSION
// ============================================================================

// Affiche un coup en notation lisible
void print_move(const Move *move) {
  char from_str[3] = {'a' + (move->from % 8), '1' + (move->from / 8), '\0'};
  char to_str[3] = {'a' + (move->to % 8), '1' + (move->to / 8), '\0'};

  printf("%s%s", from_str, to_str);

  switch (move->type) {
  case MOVE_PROMOTION: {
    const char pieces[] = "PNBRQK";
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

// Convertit un coup en string (thread-safe avec buffer static)
char *move_to_string(const Move *move) {
  static char buffer[16];
  char from_str[3] = {'a' + (move->from % 8), '1' + (move->from / 8), '\0'};
  char to_str[3] = {'a' + (move->to % 8), '1' + (move->to / 8), '\0'};

  sprintf(buffer, "%s%s", from_str, to_str);

  if (move->type == MOVE_PROMOTION) {
    const char pieces[] = "pnbrqk"; // UCI utilise minuscules
    sprintf(buffer + strlen(buffer), "%c", pieces[move->promotion]);
  }

  return buffer;
}

// Affichage debug du board
void print_board_debug(const Board *board) {
  const char piece_chars[] = ".PNBRQK";
  printf("\n  +---+---+---+---+---+---+---+---+\n");

  for (int rank = 7; rank >= 0; rank--) {
    printf("%d |", rank + 1);
    for (int file = 0; file < 8; file++) {
      Square sq = rank * 8 + file;
      PieceType piece = get_piece_type(board, sq);
      Couleur color = get_piece_color(board, sq);

      char c = piece_chars[piece + 1];
      if (piece != EMPTY && color == BLACK)
        c = c + 32; // Minuscule pour noir

      printf(" %c |", c);
    }
    printf("\n  +---+---+---+---+---+---+---+---+\n");
  }
  printf("    a   b   c   d   e   f   g   h\n\n");
  printf("To move: %s\n", board->to_move == WHITE ? "White" : "Black");
}

// ============================================================================
// UTILITAIRES D'ANALYSE DE PIÈCES
// ============================================================================

// Valeur des pièces en centipawns
int piece_value(PieceType piece) {
  static const int values[] = {100,    // PAWN
                               320,    // KNIGHT
                               330,    // BISHOP
                               500,    // ROOK
                               900,    // QUEEN
                               20000}; // KING

  if (piece >= PAWN && piece <= KING)
    return values[piece];
  return 0;
}

// Vérifie si une couleur a du matériel non-pion
int has_non_pawn_material(const Board *board, Couleur color) {
  for (PieceType p = KNIGHT; p <= QUEEN; p++) {
    if (board->pieces[color][p] != 0)
      return 1;
  }
  return 0;
}

// Compte le nombre de pièces d'un type et couleur
int count_pieces(const Board *board, PieceType piece, Couleur color) {
  Bitboard bb = board->pieces[color][piece];
  int count = 0;
  while (bb) {
    count++;
    bb &= bb - 1; // Clear le bit le moins significatif
  }
  return count;
}

// ============================================================================
// UTILITAIRES D'ANALYSE DE PIONS
// ============================================================================

// Ces fonctions sont implémentées dans evaluation.c car elles utilisent
// des helpers spécifiques à l'évaluation

// ============================================================================
// UTILITAIRES D'ANALYSE DE COUPS
// ============================================================================

// Vérifie si un coup donne échec
int gives_check(const Board *board, const Move *move) {
  Board temp;
  memcpy(&temp, board, sizeof(Board));

  Board backup;
  make_move_temp(&temp, move, &backup);

  Couleur opponent = (temp.to_move == WHITE) ? BLACK : WHITE;
  int in_check = is_in_check(&temp, opponent);

  return in_check;
}

// Vérifie si un coup se dirige vers le centre
int moves_toward_center(const Board *board, const Move *move) {
  (void)board; // Unused
  int from_dist = square_to_center_distance(move->from);
  int to_dist = square_to_center_distance(move->to);
  return to_dist < from_dist;
}

// Vérifie si un coup est évidemment mauvais
int is_obviously_bad_move(const Board *board, const Move *move) {
  // Un coup est mauvais s'il perd du matériel sans compensation
  if (move->type == MOVE_CAPTURE) {
    PieceType attacker = get_piece_type(board, move->from);
    PieceType victim = move->captured_piece;

    // Capturer avec une pièce de valeur supérieure est suspect
    if (piece_value(attacker) > piece_value(victim) + 100) {
      // Vérifier si la case de destination est attaquée
      Board temp;
      memcpy(&temp, board, sizeof(Board));
      Board backup;
      make_move_temp(&temp, move, &backup);

      Couleur opponent = (board->to_move == WHITE) ? BLACK : WHITE;
      if (is_square_attacked(&temp, move->to, opponent))
        return 1; // Case attaquée, coup probablement mauvais
    }
  }

  return 0;
}

// Vérifie si un coup est une capture
int is_capture(const Move *move) {
  return (move->type == MOVE_CAPTURE || move->type == MOVE_EN_PASSANT);
}

// Vérifie si un coup est une promotion
int is_promotion(const Move *move) { return (move->type == MOVE_PROMOTION); }

// ============================================================================
// UTILITAIRES DE PHASE DE JEU
// ============================================================================

// Ces fonctions sont implémentées dans evaluation.c car elles utilisent
// des helpers spécifiques à l'évaluation

// ============================================================================
// UTILITAIRES DE GÉOMÉTRIE ÉCHIQUIER
// ============================================================================

// Distance de Manhattan entre deux cases
int square_distance(Square sq1, Square sq2) {
  int file1 = file_of(sq1);
  int rank1 = rank_of(sq1);
  int file2 = file_of(sq2);
  int rank2 = rank_of(sq2);

  return abs(file1 - file2) + abs(rank1 - rank2);
}

// Distance d'une case au centre
int square_to_center_distance(Square sq) {
  int file = file_of(sq);
  int rank = rank_of(sq);

  int file_dist = abs(file - 3) < abs(file - 4) ? abs(file - 3) : abs(file - 4);
  int rank_dist = abs(rank - 3) < abs(rank - 4) ? abs(rank - 3) : abs(rank - 4);

  return file_dist + rank_dist;
}

// Vérifie si une case est au centre (e4, e5, d4, d5)
int is_center_square(Square sq) {
  return (sq == 27 || sq == 28 || sq == 35 || sq == 36); // d4, e4, d5, e5
}

// Vérifie si une case est dans le centre étendu
int is_extended_center_square(Square sq) {
  int file = file_of(sq);
  int rank = rank_of(sq);
  return (file >= 2 && file <= 5 && rank >= 2 && rank <= 5);
}

// Vérifie si deux cases sont sur la même colonne
int same_file(Square sq1, Square sq2) { return file_of(sq1) == file_of(sq2); }

// Vérifie si deux cases sont sur la même rangée
int same_rank(Square sq1, Square sq2) { return rank_of(sq1) == rank_of(sq2); }

// Vérifie si deux cases sont sur la même diagonale
int same_diagonal(Square sq1, Square sq2) {
  int file_diff = abs(file_of(sq1) - file_of(sq2));
  int rank_diff = abs(rank_of(sq1) - rank_of(sq2));
  return file_diff == rank_diff;
}

// Obtient la colonne d'une case (0-7)
int file_of(Square sq) { return sq % 8; }

// Obtient la rangée d'une case (0-7)
int rank_of(Square sq) { return sq / 8; }

// ============================================================================
// UTILITAIRES DE TEMPS ET PERFORMANCE
// ============================================================================

// Obtient le temps actuel en millisecondes
long get_time_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// Démarre un timer
void timer_start(Timer *timer) { timer->start_time_ms = get_time_ms(); }

// Arrête un timer
void timer_stop(Timer *timer) { timer->end_time_ms = get_time_ms(); }

// Obtient le temps écoulé
long timer_elapsed_ms(const Timer *timer) {
  return timer->end_time_ms - timer->start_time_ms;
}
