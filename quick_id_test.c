#include "search.h"
#include <stdio.h>

int main() {
    init_zobrist();
    
    Board board;
    board_init(&board);
    
    printf("Test Iterative Deepening rapide:\n");
    SearchResult result = search_iterative_deepening(&board, 4, 2000);
    
    printf("\nMeilleur coup final: ");
    print_move(&result.best_move);
    printf(" (Score: %d)\n", result.score);
    
    return 0;
}
