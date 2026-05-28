#include "polyglot.h"
#include "logger.h"
#include <arpa/inet.h> // For ntohs, ntohl
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Polyglot Zobrist constants
static uint64_t polyglot_random_piece[12][64];
static uint64_t polyglot_random_castle[16];
static uint64_t polyglot_random_en_passant[8];
static uint64_t polyglot_random_side;

// Polyglot PRNG: LCG with seed 0
static uint64_t pg_lcg_seed = 0;
static uint64_t pg_next_random(void) {
  pg_lcg_seed = pg_lcg_seed * 6364136223846793005ULL + 1ULL;
  return pg_lcg_seed;
}

void polyglot_init(void) {
  pg_lcg_seed = 0;

  // 1. Piece randoms
  // Pieces: White Pawn (0), Knight (1), Bishop (2), Rook (3), Queen (4), King
  // (5)
  //        Black Pawn (6), Knight (7), Bishop (8), Rook (9), Queen (10), King
  //        (11)
  for (int p = 0; p < 12; p++) {
    for (int s = 0; s < 64; s++) {
      polyglot_random_piece[p][s] = pg_next_random();
    }
  }

  // 2. Castle randoms
  // Bits: WK (0), WQ (1), BK (2), BQ (3)
  // Total 16 combinations (0-15)
  // Wait, Polyglot spec says: "The castle randoms are for the 4 bits."
  // Actually, they are usually handled by XORing the bits' corresponding
  // randoms. But we can pre-calculate the 16 combinations. The randoms are: WK,
  // WQ, BK, BQ.
  uint64_t wk = pg_next_random();
  uint64_t wq = pg_next_random();
  uint64_t bk = pg_next_random();
  uint64_t bq = pg_next_random();

  for (int i = 0; i < 16; i++) {
    uint64_t h = 0;
    if (i & 1)
      h ^= wk;
    if (i & 2)
      h ^= wq;
    if (i & 4)
      h ^= bk;
    if (i & 8)
      h ^= bq;
    polyglot_random_castle[i] = h;
  }

  // 3. En passant randoms (8 files)
  for (int f = 0; f < 8; f++) {
    polyglot_random_en_passant[f] = pg_next_random();
  }

  // 4. Side random
  polyglot_random_side = pg_next_random();
}

uint64_t polyglot_hash(const Board *board) {
  uint64_t hash = 0;

  // 1. Pieces
  for (int color = WHITE; color <= BLACK; color++) {
    for (int type = PAWN; type <= KING; type++) {
      Bitboard pieces = board->pieces[color][type];
      int pg_piece = type + (color == BLACK ? 6 : 0);
      while (pieces) {
        Square sq = __builtin_ctzll(pieces);
        pieces &= pieces - 1;
        // Polyglot square mapping: A1=0, H1=7, A8=56, H8=63
        // My engine mapping is the same.
        hash ^= polyglot_random_piece[pg_piece][sq];
      }
    }
  }

  // 2. Castle
  hash ^= polyglot_random_castle[board->castle_rights];

  // 3. En passant
  // Only XOR if a capture is possible.
  // For simplicity, we check if there are pawns that can capture.
  if (board->en_passant != -1) {
    int file = board->en_passant % 8;
    int rank = board->en_passant / 8;
    Couleur side = board->to_move;
    Couleur opponent = (side == WHITE) ? BLACK : WHITE;

    // The en passant pawn is on the rank before the en passant square
    Square ep_pawn_sq =
        (side == WHITE) ? board->en_passant - 8 : board->en_passant + 8;

    // Check if any neighboring pawn can capture
    bool can_capture = false;
    if (file > 0) {
      Square left = ep_pawn_sq - 1;
      if (GET_BIT(board->pieces[side][PAWN], left))
        can_capture = true;
    }
    if (file < 7) {
      Square right = ep_pawn_sq + 1;
      if (GET_BIT(board->pieces[side][PAWN], right))
        can_capture = true;
    }

    if (can_capture) {
      hash ^= polyglot_random_en_passant[file];
    }
  }

  // 4. Side
  if (board->to_move == WHITE) {
    hash ^= polyglot_random_side;
  }

  return hash;
}

Move polyglot_get_move(const Board *board, const char *book_path) {
  if (!book_path)
    return (Move){.from = -1};

  FILE *f = fopen(book_path, "rb");
  if (!f)
    return (Move){.from = -1};

  uint64_t target_key = polyglot_hash(board);

  // Binary search in the book
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  long num_entries = size / 16;

  long low = 0, high = num_entries - 1;
  long found_idx = -1;

  while (low <= high) {
    long mid = low + (high - low) / 2;
    fseek(f, mid * 16, SEEK_SET);

    uint64_t key;
    if (fread(&key, 8, 1, f) != 1)
      break;

    // Polyglot is big-endian
    key = ((uint64_t)ntohl(key & 0xFFFFFFFF) << 32) | ntohl(key >> 32);
    // Wait, ntohl is for 32-bit. For 64-bit we need more.
    // Let's use a better way to read 64-bit big endian.
    fseek(f, mid * 16, SEEK_SET);
    unsigned char buf[16];
    if (fread(buf, 1, 16, f) != 16)
      break;

    uint64_t entry_key = 0;
    for (int i = 0; i < 8; i++)
      entry_key = (entry_key << 8) | buf[i];

    if (entry_key == target_key) {
      found_idx = mid;
      break;
    } else if (entry_key < target_key) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  if (found_idx == -1) {
    fclose(f);
    return (Move){.from = -1};
  }

  // Find all entries with the same key
  // Go to the first entry
  long start_idx = found_idx;
  while (start_idx > 0) {
    fseek(f, (start_idx - 1) * 16, SEEK_SET);
    unsigned char buf[8];
    if (fread(buf, 1, 8, f) != 8)
      break;
    uint64_t entry_key = 0;
    for (int i = 0; i < 8; i++)
      entry_key = (entry_key << 8) | buf[i];
    if (entry_key != target_key)
      break;
    start_idx--;
  }

  // Collect all moves and weights
  typedef struct {
    uint16_t move;
    uint16_t weight;
  } BookMove;
  BookMove moves[128];
  int count = 0;
  uint32_t total_weight = 0;

  fseek(f, start_idx * 16, SEEK_SET);
  while (count < 128) {
    unsigned char buf[16];
    if (fread(buf, 1, 16, f) != 16)
      break;
    uint64_t entry_key = 0;
    for (int i = 0; i < 8; i++)
      entry_key = (entry_key << 8) | buf[i];
    if (entry_key != target_key)
      break;

    uint16_t move_bits = ((uint16_t)buf[8] << 8) | buf[9];
    uint16_t weight = ((uint16_t)buf[10] << 8) | buf[11];

    moves[count].move = move_bits;
    moves[count].weight = weight;
    total_weight += weight;
    count++;
  }
  fclose(f);

  if (count == 0)
    return (Move){.from = -1};

  // Weighted random selection
  if (total_weight == 0) {
    // Pick one randomly
    int r = rand() % count;
    return parse_polyglot_move(moves[r].move, board);
  }

  uint32_t r = rand() % total_weight;
  uint32_t current_weight = 0;
  for (int i = 0; i < count; i++) {
    current_weight += moves[i].weight;
    if (r < current_weight) {
      return parse_polyglot_move(moves[i].move, board);
    }
  }

  return parse_polyglot_move(moves[count - 1].move, board);
}

Move parse_polyglot_move(uint16_t move_bits, const Board *board) {
  int to_file = move_bits & 7;
  int to_rank = (move_bits >> 3) & 7;
  int from_file = (move_bits >> 6) & 7;
  int from_rank = (move_bits >> 9) & 7;
  int promo = (move_bits >> 12) & 7;

  Move move;
  move.from = from_rank * 8 + from_file;
  move.to = to_rank * 8 + to_file;
  move.type = MOVE_NORMAL;
  move.promotion = EMPTY;

  // Check for special moves
  PieceType piece = get_piece_type(board, move.from);
  PieceType captured = get_piece_type(board, move.to);

  if (captured != EMPTY) {
    move.type = MOVE_CAPTURE;
    move.captured_piece = captured;
  }

  if (piece == PAWN) {
    if (move.to == board->en_passant) {
      move.type = MOVE_EN_PASSANT;
      move.captured_piece = PAWN;
    } else if (abs(to_rank - from_rank) == 1 && from_file != to_file &&
               captured == EMPTY) {
      // Must be en passant if it's a diagonal move to empty square
      move.type = MOVE_EN_PASSANT;
      move.captured_piece = PAWN;
    }

    if (promo > 0) {
      move.type = MOVE_PROMOTION;
      static const PieceType promo_map[] = {EMPTY, KNIGHT, BISHOP, ROOK, QUEEN};
      move.promotion = promo_map[promo];
    }
  } else if (piece == KING) {
    if (abs(to_file - from_file) == 2) {
      move.type = MOVE_CASTLE;
    }
  }

  return move;
}
