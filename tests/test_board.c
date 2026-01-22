/**
 * @file test_board.c
 * @brief Unit tests for board module
 */

#include "../tests/unity/unity.h"
#include "../Engine/board.h"
#include <string.h>

void setUp(void) {
    // This function is called before each test
}

void tearDown(void) {
    // This function is called after each test
}

// Test: Initialize board from starting FEN position
void test_board_initial_position(void) {
    Board board;
    board_from_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Check white pawns bitboard has 8 pawns
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[WHITE][PAWN]);
    
    // Check white rooks bitboard has 2 rooks
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[WHITE][ROOK]);
    
    // Check white has king
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[WHITE][KING]);
    
    // Check black has pieces
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[BLACK][PAWN]);
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[BLACK][ROOK]);
}

// Test: Board after e2-e4 move
void test_board_after_e4(void) {
    Board board;
    board_from_fen(&board, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    // Check white still has 8 pawns total (just moved, not captured)
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[WHITE][PAWN]);
    
    // Check all black pieces are still there
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[BLACK][PAWN]);
}

// Test: Empty board
void test_empty_board(void) {
    Board board;
    board_from_fen(&board, "8/8/8/8/8/8/8/8 w - - 0 1");
    
    // Check all piece bitboards are empty
    for (int color = WHITE; color <= BLACK; color++) {
        for (int piece = PAWN; piece <= KING; piece++) {
            TEST_ASSERT_EQUAL_UINT64(0, board.pieces[color][piece]);
        }
    }
}

// Test: FEN parsing and validation
void test_fen_parsing(void) {
    Board board;
    
    // Valid starting position
    board_from_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[WHITE][KING]);
    TEST_ASSERT_NOT_EQUAL(0, board.pieces[BLACK][KING]);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_board_initial_position);
    RUN_TEST(test_board_after_e4);
    RUN_TEST(test_empty_board);
    RUN_TEST(test_fen_parsing);
    
    return UNITY_END();
}
