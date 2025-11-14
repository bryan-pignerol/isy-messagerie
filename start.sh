#!/bin/bash

#=============================================================================#
# ISY MESSAGERIE - Script de demarrage rapide
#=============================================================================#
# Auteur       : Bryan
# Date         : 14/11/2025
# Version      : 1.0
#-----------------------------------------------------------------------------#
# Description  : Script pour tester rapidement le projet
#=============================================================================#

echo "=========================================="
echo "  ISY MESSAGERIE - Demarrage Rapide"
echo "=========================================="
echo ""

# Compilation si necessaire
if [ ! -f "./ServeurISY" ]; then
    echo "Compilation du projet..."
    make
    echo ""
fi

echo "Options:"
echo "  1 - Lancer le serveur"
echo "  2 - Lancer un client (Bryan)"
echo "  3 - Lancer un second client (Sophie)"
echo "  4 - Tout nettoyer"
echo ""
read -p "Votre choix : " choix

case $choix in
    1)
        echo ""
        echo "Lancement du serveur..."
        echo "Appuyez sur CTRL+C pour arreter"
        echo ""
        ./ServeurISY
        ;;
    2)
        echo ""
        echo "Lancement du client Bryan..."
        echo ""
        ./ClientISY
        ;;
    3)
        echo ""
        echo "Lancement du client Sophie..."
        echo ""
        cp client_config.txt client_config_backup.txt
        cp client_config2.txt client_config.txt
        ./ClientISY
        mv client_config_backup.txt client_config.txt
        ;;
    4)
        echo ""
        echo "Nettoyage..."
        make fclean
        # Supprimer les segments de memoire partagee orphelins
        ipcs -m | grep $(whoami) | awk '{print $2}' | xargs -I {} ipcrm -m {} 2>/dev/null
        echo "Nettoyage termine!"
        ;;
    *)
        echo "Choix invalide"
        ;;
esac
