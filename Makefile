# Variables
CC = gcc
CFLAGS = -Wall -I./include
SRC_DIR = src
OBJ_DIR = build
EXEC = main.exe

# Archivos fuente y objetos
SRC_FILES = $(wildcard $(SRC_DIR)/*.c) main.c
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Regla por defecto
all: $(EXEC)

# Crear el ejecutable
$(EXEC): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(EXEC)

# Compilar los archivos .c a .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar los archivos generados
clean:
	rm -rf $(OBJ_DIR) $(EXEC)
