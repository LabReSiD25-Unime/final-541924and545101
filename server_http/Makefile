# Definiamo il compilatore da utilizzare (GCC è lo standard per il C su Linux)
CC = gcc

# Parametri per il compilatore:
# -Wall: Attiva tutti i messaggi di avviso (Warnings) per trovare errori logici
# -Iinclude: Indica al compilatore di cercare i file header (.h) nella cartella include
# -lpthread: Include la libreria per la gestione dei thread (fondamentale per il pool)
CFLAGS = -Wall -Iinclude -lpthread

# Definiamo le cartelle del progetto per mantenere l'ordine
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

# Il nome del file finale (l'eseguibile) che verrà creato nella cartella bin
TARGET = $(BIN_DIR)/server

# Cerchiamo automaticamente tutti i file sorgente (.c) nella cartella src
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Trasformiamo la lista dei file .c in una lista di file oggetto (.o) nella cartella obj
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Regola di default: esegue la compilazione del target finale
all: $(TARGET)

# Regola per creare l'eseguibile unendo (linking) tutti i file oggetto
$(TARGET): $(OBJS)
	# Crea la cartella bin se non esiste (-p evita errori se è già presente)
	mkdir -p $(BIN_DIR)
	# Esegue il linking finale includendo la libreria pthread
	$(CC) $(OBJS) -o $(TARGET) -lpthread

# Regola generica per compilare ogni singolo file .c in un file oggetto .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	# Crea la cartella obj per i file intermedi
	mkdir -p $(OBJ_DIR)
	# Compila il file (-c) senza eseguire il linking, generando il file oggetto (-o)
	$(CC) $(CFLAGS) -c $< -o $@

# Rimuove le cartelle dei file compilati per pulire il progetto e ricompilare da zero
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Scorciatoia per compilare tutto e avviare immediatamente il server
run: all
	./$(TARGET)