# ============================================================================
# Makefile pour le projet ISY Messagerie
# ============================================================================

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -g
LDFLAGS = -lpthread

# Répertoires
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj

# Fichiers sources
SRC_SERVEUR = $(SRC_DIR)/ServeurISY.c
SRC_GROUPE = $(SRC_DIR)/GroupeISY.c
SRC_CLIENT = $(SRC_DIR)/ClientISY.c
SRC_AFFICHAGE = $(SRC_DIR)/AffichageISY.c

# Fichiers objets
OBJ_SERVEUR = $(OBJ_DIR)/ServeurISY.o
OBJ_GROUPE = $(OBJ_DIR)/GroupeISY.o
OBJ_CLIENT = $(OBJ_DIR)/ClientISY.o
OBJ_AFFICHAGE = $(OBJ_DIR)/AffichageISY.o

# Fichiers d'en-tête
HEADERS = $(INC_DIR)/Commun.h

# Exécutables
EXEC_SERVEUR = ServeurISY
EXEC_GROUPE = GroupeISY
EXEC_CLIENT = ClientISY
EXEC_AFFICHAGE = AffichageISY

# Tous les exécutables
ALL_EXEC = $(EXEC_SERVEUR) $(EXEC_GROUPE) $(EXEC_CLIENT) $(EXEC_AFFICHAGE)

# ============================================================================
# Règles principales
# ============================================================================

# Compilation de tous les programmes
all: $(OBJ_DIR) $(ALL_EXEC)
	@echo "==> Compilation terminée avec succès !"

# Création du répertoire obj s'il n'existe pas
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@echo "==> Répertoire $(OBJ_DIR) créé"

# ============================================================================
# Règles de compilation des exécutables
# ============================================================================

$(EXEC_SERVEUR): $(OBJ_SERVEUR)
	@echo "==> Création de l'exécutable $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(EXEC_GROUPE): $(OBJ_GROUPE)
	@echo "==> Création de l'exécutable $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(EXEC_CLIENT): $(OBJ_CLIENT)
	@echo "==> Création de l'exécutable $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(EXEC_AFFICHAGE): $(OBJ_AFFICHAGE)
	@echo "==> Création de l'exécutable $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ============================================================================
# Règles de compilation des fichiers objets
# ============================================================================

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@echo "==> Compilation de $<"
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# ============================================================================
# Règles de nettoyage
# ============================================================================

# Nettoyage des fichiers objets
clean:
	@echo "==> Nettoyage des fichiers objets"
	rm -rf $(OBJ_DIR)

# Nettoyage complet (objets + exécutables)
fclean: clean
	@echo "==> Nettoyage des exécutables"
	rm -f $(ALL_EXEC)

# Recompilation complète
re: fclean all

# ============================================================================
# Règles spécifiques
# ============================================================================

# Compilation du serveur uniquement
serveur: $(OBJ_DIR) $(EXEC_SERVEUR)
	@echo "==> ServeurISY compilé"

# Compilation du gestionnaire de groupe uniquement
groupe: $(OBJ_DIR) $(EXEC_GROUPE)
	@echo "==> GroupeISY compilé"

# Compilation du client uniquement
client: $(OBJ_DIR) $(EXEC_CLIENT)
	@echo "==> ClientISY compilé"

# Compilation de l'affichage uniquement
affichage: $(OBJ_DIR) $(EXEC_AFFICHAGE)
	@echo "==> AffichageISY compilé"

# ============================================================================
# Règles utilitaires
# ============================================================================

# Affichage de l'aide
help:
	@echo "============================================"
	@echo "Makefile pour ISY Messagerie"
	@echo "============================================"
	@echo "Cibles disponibles :"
	@echo "  all        : Compile tous les programmes (défaut)"
	@echo "  serveur    : Compile uniquement ServeurISY"
	@echo "  groupe     : Compile uniquement GroupeISY"
	@echo "  client     : Compile uniquement ClientISY"
	@echo "  affichage  : Compile uniquement AffichageISY"
	@echo "  clean      : Supprime les fichiers objets"
	@echo "  fclean     : Supprime les objets et exécutables"
	@echo "  re         : Recompile tout depuis zéro"
	@echo "  help       : Affiche cette aide"
	@echo "============================================"

# Déclaration des cibles PHONY
.PHONY: all clean fclean re serveur groupe client affichage help
