CC = gcc
CFLAGS = -Wall -Wpedantic -Wextra -ggdb -std=c18
CINCLUDES = -I./includes/raylib/src
CLIBS = -L./includes/raylib/src -lraylib -lgcrypt -lm

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    detected_OS := Windows
else
    detected_OS := $(shell uname)  # same as "uname -s"
endif

ifeq ($(detected_OS),Windows)
		CLIBS += -lgdi32 -lwinmm -lopengl32
endif

SRC = src
OBJ = obj

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

BINDIR = bin
BIN = $(BINDIR)/itsraining

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CINCLUDES) $^ -o $@ $(CLIBS)

$(OBJ)/%.o: $(SRC)/%.c
	export PLATFORM_OS=WINDOWS
	@$(MAKE) -C ./includes/raylib/src/
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CINCLUDES) -c $< -o $@

clean:
	rm -rf $(BINDIR) $(OBJ)
	@$(MAKE) -C ./includes/raylib/src/ clean

run: $(BIN)
	./$(BIN)

$(OBJ):
	@mkdir -p $@

