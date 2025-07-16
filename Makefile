# Nome do executável
TARGET = ndn

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Pastas
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

# Ficheiros fonte e objeto
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# Instrução principal
all: $(TARGET)

# Como compilar o executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Como compilar os .o
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Criar pasta bin se não existir
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Limpeza
clean:
	rm -rf $(BIN_DIR)/*.o $(TARGET)

# Limpeza completa
fclean: clean
	rm -rf $(BIN_DIR)

# Reconstrução total
re: fclean all
