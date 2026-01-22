/**
 * @file test_zobrist.c
 * @brief Unit tests for Zobrist hashing module
 */

#include "../Engine/board.h"
#include "../Engine/zobrist.h"
#include "../tests/unity/unity.h"
#include <string.h>

void setUp(void) { init_zobrist(); }

void tearDown(void) {
  // Nothing to clean up
}

// Test: Hash computation for initial position
void test_zobrist_initial_position(void) {
  Board board;
  board_from_fen(&board,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  uint64_t hash1 = zobrist_hash(&board);
  uint64_t hash2 = zobrist_hash(&board);

  // Same position should produce same hash
  TEST_ASSERT_EQUAL_UINT64(hash1, hash2);
}

// Test: Different positions have different hashes
void test_zobrist_different_positions(void) {
  Board board1, board2;

  board_from_fen(&board1,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  board_from_fen(&board2,
                 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

  uint64_t hash1 = zobrist_hash(&board1);
  uint64_t hash2 = zobrist_hash(&board2);

  // Different positions should produce different hashes
  TEST_ASSERT_NOT_EQUAL(hash1, hash2);
}

// Test: Hash is non-zero for starting position
void test_zobrist_non_zero(void) {
  Board board;
  board_from_fen(&board,
                 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  uint64_t hash = zobrist_hash(&board);

  // Hash should be non-zero
  TEST_ASSERT_NOT_EQUAL(0, hash);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_zobrist_initial_position);
  RUN_TEST(test_zobrist_different_positions);
  RUN_TEST(test_zobrist_non_zero);

  return UNITY_END();
}
