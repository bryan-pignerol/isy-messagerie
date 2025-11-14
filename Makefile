#=============================================================================#
# ISY MESSAGERIE - Makefile
#=============================================================================#
# Auteur       : Bryan
# Date         : 14/11/2025
# Version      : 1.0
#-----------------------------------------------------------------------------#
# Description  : Makefile pour la compilation du projet ISY Messagerie
#=============================================================================#

#=============================================================================#
# VARIABLES DE COMPILATION
#=============================================================================#
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -I$(INC_DIR)
LDFLAGS = -pthread

#=============================================================================#
# REPERTOIRES
#=============================================================================#
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj

#=============================================================================#
# FICHIERS
#=============================================================================#
SERVEUR_SRC = $(SRC_DIR)/ServeurISY.c
GROUPE_SRC = $(SRC_DIR)/GroupeISY.c
CLIENT_SRC = $(SRC_DIR)/ClientISY.c
AFFICHAGE_SRC = $(SRC_DIR)/AffichageISY.c

SERVEUR_OBJ = $(OBJ_DIR)/ServeurISY.o
GROUPE_OBJ = $(OBJ_DIR)/GroupeISY.o
CLIENT_OBJ = $(OBJ_DIR)/ClientISY.o
AFFICHAGE_OBJ = $(OBJ_DIR)/AffichageISY.o

#=============================================================================#
# EXECUTABLES
#=============================================================================#
SERVEUR_BIN = ServeurISY
GROUPE_BIN = GroupeISY
CLIENT_BIN = ClientISY
AFFICHAGE_BIN = AffichageISY

EXECUTABLES = $(SERVEUR_BIN) $(GROUPE_BIN) $(CLIENT_BIN) $(AFFICHAGE_BIN)

#=============================================================================#
# REGLES PRINCIPALES
#=============================================================================#

all: $(EXECUTABLES)
	@echo ""
	@echo "=========================================="
	@echo "  Compilation terminee avec succes !"
	@echo "=========================================="
	@echo ""

#=============================================================================#
# COMPILATION DES EXECUTABLES
#=============================================================================#

$(SERVEUR_BIN): $(SERVEUR_OBJ)
	@echo "Linkage de $(SERVEUR_BIN)..."
	$(CC) $(SERVEUR_OBJ) -o $(SERVEUR_BIN) $(LDFLAGS)

$(GROUPE_BIN): $(GROUPE_OBJ)
	@echo "Linkage de $(GROUPE_BIN)..."
	$(CC) $(GROUPE_OBJ) -o $(GROUPE_BIN) $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_OBJ)
	@echo "Linkage de $(CLIENT_BIN)..."
	$(CC) $(CLIENT_OBJ) -o $(CLIENT_BIN) $(LDFLAGS)

$(AFFICHAGE_BIN): $(AFFICHAGE_OBJ)
	@echo "Linkage de $(AFFICHAGE_BIN)..."
	$(CC) $(AFFICHAGE_OBJ) -o $(AFFICHAGE_BIN) $(LDFLAGS)

#=============================================================================#
# COMPILATION DES FICHIERS OBJETS
#=============================================================================#

$(OBJ_DIR)/ServeurISY.o: $(SERVEUR_SRC) $(INC_DIR)/ServeurISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de ServeurISY.c..."
	$(CC) $(CFLAGS) -c $(SERVEUR_SRC) -o $(OBJ_DIR)/ServeurISY.o

$(OBJ_DIR)/GroupeISY.o: $(GROUPE_SRC) $(INC_DIR)/GroupeISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de GroupeISY.c..."
	$(CC) $(CFLAGS) -c $(GROUPE_SRC) -o $(OBJ_DIR)/GroupeISY.o

$(OBJ_DIR)/ClientISY.o: $(CLIENT_SRC) $(INC_DIR)/ClientISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de ClientISY.c..."
	$(CC) $(CFLAGS) -c $(CLIENT_SRC) -o $(OBJ_DIR)/ClientISY.o

$(OBJ_DIR)/AffichageISY.o: $(AFFICHAGE_SRC) $(INC_DIR)/AffichageISY.h $(INC_DIR)/Commun.h
	@echo "Compilation de AffichageISY.c..."
	$(CC) $(CFLAGS) -c $(AFFICHAGE_SRC) -o $(OBJ_DIR)/AffichageISY.o

#=============================================================================#
# NETTOYAGE
#=============================================================================#

clean:
	@echo "Nettoyage des fichiers objets..."
	rm -f $(OBJ_DIR)/*.o
	@echo "Fichiers objets supprimes."

fclean: clean
	@echo "Nettoyage des executables..."
	rm -f $(EXECUTABLES)
	@echo "Executables supprimes."

re: fclean all

#=============================================================================#
# REGLES UTILES
#=============================================================================#

serveur: $(SERVEUR_BIN)
	@echo "$(SERVEUR_BIN) compile."

groupe: $(GROUPE_BIN)
	@echo "$(GROUPE_BIN) compile."

client: $(CLIENT_BIN)
	@echo "$(CLIENT_BIN) compile."

affichage: $(AFFICHAGE_BIN)
	@echo "$(AFFICHAGE_BIN) compile."

#=============================================================================#
# REGLES PHONY
#=============================================================================#

.PHONY: all clean fclean re serveur groupe client affichage
