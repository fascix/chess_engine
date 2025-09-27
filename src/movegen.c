#define ADD_PROMOTIONS(from, to, captured, moves)                              \
  do {                                                                         \
    Move promoQ = create_promotion_move((from), (to), QUEEN);                  \
    promoQ.captured_piece = (captured);                                        \
    movelist_add((moves), promoQ);                                             \
    Move promoR = create_promotion_move((from), (to), ROOK);                   \
    promoR.captured_piece = (captured);                                        \
    movelist_add((moves), promoR);                                             \
    Move promoB = create_promotion_move((from), (to), BISHOP);                 \
    promoB.captured_piece = (captured);                                        \
    movelist_add((moves), promoB);                                             \
    Move promoN = create_promotion_move((from), (to), KNIGHT);                 \
    promoN.captured_piece = (captured);                                        \
    movelist_add((moves), promoN);                                             \
  } while (0)
#include "movegen.h"
#include <stdio.h>
#include <stdlib.h>
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
  printf("Coups générés (%d) :\\n", list->count);
  for (int i = 0; i < list->count; i++) {
    printf("%2d. ", i + 1);
    print_move(&list->moves[i]);
    printf("\\n");
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

  generate_pawn_moves(board, color, moves);
  generate_rook_moves(board, color, moves);
  generate_bishop_moves(board, color, moves);
  generate_knight_moves(board, color, moves);
  generate_queen_moves(board, color, moves);
  generate_king_moves(board, color, moves);
}

void generate_pawn_moves(const Board *board, Couleur color, MoveList *moves) {
  // Initialiser la liste des mouvements (éviter les coups corrompus)
  if (!moves)
    return;

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
          // Ajoute toutes les promotions (pas de capture ici)
          ADD_PROMOTIONS(from, one_forward, EMPTY, moves);
        } else {
          // Coup normal
          Move m = create_move(from, one_forward, MOVE_NORMAL);
          movelist_add(moves, m);
        }

        // 2. DOUBLE SAUT - Si première rangée et case deux cases devant libre
        Square two_forward = from + (2 * direction);
        if ((from / 8) == start_rank && two_forward >= A1 &&
            two_forward <= H8 && !is_square_occupied(board, two_forward)) {
          Move m = create_move(from, two_forward, MOVE_NORMAL);
          movelist_add(moves, m);
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
            // Promotions avec capture
            ADD_PROMOTIONS(from, left_capture, captured, moves);
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
            // Promotions avec capture
            ADD_PROMOTIONS(from, right_capture, captured, moves);
          } else {
            Move capture = create_move(from, right_capture, MOVE_CAPTURE);
            capture.captured_piece = captured;
            movelist_add(moves, capture);
          }
        }
      }
    }
  }

  // 4. EN PASSANT
  if (board->en_passant != -1) {
    // Vérification : la case en_passant doit être sur le rang 5 (pour les
    // blancs) ou 2 (pour les noirs)
    int ep_rank_check = board->en_passant / 8;
    if ((color == WHITE && ep_rank_check != 5) ||
        (color == BLACK && ep_rank_check != 2)) {
      return; // Pas de vrai en passant possible
    }
    // Vérifier si des pions peuvent capturer en passant
    Square ep_square = board->en_passant;
    int ep_file = ep_square % 8;
    int ep_rank = ep_square / 8;

    // La prise se fait sur la case en_passant, le pion adverse est sur la
    // rangée du pion qui bouge
    Square target_pawn_square;
    if (color == WHITE) {
      target_pawn_square =
          ep_square - 8; // Pion noir sur rangée 5, capture sur rangée 6
    } else {
      target_pawn_square =
          ep_square + 8; // Pion blanc sur rangée 4, capture sur rangée 3
    }

    // Chercher les pions qui peuvent capturer (à gauche et à droite du pion
    // cible)
    Square left_attacker = target_pawn_square - 1;
    Square right_attacker = target_pawn_square + 1;

    // Attaquant depuis la gauche
    if (ep_file > 0 && left_attacker >= A1 && left_attacker <= H8) {
      if (is_square_occupied(board, left_attacker) &&
          get_piece_color(board, left_attacker) == color &&
          get_piece_type(board, left_attacker) == PAWN) {
        Move ep_move = create_move(left_attacker, ep_square, MOVE_EN_PASSANT);
        ep_move.captured_piece = PAWN;
        movelist_add(moves, ep_move);
      }
    }

    // Attaquant depuis la droite
    if (ep_file < 7 && right_attacker >= A1 && right_attacker <= H8) {
      if (is_square_occupied(board, right_attacker) &&
          get_piece_color(board, right_attacker) == color &&
          get_piece_type(board, right_attacker) == PAWN) {
        Move ep_move = create_move(right_attacker, ep_square, MOVE_EN_PASSANT);
        ep_move.captured_piece = PAWN;
        movelist_add(moves, ep_move);
      }
    }
  }
}

// Fonction générique pour les mouvements de sliding pieces (tours, fous, dame)
void slide_direction(const Board *board, Square from, int offset, Couleur color,
                     MoveList *moves) {
  // Pré-calculer le nombre maximum de pas dans cette direction
  int max_steps;
  int rank = from / 8;
  int file = from % 8;

  switch (offset) {
  case +8:
    max_steps = (7 - rank);
    break; // Nord: cases jusqu'au bord
  case -8:
    max_steps = rank;
    break; // Sud: cases jusqu'au bord
  case +1:
    max_steps = (7 - file);
    break; // Est: cases jusqu'au bord
  case -1:
    max_steps = file;
    break; // Ouest: cases jusqu'au bord
  // Directions diagonales
  case +9:
    max_steps = (7 - rank < 7 - file) ? (7 - rank) : (7 - file);
    break; // Nord-Est: limité par bord haut OU droit
  case +7:
    max_steps = (7 - rank < file) ? (7 - rank) : file;
    break; // Nord-Ouest: limité par bord haut OU gauche
  case -7:
    max_steps = (rank < 7 - file) ? rank : (7 - file);
    break; // Sud-Est: limité par bord bas OU droit
  case -9:
    max_steps = (rank < file) ? rank : file;
    break; // Sud-Ouest: limité par bord bas OU gauche
  default:
    return; // Direction invalide
  }

  // Nouvelle boucle : vérifier toutes les cases intermédiaires libres avant
  // d'ajouter un coup
  for (int step = 1; step <= max_steps; step++) {
    Square to = from + step * offset;
    // Vérification de sécurité (ne devrait pas arriver avec le pré-calcul)
    if (to < A1 || to > H8)
      break;
    // Vérifier que toutes les cases intermédiaires sont libres
    if (is_square_occupied(board, to)) {
      if (get_piece_color(board, to) != color) {
        Move capture = create_move(from, to, MOVE_CAPTURE);
        capture.captured_piece = get_piece_type(board, to);
        movelist_add(moves, capture);
      }
      break; // Obstacle rencontré
    } else {
      Move m = create_move(from, to, MOVE_NORMAL);
      movelist_add(moves, m);
    }
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

    // PETIT ROQUE (roi vers g1/g8, tour vers f1/f8)
    if ((color == WHITE && (board->castle_rights & WHITE_KINGSIDE)) ||
        (color == BLACK && (board->castle_rights & BLACK_KINGSIDE))) {

      Square king_pos = __builtin_ctzll(board->pieces[color][KING]);
      Square rook_pos = (color == WHITE) ? H1 : H8;
      Square king_dest = (color == WHITE) ? G1 : G8;
      Square rook_dest = (color == WHITE) ? F1 : F8;

      // Vérifier que les cases entre roi et tour sont vides
      int squares_clear = 1;
      for (Square sq = king_pos + 1; sq < rook_pos; sq++) {
        if (is_square_occupied(board, sq)) {
          squares_clear = 0;
          break;
        }
      }

      // Vérifier que les cases de passage du roi ne sont pas attaquées
      Couleur opponent = (color == WHITE) ? BLACK : WHITE;
      if (squares_clear &&
          !is_square_attacked(board, king_pos + 1, opponent) && // case f1/f8
          !is_square_attacked(board, king_dest, opponent)) {    // case g1/g8
        Move castle_move = create_move(king_pos, king_dest, MOVE_CASTLE);
        if (is_move_legal(board, &castle_move)) {
          movelist_add(moves, castle_move);
        }
      }
    }

    // GRAND ROQUE (roi vers c1/c8, tour vers d1/d8)
    if ((color == WHITE && (board->castle_rights & WHITE_QUEENSIDE)) ||
        (color == BLACK && (board->castle_rights & BLACK_QUEENSIDE))) {

      Square king_pos = __builtin_ctzll(board->pieces[color][KING]);
      Square rook_pos = (color == WHITE) ? A1 : A8;
      Square king_dest = (color == WHITE) ? C1 : C8;
      Square rook_dest = (color == WHITE) ? D1 : D8;

      // Vérifier que les cases entre roi et tour sont vides
      int squares_clear = 1;
      for (Square sq = rook_pos + 1; sq < king_pos; sq++) {
        if (is_square_occupied(board, sq)) {
          squares_clear = 0;
          break;
        }
      }

      // Vérifier que les cases de passage du roi ne sont pas attaquées
      Couleur opponent = (color == WHITE) ? BLACK : WHITE;
      if (squares_clear &&
          !is_square_attacked(board, king_pos - 1, opponent) && // case d1/d8
          !is_square_attacked(board, king_dest, opponent)) {    // case c1/c8
        Move castle_move = create_move(king_pos, king_dest, MOVE_CASTLE);
        if (is_move_legal(board, &castle_move)) {
          movelist_add(moves, castle_move);
        }
      }
    }
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

  // Gérer la capture
  if (move->type == MOVE_CAPTURE || move->type == MOVE_EN_PASSANT) {
    if (move->type == MOVE_EN_PASSANT) {
      // En passant : retirer le pion capturé
      Square captured_square =
          (piece_color == WHITE) ? move->to - 8 : move->to + 8;
      Couleur opponent = (piece_color == WHITE) ? BLACK : WHITE;
      board->pieces[opponent][PAWN] &= ~(1ULL << captured_square);
      board->occupied[opponent] &= ~(1ULL << captured_square);
    } else {
      // Capture normale : retirer la pièce capturée
      Couleur opponent = (piece_color == WHITE) ? BLACK : WHITE;
      board->pieces[opponent][move->captured_piece] &= ~(1ULL << move->to);
      board->occupied[opponent] &= ~(1ULL << move->to);
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
  if (piece_type == PAWN && (move->to - move->from) == 16) {
    board->en_passant =
        (piece_color == WHITE) ? (move->from + 8) : (move->from - 8);
  }
  // Changer le joueur actif
  board->to_move = (piece_color == WHITE) ? BLACK : WHITE;
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

  for (int i = 0; i < moves->count; i++) {
    if (is_move_legal(board, &moves->moves[i])) {
      movelist_add(&legal_moves, moves->moves[i]);
    }
  }

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