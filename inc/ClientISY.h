/*============================================================================*
 * ISY MESSAGERIE - ClientISY.h
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Interface client pour la gestion des commandes utilisateur
 *                et l'envoi de messages aux groupes
 *============================================================================*/

#ifndef CLIENTISY_H
#define CLIENTISY_H

#include "Commun.h"

/*============================================================================*
 * STRUCTURES
 *============================================================================*/

/* Configuration du client */
typedef struct {
    char nom_utilisateur[TAILLE_EMETTEUR];
    char ip_serveur[16];
    int port_serveur;
} struct_config_client;

/* Groupe rejoint par le client */
typedef struct {
    char nom[50];
    int port;
    char moderateur[TAILLE_EMETTEUR];
    pid_t pid_affichage;
    int actif;
} struct_groupe_client;

/*============================================================================*
 * PROTOTYPES
 *============================================================================*/

int lire_configuration(struct_config_client *config);
void afficher_menu();
void creer_groupe();
void rejoindre_groupe();
void lister_groupes();
void dialoguer_groupe();
int envoyer_serveur(struct_message *msg);
int recevoir_serveur(struct_message *msg);
void gestionnaire_sigint(int sig);

#endif /* CLIENTISY_H */
