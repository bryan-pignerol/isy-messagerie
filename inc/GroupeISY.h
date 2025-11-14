/*============================================================================*
 * ISY MESSAGERIE - GroupeISY.h
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : En-tête du gestionnaire de groupe de discussion
 *============================================================================*/

#ifndef GROUPEISY_H
#define GROUPEISY_H

#include "Commun.h"

/*============================================================================*
 * PROTOTYPES DES FONCTIONS
 *============================================================================*/

/* Initialisation */
int initialiser_socket_groupe(int port);

/* Gestion des signaux */
void gestionnaire_sigint_groupe(int sig);

/* Gestion des membres */
int ajouter_membre(const char *nom, struct sockaddr_in *addr);
int retirer_membre(const char *nom);
int trouver_membre(const char *nom);

/* Gestion des messages */
void redistribuer_message(struct struct_message *msg, int socket_fd);
void traiter_message(struct struct_message *msg, struct sockaddr_in *addr_client, int socket_fd);

/* Gestion des points */
void ajouter_points(const char *nom, int points);

/* Nettoyage */
void terminer_groupe_proprement(void);

#endif /* GROUPEISY_H */
