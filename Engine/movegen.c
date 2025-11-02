#include "movegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialise une liste de coups vide
void movelist_init(MoveList *list) { list->count = 0; }

void movelist_add(MoveList *list, Move move) {
  if (list->count < 256) {
    list->moves[list->count] = move;
    list->count++;
  }
  // Si la liste est pleine (>= 256), le coup est silencieusement ignoré
  // car cela indique une position anormale
}

Move create_move(Square from, Square to, MoveType type) {
  Move move;
  move.from = from;
  move.to = to;
  move.type = type;
  move.promotion = EMPTY;
  move.captured_piece = EMPTY;
  return move;
}

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
// Convertit un coup en string (thread-safe avec buffer static)
char *move_to_string(const Move *move) {
  static char buffer[16];
  char from_str[3] = {'a' + (move->from % 8), '1' + (move->from / 8), '\0'};
  char to_str[3] = {'a' + (move->to % 8), '1' + (move->to / 8), '\0'};

  sprintf(buffer, "%s%s", from_str, to_str);

  switch (move->type) {
  case MOVE_PROMOTION: {
    const char pieces[] = "pnbrqk"; // UCI utilise minuscules
    sprintf(buffer + strlen(buffer), "%c",
            pieces[move->promotion]); // UCI sans '='
    break;
  }
  default:
    break;
  }

  return buffer;
}
// Génération complète de tous les coups légaux pour une position
void generate_moves(const Board *board, MoveList *moves) {
  // Vérifications de sécurité
  if (!moves || !board)
    return;

  movelist_init(moves);

  // Générer les coups de toutes les pièces selon le joueur actuel
  Couleur color = board->to_move;

#ifdef DEBUG
  fprintf(stderr, "[DEBUG] generate_moves for %s\n",
          color == WHITE ? "WHITE" : "BLACK");
  int start_count = moves->count;
#endif

  generate_pawn_moves(board, color, moves);
#ifdef DEBUG
  fprintf(stderr, "[DEBUG] After pawn_moves: %d moves (added %d)\n",
          moves->count, moves->count - start_count);
  start_count = moves->count;
#endif

  generate_rook_moves(board, color, moves);
#ifdef DEBUG
  fprintf(stderr, "[DEBUG] After rook_moves: %d moves (added %d)\n",
          moves->count, moves->count - start_count);
  start_count = moves->count;
#endif

  generate_bishop_moves(board, color, moves);
#ifdef DEBUG
  fprintf(stderr, "[DEBUG] After bishop_moves: %d moves (added %d)\n",
          moves->count, moves->count - start_count);
  start_count = moves->count;
#endif

  generate_knight_moves(board, color, moves);
#ifdef DEBUG
  fprintf(stderr, "[DEBUG] After knight_moves: %d moves (added %d)\n",
          moves->count, moves->count - start_count);
  start_count = moves->count;
#endif

  generate_queen_moves(board, color, moves);
#ifdef DEBUG
  fprintf(stderr, "[DEBUG] After queen_moves: %d moves (added %d)\n",
          moves->count, moves->count - start_count);
  start_count = moves->count;
#endif

  generate_king_moves(board, color, moves);
#ifdef DEBUG
  fprintf(stderr, "[DEBUG] After king_moves: %d moves (added %d)\n",
          moves->count, moves->count - start_count);
#endif
}

static void generate_pawn_pushes(const Board *board, Couleur color, Square from,
                                 int direction, int start_rank,
                                 MoveList *moves) {
  Square one_forward = from + direction;

  if (one_forward >= A1 && one_forward <= H8 &&
      !is_square_occupied(board, one_forward)) {
    int rank = one_forward / 8;
    if ((color == WHITE && rank == 7) || (color == BLACK && rank == 0)) {
      ADD_PROMOTIONS(from, one_forward, EMPTY, moves);
    } else {
      Move m = create_move(from, one_forward, MOVE_NORMAL);
      movelist_add(moves, m);
    }

    Square two_forward = from + (2 * direction);
    if ((from / 8) == start_rank && two_forward >= A1 && two_forward <= H8 &&
        !is_square_occupied(board, two_forward)) {
      Move m = create_move(from, two_forward, MOVE_NORMAL);
      movelist_add(moves, m);
    }
  }
}

// Helper: ajoute une capture de pion (avec promotion si sur dernière rangée)
static void add_pawn_capture(const Board *board, Square from, Square to,
                             Couleur color, MoveList *moves) {
  if (to < A1 || to > H8)
    return;
  if (!is_square_occupied(board, to))
    return;
  if (get_piece_color(board, to) == color)
    return;

  PieceType captured = get_piece_type(board, to);
  int rank = to / 8;

  // Promotion si dernière rangée
  if ((color == WHITE && rank == 7) || (color == BLACK && rank == 0)) {
    ADD_PROMOTIONS(from, to, captured, moves);
  } else {
    Move capture = create_move(from, to, MOVE_CAPTURE);
    capture.captured_piece = captured;
    movelist_add(moves, capture);
  }
}

static void generate_pawn_captures(const Board *board, Couleur color,
                                   Square from, int direction,
                                   MoveList *moves) {
  int left_capture = (color == WHITE) ? from + 7 : from - 9;
  int right_capture = (color == WHITE) ? from + 9 : from - 7;

  // Vérifications de wrap-around avant d'ajouter les captures
  if ((from % 8) != 0) {
    add_pawn_capture(board, from, left_capture, color, moves);
  }
  if ((from % 8) != 7) {
    add_pawn_capture(board, from, right_capture, color, moves);
  }
}

static void generate_en_passant(const Board *board, Couleur color,
                                MoveList *moves) {
  if (board->en_passant == -1)
    return;

  int ep_rank_check = board->en_passant / 8;
#ifdef DEBUG
  fprintf(stderr, "[DEBUG EP] en_passant square=%d (rank=%d), color=%s\n",
          board->en_passant, ep_rank_check, color == WHITE ? "WHITE" : "BLACK");
#endif
  if ((color == WHITE && ep_rank_check != 5) ||
      (color == BLACK && ep_rank_check != 2)) {
#ifdef DEBUG
    fprintf(stderr, "[DEBUG EP] Wrong rank for en-passant, skipping\n");
#endif
    return;
  }

  Square ep_square = board->en_passant;
  int ep_file = ep_square % 8;

  Square target_pawn_square = (color == WHITE) ? ep_square - 8 : ep_square + 8;
  Square left_attacker = target_pawn_square - 1;
  Square right_attacker = target_pawn_square + 1;

  if (ep_file > 0 && left_attacker >= A1 && left_attacker <= H8) {
    if (is_square_occupied(board, left_attacker) &&
        get_piece_color(board, left_attacker) == color &&
        get_piece_type(board, left_attacker) == PAWN) {
      Move ep_move = create_move(left_attacker, ep_square, MOVE_EN_PASSANT);
      ep_move.captured_piece = PAWN;
#ifdef DEBUG
      fprintf(stderr, "[DEBUG EP] Adding left en-passant move from %d to %d\n",
              left_attacker, ep_square);
#endif
      movelist_add(moves, ep_move);
    }
  }

  if (ep_file < 7 && right_attacker >= A1 && right_attacker <= H8) {
    if (is_square_occupied(board, right_attacker) &&
        get_piece_color(board, right_attacker) == color &&
        get_piece_type(board, right_attacker) == PAWN) {
      Move ep_move = create_move(right_attacker, ep_square, MOVE_EN_PASSANT);
      ep_move.captured_piece = PAWN;
#ifdef DEBUG
      fprintf(stderr, "[DEBUG EP] Adding right en-passant move from %d to %d\n",
              right_attacker, ep_square);
#endif
      movelist_add(moves, ep_move);
    }
  }
}

void generate_pawn_moves(const Board *board, Couleur color, MoveList *moves) {
  if (!moves)
    return;

  Bitboard pawns = board->pieces[color][PAWN];
  int direction = (color == WHITE) ? 8 : -8;
  int start_rank = (color == WHITE) ? 1 : 6;

#ifdef DEBUG
  fprintf(stderr, "[DEBUG PAWN] Generating for %s, bitboard=0x%llx\n",
          color == WHITE ? "WHITE" : "BLACK", pawns);
#endif

  while (pawns) {
    Square from = __builtin_ctzll(pawns);
    pawns &= pawns - 1;

    if (from < A1 || from > H8)
      continue;

    generate_pawn_pushes(board, color, from, direction, start_rank, moves);
    generate_pawn_captures(board, color, from, direction, moves);
  }

  generate_en_passant(board, color, moves);
}

// Calcule la distance minimale entre deux limites
static inline int min(int a, int b) { return (a < b) ? a : b; }

// Calcule le nombre maximum de pas pour une direction donnée
static int calculate_max_steps(int rank, int file, int offset) {
  switch (offset) {
  case +8:
    return 7 - rank; // Nord
  case -8:
    return rank; // Sud
  case +1:
    return 7 - file; // Est
  case -1:
    return file; // Ouest
  case +9:
    return min(7 - rank, 7 - file); // Nord-Est
  case +7:
    return min(7 - rank, file); // Nord-Ouest
  case -7:
    return min(rank, 7 - file); // Sud-Est
  case -9:
    return min(rank, file); // Sud-Ouest
  default:
    return 0; // Direction invalide
  }
}

// Tente d'ajouter un mouvement de capture
static bool try_add_capture(const Board *board, Square from, Square to,
                            Couleur color, MoveList *moves) {
  if (get_piece_color(board, to) != color) {
    Move capture = create_move(from, to, MOVE_CAPTURE);
    capture.captured_piece = get_piece_type(board, to);
    movelist_add(moves, capture);
    return true;
  }
  return false;
}

// Ajoute un mouvement normal (case vide)
static void add_normal_move(Square from, Square to, MoveList *moves) {
  Move m = create_move(from, to, MOVE_NORMAL);
  movelist_add(moves, m);
}

// Fonction principale simplifiée
void slide_direction(const Board *board, Square from, int offset, Couleur color,
                     MoveList *moves) {
  int rank = from / 8;
  int file = from % 8;
  int max_steps = calculate_max_steps(rank, file, offset);

  if (max_steps == 0)
    return; // Direction invalide

  for (int step = 1; step <= max_steps; step++) {
    Square to = from + step * offset;

    // Vérification de sécurité (redondante avec calculate_max_steps mais
    // garde-fou)
    if (to < A1 || to > H8)
      break;

    if (is_square_occupied(board, to)) {
      try_add_capture(board, from, to, color, moves);
      break; // Obstacle rencontré
    }

    add_normal_move(from, to, moves);
  }
}

// Génération des mouvements de tour
void generate_rook_moves(const Board *board, Couleur color, MoveList *moves) {
  // Vérifications de sécurité

  if (!moves || !board)
    return;

  // Récupérer le bitboard des tours de cette couleur
  Bitboard rooks = board->pieces[color][ROOK];

  // Parcourir toutes les tours avec la méthode bitboard
  while (rooks != 0) {
    Square from = __builtin_ctzll(rooks); // Trouve la première tour
    rooks &= (rooks - 1);                 // Efface ce bit

    // Vérifier la validité de la case
    if (from < A1 || from > H8) {
      continue;
    }

    // Générer les mouvements dans les 4 directions orthogonales
    slide_direction(board, from, +8, color, moves); // Nord
    slide_direction(board, from, -8, color, moves); // Sud
    slide_direction(board, from, +1, color, moves); // Est
    slide_direction(board, from, -1, color, moves); // Ouest
  }
}

// Génération des mouvements de fou
void generate_bishop_moves(const Board *board, Couleur color, MoveList *moves) {

  // Vérifications de sécurité
  if (!moves || !board)
    return;

  // Récupérer le bitboard des fous de cette couleur
  Bitboard bishops = board->pieces[color][BISHOP];

  // Parcourir tous les fous avec la méthode bitboard
  while (bishops != 0) {
    Square from = __builtin_ctzll(bishops); // Trouve le premier fou
    bishops &= (bishops - 1);               // Efface ce bit

    // Vérifier la validité de la case
    if (from < A1 || from > H8) {
      continue;
    }

    // Générer les mouvements dans les 4 directions diagonales
    slide_direction(board, from, +9, color, moves); // Nord-Est
    slide_direction(board, from, +7, color, moves); // Nord-Ouest
    slide_direction(board, from, -7, color, moves); // Sud-Est
    slide_direction(board, from, -9, color, moves); // Sud-Ouest
  }
}

// Génération des mouvements de cavalier
void generate_knight_moves(const Board *board, Couleur color, MoveList *moves) {

  // Vérifications de sécurité
  if (!moves || !board)
    return;

  // Offsets des 8 mouvements possibles du cavalier (en forme de L)
  static const int knight_offsets[8] = {
      +17, +15, +10, +6, // Mouvements vers le haut
      -6,  -10, -15, -17 // Mouvements vers le bas
  };

  // Récupérer le bitboard des cavaliers de cette couleur
  Bitboard knights = board->pieces[color][KNIGHT];

#ifdef DEBUG
  fprintf(stderr, "[DEBUG KNIGHT] Generating for %s, bitboard=0x%llx\n",
          color == WHITE ? "WHITE" : "BLACK", knights);
#endif

  // Parcourir tous les cavaliers avec la méthode bitboard
  while (knights != 0) {
    Square from = __builtin_ctzll(knights); // Trouve le premier cavalier
    knights &= (knights - 1);               // Efface ce bit

    // Vérifier la validité de la case
    if (from < A1 || from > H8) {
      continue;
    }

    int from_file = from % 8;
    int from_rank = from / 8;

    // Tester les 8 mouvements possibles
    for (int i = 0; i < 8; i++) {
      Square to = from + knight_offsets[i];

      // Vérification basique : case dans les limites du plateau
      if (to < A1 || to > H8) {
        continue;
      }

      int to_file = to % 8;
      int to_rank = to / 8;

      // Vérification smart anti-wrap : déplacement colonne/rangée cohérent
      int file_diff = abs(to_file - from_file);
      int rank_diff = abs(to_rank - from_rank);

      // Cavalier : soit (1,2) soit (2,1) en déplacement colonne/rangée
      if (!((file_diff == 1 && rank_diff == 2) ||
            (file_diff == 2 && rank_diff == 1))) {
        continue; // Mouvement invalide (wrap-around détecté)
      }

      // Case vide → mouvement normal
      if (!is_square_occupied(board, to)) {
        Move m = create_move(from, to, MOVE_NORMAL);
        movelist_add(moves, m);
      }
      // Case occupée par pièce ennemie → capture
      else if (get_piece_color(board, to) != color) {
        Move capture = create_move(from, to, MOVE_CAPTURE);
        capture.captured_piece = get_piece_type(board, to);
        movelist_add(moves, capture);
      }
      // Case occupée par pièce amie → ignorer
    }
  }
}

// Génération des mouvements de dame (combinaison tour + fou)
void generate_queen_moves(const Board *board, Couleur color, MoveList *moves) {

  // Vérifications de sécurité
  if (!moves || !board)
    return;

  // Récupérer le bitboard des dames de cette couleur
  Bitboard queens = board->pieces[color][QUEEN];

  // Parcourir toutes les dames avec la méthode bitboard
  while (queens != 0) {
    Square from = __builtin_ctzll(queens); // Trouve la première dame
    queens &= (queens - 1);                // Efface ce bit

    // Vérifier la validité de la case
    if (from < A1 || from > H8) {
      continue;
    }

    // Générer les mouvements dans les 8 directions (tour + fou)
    // Directions orthogonales (tour)
    slide_direction(board, from, +8, color, moves); // Nord
    slide_direction(board, from, -8, color, moves); // Sud
    slide_direction(board, from, +1, color, moves); // Est
    slide_direction(board, from, -1, color, moves); // Ouest

    // Directions diagonales (fou)
    slide_direction(board, from, +9, color, moves); // Nord-Est
    slide_direction(board, from, +7, color, moves); // Nord-Ouest
    slide_direction(board, from, -7, color, moves); // Sud-Est
    slide_direction(board, from, -9, color, moves); // Sud-Ouest
  }
}

// Vérifie si un déplacement du roi qui ressemble à un roque est
// illégal (non autorisé par les droits de roque ou cases attaquées)
int is_castle_illegal(const Board *board, const Move *m) {
  if (!board || !m)
    return 0;
  // On ne s'intéresse qu'aux déplacements du roi de 2 cases latérales
  int from = m->from;
  int to = m->to;
  Couleur color = get_piece_color(board, from);
  int from_rank = from / 8;
  int from_file = from % 8;
  int to_rank = to / 8;
  int to_file = to % 8;
  // Pour les blancs : e1 (4) vers g1 (6) ou c1 (2)
  // Pour les noirs : e8 (60) vers g8 (62) ou c8 (58)
  if (get_piece_type(board, from) != KING)
    return 0;
  // Seuls les coups de type normal/capture sont concernés ici (pas MOVE_CASTLE)
  if (abs(to_file - from_file) == 2 && from_rank == to_rank) {
    // Ressemble à un roque
    // Vérifier si le droit de roque est présent
    if (color == WHITE && from == E1) {
      if (to == G1) {
        // Petit roque blanc
        if (!(board->castle_rights & WHITE_KINGSIDE))
          return 1;
        // Cases entre e1 et h1 doivent être libres
        for (Square sq = F1; sq < H1; sq++) {
          if (is_square_occupied(board, sq))
            return 1;
        }
        // Cases e1, f1, g1 ne doivent pas être attaquées
        Couleur opp = BLACK;
        if (is_in_check(board, color) || is_square_attacked(board, F1, opp) ||
            is_square_attacked(board, G1, opp))
          return 1;
      } else if (to == C1) {
        // Grand roque blanc
        if (!(board->castle_rights & WHITE_QUEENSIDE))
          return 1;
        for (Square sq = B1 + 1; sq < E1; sq++) {
          if (is_square_occupied(board, sq))
            return 1;
        }
        Couleur opp = BLACK;
        if (is_in_check(board, color) || is_square_attacked(board, D1, opp) ||
            is_square_attacked(board, C1, opp))
          return 1;
      } else {
        // Mouvement latéral de 2 cases non autorisé
        return 1;
      }
    } else if (color == BLACK && from == E8) {
      if (to == G8) {
        if (!(board->castle_rights & BLACK_KINGSIDE))
          return 1;
        for (Square sq = F8; sq < H8; sq++) {
          if (is_square_occupied(board, sq))
            return 1;
        }
        Couleur opp = WHITE;
        if (is_in_check(board, color) || is_square_attacked(board, F8, opp) ||
            is_square_attacked(board, G8, opp))
          return 1;
      } else if (to == C8) {
        if (!(board->castle_rights & BLACK_QUEENSIDE))
          return 1;
        for (Square sq = B8 + 1; sq < E8; sq++) {
          if (is_square_occupied(board, sq))
            return 1;
        }
        Couleur opp = WHITE;
        if (is_in_check(board, color) || is_square_attacked(board, D8, opp) ||
            is_square_attacked(board, C8, opp))
          return 1;
      } else {
        return 1;
      }
    } else {
      // Mouvement latéral de 2 cases ailleurs que depuis e1/e8
      return 1;
    }
  }
  // Si le roi tente un déplacement diagonal de 2 cases, interdit aussi
  if (abs(to_file - from_file) == 2 && abs(to_rank - from_rank) == 2) {
    return 1;
  }
  return 0;
}

// Helper: tente d'ajouter un roque (kingside si is_kingside=1, queenside sinon)
static void try_add_castle(const Board *board, Couleur color, int is_kingside,
                           MoveList *moves) {
  // Vérifier les droits de roque
  int has_rights =
      is_kingside ? (color == WHITE ? (board->castle_rights & WHITE_KINGSIDE)
                                    : (board->castle_rights & BLACK_KINGSIDE))
                  : (color == WHITE ? (board->castle_rights & WHITE_QUEENSIDE)
                                    : (board->castle_rights & BLACK_QUEENSIDE));

#ifdef DEBUG
  fprintf(stderr, "[DEBUG CASTLE] Try %s %s castle, rights=%d\n",
          color == WHITE ? "WHITE" : "BLACK",
          is_kingside ? "kingside" : "queenside", has_rights);
#endif

  if (!has_rights)
    return;

  Square king_pos = __builtin_ctzll(board->pieces[color][KING]);
  Square rook_pos, king_dest, passage_square;

  if (is_kingside) {
    rook_pos = (color == WHITE) ? H1 : H8;
    king_dest = (color == WHITE) ? G1 : G8;
    passage_square = king_pos + 1; // f1/f8
  } else {
    rook_pos = (color == WHITE) ? A1 : A8;
    king_dest = (color == WHITE) ? C1 : C8;
    passage_square = king_pos - 1; // d1/d8
  }

  // Vérifier que les cases entre roi et tour sont vides
  Square start = (is_kingside) ? king_pos + 1 : rook_pos + 1;
  Square end = (is_kingside) ? rook_pos : king_pos;

  for (Square sq = start; sq < end; sq++) {
    if (is_square_occupied(board, sq)) {
      return; // Chemin bloqué
    }
  }

  // Vérifier que les cases de passage du roi ne sont pas attaquées
  Couleur opponent = (color == WHITE) ? BLACK : WHITE;
  if (!is_square_attacked(board, passage_square, opponent) &&
      !is_square_attacked(board, king_dest, opponent)) {
    Move castle_move = create_move(king_pos, king_dest, MOVE_CASTLE);
    if (is_move_legal(board, &castle_move)) {
#ifdef DEBUG
      fprintf(stderr, "[DEBUG CASTLE] Adding %s %s castle move from %d to %d\n",
              color == WHITE ? "WHITE" : "BLACK",
              is_kingside ? "kingside" : "queenside", king_pos, king_dest);
#endif
      movelist_add(moves, castle_move);
    } else {
#ifdef DEBUG
      fprintf(
          stderr,
          "[DEBUG CASTLE] Castle move is illegal (king in check after move)\n");
#endif
    }
  } else {
#ifdef DEBUG
    fprintf(stderr,
            "[DEBUG CASTLE] Castle blocked: passage_square=%d attacked=%d, "
            "king_dest=%d attacked=%d\n",
            passage_square, is_square_attacked(board, passage_square, opponent),
            king_dest, is_square_attacked(board, king_dest, opponent));
#endif
  }
}

// Génération des mouvements de roi (mouvements de base uniquement)
void generate_king_moves(const Board *board, Couleur color, MoveList *moves) {

  // Vérifications de sécurité
  if (!moves || !board)
    return;

  // Offsets des 8 mouvements possibles du roi (1 case dans chaque direction)
  static const int king_offsets[8] = {
      +8, // Nord
      -8, // Sud
      +1, // Est
      -1, // Ouest
      +9, // Nord-Est
      +7, // Nord-Ouest
      -7, // Sud-Est
      -9  // Sud-Ouest
  };

  // Récupérer le bitboard du roi de cette couleur
  Bitboard kings = board->pieces[color][KING];

  // Parcourir le roi (normalement un seul)
  while (kings != 0) {
    Square from = __builtin_ctzll(kings); // Trouve le roi
    kings &= (kings - 1);                 // Efface ce bit

    // Vérifier la validité de la case
    if (from < A1 || from > H8) {
      continue;
    }

    int from_file = from % 8;
    int from_rank = from / 8;

    // Tester les 8 mouvements possibles
    for (int i = 0; i < 8; i++) {
      Square to = from + king_offsets[i];
      // Vérification basique : case dans les limites du plateau
      if (to < A1 || to > H8) {
        continue;
      }
      int to_file = to % 8;
      int to_rank = to / 8;
      // Vérification anti-wrap : déplacement max 1 case en colonne/rangée
      int file_diff = abs(to_file - from_file);
      int rank_diff = abs(to_rank - from_rank);
      if (file_diff > 1 || rank_diff > 1) {
        continue; // Mouvement invalide (wrap-around détecté)
      }
      // Coups pseudo-légaux du roi
      if (!is_square_occupied(board, to)) {
        Move m = create_move(from, to, MOVE_NORMAL);
        movelist_add(moves, m);
      } else if (get_piece_color(board, to) != color) {
        Move m = create_move(from, to, MOVE_CAPTURE);
        m.captured_piece = get_piece_type(board, to);
        movelist_add(moves, m);
      }
      // Case occupée par pièce amie → ignorer
    }
  }

  // Ajouter les roques si conditions remplies
  if (!is_in_check(board, color)) { // Le roi ne doit pas être en échec
    // Petit roque (kingside)
    try_add_castle(board, color, 1, moves);
    // Grand roque (queenside)
    try_add_castle(board, color, 0, moves);
  }
}

// Vérifie si une case est attaquée par la couleur adverse
int is_square_attacked(const Board *board, Square square,
                       Couleur attacking_color) {
  if (!board || square < A1 || square > H8) {
    return 0;
  }

  int target_rank = square / 8;
  int target_file = square % 8;

  // Vérifier attaques de pions
  int pawn_direction = (attacking_color == WHITE) ? 1 : -1;
  int pawn_rank = target_rank - pawn_direction;

  if (pawn_rank >= 0 && pawn_rank <= 7) {
    // Pion attaque depuis la gauche
    if (target_file > 0) {
      Square left_pawn = pawn_rank * 8 + (target_file - 1);
      if (is_square_occupied(board, left_pawn) &&
          get_piece_color(board, left_pawn) == attacking_color &&
          get_piece_type(board, left_pawn) == PAWN) {
        return 1;
      }
    }
    // Pion attaque depuis la droite
    if (target_file < 7) {
      Square right_pawn = pawn_rank * 8 + (target_file + 1);
      if (is_square_occupied(board, right_pawn) &&
          get_piece_color(board, right_pawn) == attacking_color &&
          get_piece_type(board, right_pawn) == PAWN) {
        return 1;
      }
    }
  }

  // Vérifier attaques de cavalier
  static const int knight_moves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                                         {1, -2},  {1, 2},  {2, -1},  {2, 1}};

  for (int i = 0; i < 8; i++) {
    int rank = target_rank + knight_moves[i][0];
    int file = target_file + knight_moves[i][1];

    if (rank >= 0 && rank <= 7 && file >= 0 && file <= 7) {
      Square from = rank * 8 + file;
      if (is_square_occupied(board, from) &&
          get_piece_color(board, from) == attacking_color &&
          get_piece_type(board, from) == KNIGHT) {
        return 1;
      }
    }
  }

  // Vérifier attaques orthogonales (tour/dame)
  static const int rook_dirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  for (int dir = 0; dir < 4; dir++) {
    for (int dist = 1; dist < 8; dist++) {
      int rank = target_rank + dist * rook_dirs[dir][0];
      int file = target_file + dist * rook_dirs[dir][1];

      if (rank < 0 || rank > 7 || file < 0 || file > 7)
        break;

      Square from = rank * 8 + file;
      if (is_square_occupied(board, from)) {
        if (get_piece_color(board, from) == attacking_color) {
          PieceType piece = get_piece_type(board, from);
          if (piece == ROOK || piece == QUEEN) {
            return 1;
          }
        }
        break; // Pièce bloque
      }
    }
  }

  // Vérifier attaques diagonales (fou/dame)
  static const int bishop_dirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
  for (int dir = 0; dir < 4; dir++) {
    for (int dist = 1; dist < 8; dist++) {
      int rank = target_rank + dist * bishop_dirs[dir][0];
      int file = target_file + dist * bishop_dirs[dir][1];

      if (rank < 0 || rank > 7 || file < 0 || file > 7)
        break;

      Square from = rank * 8 + file;
      if (is_square_occupied(board, from)) {
        if (get_piece_color(board, from) == attacking_color) {
          PieceType piece = get_piece_type(board, from);
          if (piece == BISHOP || piece == QUEEN) {
            return 1;
          }
        }
        break; // Pièce bloque
      }
    }
  }

  // Vérifier attaques de roi
  for (int rank_offset = -1; rank_offset <= 1; rank_offset++) {
    for (int file_offset = -1; file_offset <= 1; file_offset++) {
      if (rank_offset == 0 && file_offset == 0)
        continue;

      int rank = target_rank + rank_offset;
      int file = target_file + file_offset;

      if (rank >= 0 && rank <= 7 && file >= 0 && file <= 7) {
        Square from = rank * 8 + file;
        if (is_square_occupied(board, from) &&
            get_piece_color(board, from) == attacking_color &&
            get_piece_type(board, from) == KING) {
          return 1;
        }
      }
    }
  }

  return 0;
}

// Vérifie si le roi de la couleur donnée est en échec
int is_in_check(const Board *board, Couleur color) {
  if (!board)
    return 0;

  // Trouver le roi
  Bitboard kings = board->pieces[color][KING];
  if (kings == 0)
    return 0; // Pas de roi (situation anormale)

  Square king_square = __builtin_ctzll(kings);

  // Vérifier si le roi est attaqué par la couleur adverse
  Couleur opponent = (color == WHITE) ? BLACK : WHITE;
  return is_square_attacked(board, king_square, opponent);
}

// Effectue temporairement un mouvement pour tester sa légalité
void make_move_temp(Board *board, const Move *move, Board *backup) {

  // Sauvegarder l'état actuel
  *backup = *board;

  // Effacer la pièce de la case de départ
  PieceType piece_type = get_piece_type(board, move->from);
  Couleur piece_color = get_piece_color(board, move->from);

  board->pieces[piece_color][piece_type] &= ~(1ULL << move->from);
  board->occupied[piece_color] &= ~(1ULL << move->from);

  // Mettre à jour les droits de roque
  if (piece_type == KING) {
    if (piece_color == WHITE) {
      board->castle_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
    } else {
      board->castle_rights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
    }
  } else if (piece_type == ROOK) {
    if (piece_color == WHITE) {
      if (move->from == H1)
        board->castle_rights &= ~WHITE_KINGSIDE;
      if (move->from == A1)
        board->castle_rights &= ~WHITE_QUEENSIDE;
    } else {
      if (move->from == H8)
        board->castle_rights &= ~BLACK_KINGSIDE;
      if (move->from == A8)
        board->castle_rights &= ~BLACK_QUEENSIDE;
    }
  }

  // Gérer la capture (y compris les promotions avec capture)
  PieceType captured_piece_type = EMPTY;
  Square captured_square = move->to;
  if (move->type == MOVE_CAPTURE || move->type == MOVE_EN_PASSANT ||
      (move->type == MOVE_PROMOTION && move->captured_piece != EMPTY)) {
    Couleur opponent = (piece_color == WHITE) ? BLACK : WHITE;
    if (move->type == MOVE_EN_PASSANT) {
      captured_square = (piece_color == WHITE) ? move->to - 8 : move->to + 8;
      captured_piece_type = PAWN;
    } else {
      captured_piece_type = move->captured_piece;
    }

    if (captured_piece_type != EMPTY) {
      board->pieces[opponent][captured_piece_type] &=
          ~(1ULL << captured_square);
      board->occupied[opponent] &= ~(1ULL << captured_square);

      // Si une tour est capturée sur sa case initiale, annuler le droit de
      // roque IMPORTANT: Ce bloc doit être à l'intérieur du if
      // (captured_piece_type != EMPTY)
      if (captured_piece_type == ROOK) {
        if (captured_square == H1)
          board->castle_rights &= ~WHITE_KINGSIDE;
        if (captured_square == A1)
          board->castle_rights &= ~WHITE_QUEENSIDE;
        if (captured_square == H8)
          board->castle_rights &= ~BLACK_KINGSIDE;
        if (captured_square == A8)
          board->castle_rights &= ~BLACK_QUEENSIDE;
      }
    }
  }

  // Placer la pièce sur la case d'arrivée
  if (move->type == MOVE_PROMOTION) {
    board->pieces[piece_color][move->promotion] |= (1ULL << move->to);
  } else {
    board->pieces[piece_color][piece_type] |= (1ULL << move->to);
  }
  board->occupied[piece_color] |= (1ULL << move->to);

  // Gérer le roque
  if (move->type == MOVE_CASTLE) {
    Square rook_from, rook_to;
    if (move->to > move->from) { // Petit roque
      rook_from = (piece_color == WHITE) ? H1 : H8;
      rook_to = (piece_color == WHITE) ? F1 : F8;
    } else { // Grand roque
      rook_from = (piece_color == WHITE) ? A1 : A8;
      rook_to = (piece_color == WHITE) ? D1 : D8;
    }

    // Déplacer la tour
    board->pieces[piece_color][ROOK] &= ~(1ULL << rook_from);
    board->pieces[piece_color][ROOK] |= (1ULL << rook_to);
    board->occupied[piece_color] &= ~(1ULL << rook_from);
    board->occupied[piece_color] |= (1ULL << rook_to);
  }

  // Recalculer all_pieces
  board->all_pieces = board->occupied[WHITE] | board->occupied[BLACK];

  // Réinitialiser en_passant par défaut
  board->en_passant = -1;
  // Si un pion avance de deux cases, définir la case en_passant
  if (piece_type == PAWN && abs((int)move->to - (int)move->from) == 16) {
    board->en_passant =
        (piece_color == WHITE) ? (move->from + 8) : (move->from - 8);
  }
  // Basculer le joueur actif
  board->to_move = (board->to_move == WHITE) ? BLACK : WHITE;
}

// Restaure l'état du board
void unmake_move_temp(Board *board, const Board *backup) { *board = *backup; }

// Vérifie si un mouvement est légal (ne met pas le roi en échec)
int is_move_legal(const Board *board, const Move *move) {

  // 1. Vérifier que la case de départ contient bien une pièce du joueur actif
  if (move->from < A1 || move->from > H8)
    return 0;
  if (!is_square_occupied(board, move->from)) {
    return 0;
  }
  Couleur moving_color = get_piece_color(board, move->from);
  if (moving_color != board->to_move) {
    return 0;
  }

  // 2. Vérifier que la destination est valide :
  // - soit vide
  // - soit occupée par une pièce adverse (jamais une pièce amie)
  if (move->to < A1 || move->to > H8)
    return 0;
  if (is_square_occupied(board, move->to)) {
    Couleur dest_color = get_piece_color(board, move->to);
    if (dest_color == moving_color) {
      return 0;
    }
  }

  Board temp_board, backup;
  temp_board = *board;

  make_move_temp(&temp_board, move, &backup);

  // Vérifier si le roi est en échec après le mouvement
  int legal = !is_in_check(&temp_board, moving_color);

  return legal;
}

// Filtre les mouvements illégaux d'une liste
void filter_legal_moves(const Board *board, MoveList *moves) {

  MoveList legal_moves;
  movelist_init(&legal_moves);

  int filtered_count = 0;
  for (int i = 0; i < moves->count; i++) {
    if (is_move_legal(board, &moves->moves[i])) {
      movelist_add(&legal_moves, moves->moves[i]);
    } else {
      filtered_count++;
    }
  }

#ifdef DEBUG
  fprintf(stderr, "[DEBUG FILTER] Filtered %d illegal moves out of %d\n",
          filtered_count, moves->count);
#endif

  *moves = legal_moves;
}

// Génération de mouvements légaux uniquement
void generate_legal_moves(const Board *board, MoveList *moves) {
  generate_moves(board, moves);
  filter_legal_moves(board, moves);
}

// Détecte si la position est pat (aucun mouvement légal, roi pas en échec)
int is_stalemate(const Board *board) {
  if (is_in_check(board, board->to_move)) {
    return 0; // Roi en échec = pas pat
  }

  MoveList moves;
  generate_legal_moves(board, &moves);
  return (moves.count == 0); // Aucun mouvement légal = pat
}

// Détecte si la position est mat (aucun mouvement légal, roi en échec)
int is_checkmate(const Board *board) {
  if (!is_in_check(board, board->to_move)) {
    return 0; // Roi pas en échec = pas mat
  }

  MoveList moves;
  generate_legal_moves(board, &moves);
  return (moves.count == 0); // Aucun mouvement légal + échec = mat
}

// Vérifie la règle des 50 coups (50 demi-coups sans pion bougé ni capture)
int is_fifty_move_rule(const Board *board) {
  return (board->halfmove_clock >= 100); // 100 demi-coups = 50 coups complets
}

// Détecte si la partie est terminée

GameResult get_game_result(const Board *board) {
  // Vérifier règle des 50 coups
  if (is_fifty_move_rule(board)) {
    return GAME_FIFTY_MOVE_RULE;
  }

  // Vérifier mat
  if (is_checkmate(board)) {
    return (board->to_move == WHITE) ? GAME_CHECKMATE_WHITE
                                     : GAME_CHECKMATE_BLACK;
  }

  // Vérifier pat
  if (is_stalemate(board)) {
    return GAME_STALEMATE;
  }

  return GAME_ONGOING;
}
