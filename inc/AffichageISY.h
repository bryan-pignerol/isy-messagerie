/*============================================================================*
 * ISY MESSAGERIE - AffichageISY.h
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : En-tête du programme d'affichage des messages
 *============================================================================*/

#ifndef AFFICHAGEISY_H
#define AFFICHAGEISY_H

#include "Commun.h"

/*============================================================================*
 * PROTOTYPES DES FONCTIONS
 *============================================================================*/

/* Initialisation */
int initialiser_socket_affichage(int port_groupe);

/* Gestion des signaux */
void gestionnaire_sigint_affichage(int sig);

/* Affichage des messages */
void afficher_message(struct struct_message *msg);
void afficher_connexion(const char *nom);
void afficher_deconnexion(const char *nom);

/* Boucle principale */
void boucle_reception(int socket_fd, const char *nom_groupe, const char *moderateur);

/* Nettoyage */
void terminer_affichage_proprement(void);

#endif /* AFFICHAGEISY_H */
