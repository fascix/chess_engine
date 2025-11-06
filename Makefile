CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3 -march=native
LDFLAGS = -lm

SRC_DIR = src
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
TARGET = chess_engine

all: $(TARGET)

$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

debug: CFLAGS = -Wall -Wextra -std=c11 -g -DDEBUG -fsanitize=address,undefined
debug: TARGET = chess_engine_debug
debug: $(TARGET)

clean:
	rm -f $(TARGET) chess_engine_debug

.PHONY: all debug clean
