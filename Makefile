CC = gcc
SRC = src
INC = include
OBJ = obj
BIN = editr
SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) -c $< -o $@

$(OBJ):
	mkdir -p $(OBJ)

clean:
	rm -rf $(BIN) $(OBJ)
