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

# Fichiers sources
SERVEUR_SRC = $(SRC_DIR)/ServeurISY.c
GROUPE_SRC = $(SRC_DIR)/GroupeISY.c
CLIENT_SRC = $(SRC_DIR)/ClientISY.c
AFFICHAGE_SRC = $(SRC_DIR)/AffichageISY.c

# Fichiers objets
SERVEUR_OBJ = $(OBJ_DIR)/ServeurISY.o
GROUPE_OBJ = $(OBJ_DIR)/GroupeISY.o
CLIENT_OBJ = $(OBJ_DIR)/ClientISY.o
AFFICHAGE_OBJ = $(OBJ_DIR)/AffichageISY.o

# Exécutables
SERVEUR_BIN = ServeurISY
GROUPE_BIN = GroupeISY
CLIENT_BIN = ClientISY
AFFICHAGE_BIN = AffichageISY

# Règle par défaut
all: $(OBJ_DIR) $(SERVEUR_BIN) $(GROUPE_BIN) $(CLIENT_BIN) $(AFFICHAGE_BIN)
	@echo "Compilation terminée avec succès !"
	@echo "Programmes disponibles : ServeurISY, GroupeISY, ClientISY, AffichageISY"

# Création du répertoire obj
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compilation de ServeurISY
$(SERVEUR_BIN): $(SERVEUR_OBJ)
	@echo "Édition des liens pour $(SERVEUR_BIN)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SERVEUR_OBJ): $(SERVEUR_SRC) $(INC_DIR)/ServeurISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de ServeurISY.c..."
	$(CC) $(CFLAGS) -c $(SERVEUR_SRC) -o $(SERVEUR_OBJ)

# Compilation de GroupeISY
$(GROUPE_BIN): $(GROUPE_OBJ)
	@echo "Édition des liens pour $(GROUPE_BIN)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(GROUPE_OBJ): $(GROUPE_SRC) $(INC_DIR)/GroupeISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de GroupeISY.c..."
	$(CC) $(CFLAGS) -c $(GROUPE_SRC) -o $(GROUPE_OBJ)

# Compilation de ClientISY
$(CLIENT_BIN): $(CLIENT_OBJ)
	@echo "Édition des liens pour $(CLIENT_BIN)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT_OBJ): $(CLIENT_SRC) $(INC_DIR)/ClientISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de ClientISY.c..."
	$(CC) $(CFLAGS) -c $(CLIENT_SRC) -o $(CLIENT_OBJ)

# Compilation de AffichageISY
$(AFFICHAGE_BIN): $(AFFICHAGE_OBJ)
	@echo "Édition des liens pour $(AFFICHAGE_BIN)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(AFFICHAGE_OBJ): $(AFFICHAGE_SRC) $(INC_DIR)/AffichageISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de AffichageISY.c..."
	$(CC) $(CFLAGS) -c $(AFFICHAGE_SRC) -o $(AFFICHAGE_OBJ)

# Nettoyage des fichiers objets
clean:
	@echo "Nettoyage des fichiers objets..."
	rm -rf $(OBJ_DIR)

# Nettoyage complet
mrproper: clean
	@echo "Nettoyage complet..."
	rm -f $(SERVEUR_BIN) $(GROUPE_BIN) $(CLIENT_BIN) $(AFFICHAGE_BIN)

# Recompilation complète
rebuild: mrproper all

# Aide
help:
	@echo "Makefile pour ISY Messagerie"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  all       - Compile tous les programmes (défaut)"
	@echo "  clean     - Supprime les fichiers objets"
	@echo "  mrproper  - Supprime les objets et exécutables"
	@echo "  rebuild   - Recompile complètement"
	@echo "  help      - Affiche cette aide"

.PHONY: all clean mrproper rebuild help
