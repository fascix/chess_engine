CC = clang
CFLAGS = -Wall -Wextra -std=c99 -O2
SRC_DIR = src
TARGET = chess_engine
TEST_TARGET = test_compile

# Fichiers sources
SRCS = $(SRC_DIR)/board.c $(SRC_DIR)/main.c
TEST_SRCS = $(SRC_DIR)/board.c $(SRC_DIR)/test_compile.c

# Règle par défaut
all: $(TARGET)

# Compilation du projet principal
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Test de compilation
test: $(TEST_SRCS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRCS)
	./$(TEST_TARGET)

# Nettoyage
clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all test clean
