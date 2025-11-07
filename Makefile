# Makefile pour le projet ISY Messagerie
# Auteur : Votre Nom
# Date : 07/11/2025

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinc
LDFLAGS = 

# Répertoires
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj

# Fichiers
SERVEUR_SRC = $(SRC_DIR)/ServeurISY.c
SERVEUR_OBJ = $(OBJ_DIR)/ServeurISY.o
SERVEUR_BIN = ServeurISY

# Règle par défaut
all: $(OBJ_DIR) $(SERVEUR_BIN)
	@echo "Compilation terminée avec succès !"
	@echo "Exécutez './ServeurISY' pour lancer le serveur"

# Création du répertoire obj s'il n'existe pas
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compilation du serveur
$(SERVEUR_BIN): $(SERVEUR_OBJ)
	@echo "Édition des liens pour $(SERVEUR_BIN)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilation du fichier objet
$(OBJ_DIR)/ServeurISY.o: $(SRC_DIR)/ServeurISY.c $(INC_DIR)/ServeurISY.h
	@echo "Compilation de ServeurISY.c..."
	$(CC) $(CFLAGS) -c $(SRC_DIR)/ServeurISY.c -o $(OBJ_DIR)/ServeurISY.o

# Nettoyage des fichiers objets
clean:
	@echo "Nettoyage des fichiers objets..."
	rm -rf $(OBJ_DIR)

# Nettoyage complet (objets + exécutables)
mrproper: clean
	@echo "Nettoyage complet..."
	rm -f $(SERVEUR_BIN)

# Recompilation complète
rebuild: mrproper all

# Aide
help:
	@echo "Makefile pour ISY Messagerie - ServeurISY"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  all       - Compile le serveur (défaut)"
	@echo "  clean     - Supprime les fichiers objets"
	@echo "  mrproper  - Supprime les objets et exécutables"
	@echo "  rebuild   - Recompile complètement"
	@echo "  help      - Affiche cette aide"

.PHONY: all clean mrproper rebuild help