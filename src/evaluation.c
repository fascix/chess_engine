#include "evaluation.h"
#include <limits.h>
#include <stdlib.h>

// Tables de position pour encourager le développement et la centralisation
// Piece-Square Tables
static const int pawn_position_table[64] = {
    0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
    10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
    0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
    5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0};

static const int knight_position_table[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};

static const int bishop_position_table[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};

static const int rook_position_table[64] = {
    0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  0,  5,  5,  0,  0,  0};

static const int queen_position_table[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};

static const int king_position_table[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};

// Retourne la valeur d'une pièce
int piece_value(PieceType piece) {
  switch (piece) {
  case PAWN:
    return PAWN_VALUE;
  case KNIGHT:
    return KNIGHT_VALUE;
  case BISHOP:
    return BISHOP_VALUE;
  case ROOK:
    return ROOK_VALUE;
  case QUEEN:
    return QUEEN_VALUE;
  case KING:
    return KING_VALUE;
  default:
    return 0;
  }
}

// Évalue le matériel (différence de valeur des pièces)
int evaluate_material(const Board *board) {
  int material = 0;

  for (PieceType piece = PAWN; piece <= KING; piece++) {
    // __builtin_popcountll permet de compter le nombre de pièce :
    // pour les blanc
    int white_count = __builtin_popcountll(board->pieces[WHITE][piece]);
    // pour les noirs
    int black_count = __builtin_popcountll(board->pieces[BLACK][piece]);

    material += (white_count - black_count) * piece_value(piece);
  }

  return material;
}

// Évalue les bonus de position
int evaluate_position_bonus(const Board *board) {
  int bonus = 0;

  // Pour chaque type de pièce, ajouter bonus de position
  const int *position_tables[] = {pawn_position_table,   knight_position_table,
                                  bishop_position_table, rook_position_table,
                                  queen_position_table,  king_position_table};

  for (PieceType piece = PAWN; piece <= KING; piece++) {
    const int *table = position_tables[piece];

    // Parcours toutes les pièces du bitboard
    // __builtin_ctzll() retourne l'indice du premier bit à 1 (trailing zeros)
    // white_pieces &= white_pieces - 1 supprime ce bit pour passer au suivant

    // Pièces blanches
    Bitboard white_pieces = board->pieces[WHITE][piece];
    while (white_pieces) {
      Square square = __builtin_ctzll(white_pieces);
      white_pieces &= white_pieces - 1;
      bonus += table[square];
    }

    // Pièces noires (table inversée)
    Bitboard black_pieces = board->pieces[BLACK][piece];
    while (black_pieces) {
      Square square = __builtin_ctzll(black_pieces);
      black_pieces &= black_pieces - 1;
      bonus -= table[63 - square]; // Inverser pour les noirs
    }
  }

  return bonus;
}

// Vérifie si on est en fin de partie (peu de matériel)
int is_endgame(const Board *board) {
  int total_material = 0;

  for (PieceType piece = KNIGHT; piece <= QUEEN; piece++) {
    total_material +=
        __builtin_popcountll(board->pieces[WHITE][piece]) * piece_value(piece);
    total_material +=
        __builtin_popcountll(board->pieces[BLACK][piece]) * piece_value(piece);
  }

  return total_material < 2000; // Moins de 20 pions de matériel = endgame
}

// ========== ÉVALUATION AVANCÉE ==========

// Détermine la phase de jeu basée sur le matériel restant
GamePhase get_game_phase(const Board *board) {
  int total_material = 0;

  for (PieceType piece = KNIGHT; piece <= QUEEN; piece++) {
    total_material +=
        __builtin_popcountll(board->pieces[WHITE][piece]) * piece_value(piece);
    total_material +=
        __builtin_popcountll(board->pieces[BLACK][piece]) * piece_value(piece);
  }

  if (total_material > 3000)
    return OPENING_PHASE;
  if (total_material > 1500)
    return MIDDLEGAME_PHASE;
  return ENDGAME_PHASE;
}

// Facteur de phase pour interpolation (0.0 = endgame, 1.0 = opening)
float get_phase_factor(const Board *board) {
  int total_material = 0;

  for (PieceType piece = KNIGHT; piece <= QUEEN; piece++) {
    total_material +=
        __builtin_popcountll(board->pieces[WHITE][piece]) * piece_value(piece);
    total_material +=
        __builtin_popcountll(board->pieces[BLACK][piece]) * piece_value(piece);
  }

  const int MAX_MATERIAL =
      6200; // Matériel total en début de partie (hors pions/rois)
  float factor = (float)total_material / MAX_MATERIAL;
  if (factor > 1.0f)
    factor = 1.0f;
  if (factor < 0.0f)
    factor = 0.0f;

  return factor;
}

// Vérifie si un pion est passé (aucun pion adverse ne peut l'arrêter)
int is_pawn_passed(const Board *board, Square pawn_square, Couleur color) {
  int file = pawn_square % 8;
  int rank = pawn_square / 8;

  Couleur opponent = (color == WHITE) ? BLACK : WHITE;
  int direction = (color == WHITE) ? 1 : -1;

  // Vérifier les colonnes adjacentes et la colonne du pion
  for (int check_file = file - 1; check_file <= file + 1; check_file++) {
    if (check_file < 0 || check_file > 7)
      continue;

    // Vérifier toutes les cases devant le pion
    for (int check_rank = rank + direction; check_rank >= 0 && check_rank <= 7;
         check_rank += direction) {

      Square check_square = check_rank * 8 + check_file;
      if (is_square_occupied(board, check_square) &&
          get_piece_color(board, check_square) == opponent &&
          get_piece_type(board, check_square) == PAWN) {
        return 0; // Pion adverse trouvé
      }
    }
  }

  return 1; // Aucun pion adverse ne peut l'arrêter
}

// Vérifie si un pion est isolé (pas de pions alliés sur colonnes adjacentes)
int is_pawn_isolated(const Board *board, Square pawn_square, Couleur color) {
  int file = pawn_square % 8;

  // Vérifier colonnes adjacentes
  for (int check_file = file - 1; check_file <= file + 1; check_file += 2) {
    if (check_file < 0 || check_file > 7)
      continue;

    // Vérifier toute la colonne
    for (int check_rank = 0; check_rank <= 7; check_rank++) {
      Square check_square = check_rank * 8 + check_file;
      if (is_square_occupied(board, check_square) &&
          get_piece_color(board, check_square) == color &&
          get_piece_type(board, check_square) == PAWN) {
        return 0; // Pion allié trouvé sur colonne adjacente
      }
    }
  }

  return 1; // Aucun pion allié sur colonnes adjacentes
}

// Vérifie si un pion est doublé (autre pion de même couleur sur même colonne)
int is_pawn_doubled(const Board *board, Square pawn_square, Couleur color) {
  int file = pawn_square % 8;
  int rank = pawn_square / 8;

  // Vérifier toute la colonne sauf la case du pion lui-même
  for (int check_rank = 0; check_rank <= 7; check_rank++) {
    if (check_rank == rank)
      continue;

    Square check_square = check_rank * 8 + file;
    if (is_square_occupied(board, check_square) &&
        get_piece_color(board, check_square) == color &&
        get_piece_type(board, check_square) == PAWN) {
      return 1; // Autre pion trouvé sur même colonne
    }
  }

  return 0;
}

// Évalue la structure de pions
int evaluate_pawn_structure(const Board *board) {
  int score = 0;

  for (Couleur color = WHITE; color <= BLACK; color++) {
    int color_multiplier = (color == WHITE) ? 1 : -1;

    Bitboard pawns = board->pieces[color][PAWN];
    while (pawns) {
      Square pawn_square = __builtin_ctzll(pawns);
      pawns &= pawns - 1;

      // Bonus pour pions passés
      if (is_pawn_passed(board, pawn_square, color)) {
        score += PASSED_PAWN_BONUS * color_multiplier;
      }

      // Pénalité pour pions isolés
      if (is_pawn_isolated(board, pawn_square, color)) {
        score += ISOLATED_PAWN_PENALTY * color_multiplier;
      }

      // Pénalité pour pions doublés
      if (is_pawn_doubled(board, pawn_square, color)) {
        score += DOUBLED_PAWN_PENALTY * color_multiplier;
      }

      // Bonus pour pions centraux
      int file = pawn_square % 8;
      if (file >= 3 && file <= 4) { // colonnes e et d
        score += CENTER_PAWN_BONUS * color_multiplier;
      }
    }
  }

  return score;
}

// Évalue la sécurité du roi
int evaluate_king_safety(const Board *board) {
  int score = 0;

  for (Couleur color = WHITE; color <= BLACK; color++) {
    int color_multiplier = (color == WHITE) ? 1 : -1;

    Bitboard kings = board->pieces[color][KING];
    if (kings == 0)
      continue;

    Square king_square = __builtin_ctzll(kings);
    int king_file = king_square % 8;
    int king_rank = king_square / 8;

    // Bonus si le roi a roqué (approximation : roi sur g1/c1 ou g8/c8)
    if ((color == WHITE && king_rank == 0 &&
         (king_file == 2 || king_file == 6)) ||
        (color == BLACK && king_rank == 7 &&
         (king_file == 2 || king_file == 6))) {
      score += CASTLED_KING_BONUS * color_multiplier;
    }

    // Pénalité si le roi est sur une colonne ouverte (pas de pions)
    int file_has_pawn = 0;
    for (int check_rank = 0; check_rank <= 7; check_rank++) {
      Square check_square = check_rank * 8 + king_file;
      if (is_square_occupied(board, check_square) &&
          get_piece_type(board, check_square) == PAWN) {
        file_has_pawn = 1;
        break;
      }
    }

    if (!file_has_pawn) {
      score += KING_OPEN_FILE_PENALTY * color_multiplier;
    }
  }

  return score;
}

// Évalue le développement des pièces
int evaluate_piece_development(const Board *board) {
  int score = 0;

  // Bonus pour paire de fous
  for (Couleur color = WHITE; color <= BLACK; color++) {
    int color_multiplier = (color == WHITE) ? 1 : -1;

    int bishop_count = __builtin_popcountll(board->pieces[color][BISHOP]);
    if (bishop_count >= 2) {
      score += BISHOP_PAIR_BONUS * color_multiplier;
    }
  }

  return score;
}

// Évalue le contrôle du centre
int evaluate_center_control(const Board *board) {
  int score = 0;

  // Cases centrales importantes : e4, e5, d4, d5
  Square center_squares[] = {E4, E5, D4, D5};

  for (int i = 0; i < 4; i++) {
    Square square = center_squares[i];

    // Bonus si occupé par nos pièces
    if (is_square_occupied(board, square)) {
      Couleur occupant = get_piece_color(board, square);
      int multiplier = (occupant == WHITE) ? 1 : -1;
      score += 5 * multiplier;
    }

    // Bonus pour cases attaquées (approximation simple)
    if (is_square_attacked(board, square, WHITE)) {
      score += 2;
    }
    if (is_square_attacked(board, square, BLACK)) {
      score -= 2;
    }
  }

  return score;
}

// Évalue la mobilité (nombre de coups légaux)
int evaluate_mobility(const Board *board) {
  MoveList white_moves, black_moves;

  // Sauvegarder le joueur actuel
  Couleur original_to_move = board->to_move;

  // Générer mouvements pour les blancs
  ((Board *)board)->to_move = WHITE;
  generate_legal_moves(board, &white_moves);

  // Générer mouvements pour les noirs
  ((Board *)board)->to_move = BLACK;
  generate_legal_moves(board, &black_moves);

  // Restaurer le joueur actuel
  ((Board *)board)->to_move = original_to_move;

  return (white_moves.count - black_moves.count) * 2;
}

// Évaluation spécialisée pour l'ouverture
int evaluate_opening(const Board *board) {
  int score = evaluate_material(board);
  score += evaluate_position_bonus(board);
  score +=
      evaluate_piece_development(board) * 2;   // Développement plus important
  score += evaluate_center_control(board) * 2; // Centre plus important
  score += evaluate_king_safety(board);
  return score;
}

// Évaluation spécialisée pour la fin de partie
int evaluate_endgame(const Board *board) {
  int score = evaluate_material(board);
  score += evaluate_pawn_structure(board) * 2; // Pions plus importants
  score += evaluate_king_safety(board) / 2;    // Sécurité roi moins critique
  score += evaluate_mobility(board);
  return score;
}

// Évaluation avec interpolation selon la phase (Tapered Eval)
int evaluate_tapered(const Board *board, GamePhase phase, float phase_factor) {
  int opening_score = evaluate_opening(board);
  int endgame_score = evaluate_endgame(board);

  // Interpolation : phase_factor = 1.0 (opening) -> 0.0 (endgame)
  return (int)(opening_score * phase_factor +
               endgame_score * (1.0f - phase_factor));
}

// Vérifie si une pièce est "pendue" (attaquée et non défendue)
int is_piece_hanging(const Board *board, Square square, Couleur piece_color) {
  if (!is_square_occupied(board, square))
    return 0;

  Couleur opponent = (piece_color == WHITE) ? BLACK : WHITE;

  // La pièce est-elle attaquée ?
  if (!is_square_attacked(board, square, opponent)) {
    return 0; // Pas attaquée = pas pendue
  }

  // Est-elle défendue ?
  if (is_square_attacked(board, square, piece_color)) {
    return 0; // Défendue = pas vraiment pendue (échange possible)
  }

  return 1; // Attaquée et non défendue = PENDUE !
}

// Pénalité pour pièces pendues selon leur valeur
int evaluate_hanging_pieces(const Board *board) {
  int penalty = 0;

  for (Couleur color = WHITE; color <= BLACK; color++) {
    int color_multiplier =
        (color == WHITE) ? -1 : 1; // Pénalité pour nos pièces

    for (PieceType piece = PAWN; piece <= QUEEN; piece++) {
      Bitboard pieces = board->pieces[color][piece];

      while (pieces) {
        Square square = __builtin_ctzll(pieces);
        pieces &= pieces - 1;

        if (is_piece_hanging(board, square, color)) {
          // Pénalité = 90% de la valeur de la pièce
          int piece_val = piece_value(piece);
          penalty += (piece_val * 9 / 10) * color_multiplier;
        }
      }
    }
  }

  return penalty;
}

// Bonus pour développement sécurisé en ouverture
int evaluate_safe_development(const Board *board) {
  if (board->move_number > 10)
    return 0; // Après l'ouverture

  int bonus = 0;

  for (Couleur color = WHITE; color <= BLACK; color++) {
    int color_multiplier = (color == WHITE) ? 1 : -1;

    // Bonus pour cavaliers développés ET en sécurité
    Bitboard knights = board->pieces[color][KNIGHT];
    while (knights) {
      Square square = __builtin_ctzll(knights);
      knights &= knights - 1;

      // Cavaliers développés (pas sur première rangée)
      int rank = square / 8;
      int home_rank = (color == WHITE) ? 0 : 7;

      if (rank != home_rank) {
        bonus += 25 * color_multiplier; // Développé

        // Bonus supplémentaire s'il n'est pas pendu
        if (!is_piece_hanging(board, square, color)) {
          bonus += 15 * color_multiplier; // Sécurisé
        }
      }
    }

    // Même principe pour les fous
    Bitboard bishops = board->pieces[color][BISHOP];
    while (bishops) {
      Square square = __builtin_ctzll(bishops);
      bishops &= bishops - 1;

      int rank = square / 8;
      int home_rank = (color == WHITE) ? 0 : 7;

      if (rank != home_rank) {
        bonus += 20 * color_multiplier;
        if (!is_piece_hanging(board, square, color)) {
          bonus += 10 * color_multiplier;
        }
      }
    }
  }

  return bonus;
}

// Ajouter une pénalité pour mouvements de pions répétitifs
int evaluate_pawn_advancement_penalty(const Board *board) {
  if (board->move_number > 15)
    return 0; // Après l'ouverture

  int penalty = 0;
  int pawn_moves_count = 0; // Compteur approximatif

  // Pénaliser si trop de pions bougés sans développement
  for (Couleur color = WHITE; color <= BLACK; color++) {
    int color_multiplier = (color == WHITE) ? 1 : -1;
    Bitboard pawns = board->pieces[color][PAWN];

    // Approximation : pions qui ont bougé de leur position initiale
    Bitboard initial_pawns = (color == WHITE) ? 0xFF00ULL : 0xFF000000000000ULL;
    Bitboard moved_pawns = pawns & ~initial_pawns;
    int moved_count = __builtin_popcountll(moved_pawns);

    // Pénalité si > 3 pions bougés sans développement suffisant
    if (moved_count > 3) {
      int developed_pieces =
          __builtin_popcountll(
              board->pieces[color][KNIGHT] &
              ~((color == WHITE) ? 0x42ULL : 0x4200000000000000ULL)) +
          __builtin_popcountll(
              board->pieces[color][BISHOP] &
              ~((color == WHITE) ? 0x24ULL : 0x2400000000000000ULL));

      if (developed_pieces < 2) {
        penalty +=
            (moved_count - 3) * 30 * color_multiplier; // Pénalité progressive
      }
    }
  }

  return penalty;
}

// Fonction d'évaluation MISE À JOUR pour éviter les pendus
int evaluate_position(const Board *board) {
  GameResult result = get_game_result(board);
  switch (result) {
  case GAME_CHECKMATE_WHITE:
    return -MATE_SCORE;
  case GAME_CHECKMATE_BLACK:
    return MATE_SCORE;
  case GAME_STALEMATE:
  case GAME_FIFTY_MOVE_RULE:
    return STALEMATE_SCORE;
  case GAME_ONGOING:
    break;
  }

  int score = 0;

  // Évaluation de base
  score += evaluate_material(board);
  score += evaluate_position_bonus(board);

  // NOUVELLES ÉVALUATIONS TACTIQUES
  score += evaluate_hanging_pieces(board) * 2; // Très important !
  score += evaluate_safe_development(board);   // Pour l'ouverture

  // Autres évaluations selon la phase
  GamePhase phase = get_game_phase(board);
  if (phase == OPENING_PHASE) {
    score += evaluate_center_control(board) * 2;
    score += evaluate_king_safety(board);
  } else {
    score += evaluate_pawn_structure(board);
    score += evaluate_mobility(board);
  }

  // AJOUTER dans evaluate_position() :
  score += evaluate_pawn_advancement_penalty(board);
  return score;
}