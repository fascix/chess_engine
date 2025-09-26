CC = clang
CFLAGS = -Wall -Wextra -std=c99 -O2
SRC_DIR = src
TARGET = chess_engine

# Fichiers sources du moteur
CORE_SRCS = $(SRC_DIR)/board.c $(SRC_DIR)/movegen.c $(SRC_DIR)/evaluation.c $(SRC_DIR)/search.c $(SRC_DIR)/utils.c
MAIN_SRCS = $(CORE_SRCS) $(SRC_DIR)/main.c $(SRC_DIR)/uci.c

# Fichiers de test
TEST_BOARD_SRCS = $(SRC_DIR)/board.c $(SRC_DIR)/test_compile.c
TEST_MOVEGEN_SRCS = $(CORE_SRCS) $(SRC_DIR)/test_movegen.c
TEST_SEARCH_SRCS = $(CORE_SRCS) $(SRC_DIR)/test_search.c

# Règle all : clean + compile tout
all: clean $(TARGET) chess_uci test_binaries
	@echo "✅ Compilation complète terminée !"

# Compilation du moteur principal avec UCI
$(TARGET): $(MAIN_SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN_SRCS)

# Compilation de chess_uci (même chose que le target principal)
chess_uci: $(MAIN_SRCS)
	$(CC) $(CFLAGS) -o chess_uci $(MAIN_SRCS)

# Compiler tous les binaires de test sans les exécuter
test_binaries: test_board_bin test_movegen_bin test_search_bin

test_board_bin: $(TEST_BOARD_SRCS)
	$(CC) $(CFLAGS) -o test_board $(TEST_BOARD_SRCS)

test_movegen_bin: $(TEST_MOVEGEN_SRCS)
	$(CC) $(CFLAGS) -o test_movegen $(TEST_MOVEGEN_SRCS)

test_search_bin: $(TEST_SEARCH_SRCS)
	$(CC) $(CFLAGS) -o test_search $(TEST_SEARCH_SRCS)

# Tests individuels avec exécution
test_board: test_board_bin
	./test_board

test_movegen: test_movegen_bin
	./test_movegen

test_search: test_search_bin
	./test_search

# Exécuter tous les tests
test_all: test_binaries
	@echo "🧪 Exécution de tous les tests..."
	./test_board
	./test_movegen
	./test_search
	@echo "✅ Tous les tests passés avec succès !"

# Test de compilation simple (legacy)
test: test_board

# Nettoyage
clean:
	@echo "🧹 Nettoyage des fichiers..."
	rm -f $(TARGET) chess_uci test_board test_movegen test_search
	rm -rf *.dSYM

.PHONY: all chess_uci test_binaries test_board_bin test_movegen_bin test_search_bin test_board test_movegen test_search test_all test clean
