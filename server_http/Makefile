CC = gcc

# Parametri per il compilatore:
# -Wall: Attiva tutti i messaggi di avviso
# -Iinclude: Cerca i file header (.h) nella cartella include
# -lpthread: Include la libreria per la gestione dei thread
CFLAGS = -Wall -Iinclude -lpthread

# Definiamo le cartelle del progetto
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

# Il nome del file finale
TARGET = $(BIN_DIR)/server

# Cerchiamo automaticamente tutti i file sorgente (.c)
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Trasformiamo la lista dei file .c in file oggetto (.o)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Regola di default
all: $(TARGET)

# Crea l'eseguibile unendo (linking) tutti i file oggetto
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET) -lpthread

# Regola generica per compilare ogni singolo file .c in un file oggetto .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rimuove le cartelle dei file compilati per pulire il progetto
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Scorciatoia per compilare tutto e avviare immediatamente il server
run: all
	./$(TARGET)