CC = gcc
CFLAGS_COMMON = -Wall -Wextra -std=c11
CFLAGS_DEBUG = -g -DDEBUG -fsanitize=address,undefined
CFLAGS_RELEASE = -O3 -march=native -flto -DNDEBUG

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

all: chess_engine

debug: CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
debug: chess_engine_debug

chess_engine: $(OBJ)
    $(CC) $(CFLAGS_RELEASE) -o $@ $^

chess_engine_debug: $(OBJ)
    $(CC) $(CFLAGS_DEBUG) -o $@ $^

clean:
    rm -f chess_engine chess_engine_debug $(OBJ)