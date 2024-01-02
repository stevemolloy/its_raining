CC = gcc
CFLAGS = -Wall -Wpedantic -Wextra -ggdb -std=c18
CINCLUDES = -I./includes/raylib/src
CLIBS = -L./includes/raylib/src -lraylib -lgcrypt -lm

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

