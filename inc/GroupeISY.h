/*============================================================================*
 * ISY MESSAGERIE - GroupeISY.h
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Gestion des discussions au sein d'un groupe de messagerie
 *                Redistribution des messages aux membres du groupe
 *============================================================================*/

#ifndef GROUPEISY_H
#define GROUPEISY_H

#include "Commun.h"

/*============================================================================*
 * STRUCTURES
 *============================================================================*/

/* Structure de la mémoire partagée avec ServeurISY */
typedef struct {
    int actif;
    int nombre_membres;
    char moderateur[TAILLE_EMETTEUR];
    struct_membre membres[MAX_MEMBRES];
    char commande[100];
    int flag_commande;
} struct_shm_groupe;

/*============================================================================*
 * PROTOTYPES
 *============================================================================*/

int initialiser_groupe(int port);
void traiter_message(struct_message *msg, struct sockaddr_in *adresse_client);
void redistribuer_message(struct_message *msg);
int ajouter_membre(const char *nom, struct sockaddr_in *adresse);
int retirer_membre(const char *nom);
int chercher_membre(const char *nom);
void gestionnaire_sigint(int sig);
void gestionnaire_sigterm(int sig);

#endif /* GROUPEISY_H */
