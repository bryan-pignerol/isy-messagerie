/*============================================================================*
 * ISY MESSAGERIE - ServeurISY.h
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : En-tête du serveur principal ISY
 *============================================================================*/

#ifndef SERVEURISY_H
#define SERVEURISY_H

#include "Commun.h"

/*============================================================================*
 * PROTOTYPES DES FONCTIONS
 *============================================================================*/

/* Initialisation et configuration */
int lire_configuration(const char *fichier, ConfigServeur *config);

/* Gestion des sockets */
int initialiser_socket_serveur(ConfigServeur *config);

/* Gestion des signaux */
void gestionnaire_sigint(int sig);

/* Gestion des groupes */
int creer_groupe(const char *nom, const char *mot_passe, const char *moderateur);
int detruire_groupe(const char *nom);
int trouver_groupe(const char *nom);
void lister_groupes(char *buffer);

/* Gestion des messages */
void traiter_demande_connexion(struct struct_message *msg, struct sockaddr_in *addr_client);
void traiter_demande_deconnexion(struct struct_message *msg);
void traiter_demande_creation(struct struct_message *msg, struct sockaddr_in *addr_client);
void traiter_demande_liste(struct struct_message *msg, struct sockaddr_in *addr_client);

/* Nettoyage */
void terminer_proprement(void);

#endif /* SERVEURISY_H */
