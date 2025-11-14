/*============================================================================*
 * ISY MESSAGERIE - ClientISY.h
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : En-tête du client pour la gestion des commandes
 *============================================================================*/

#ifndef CLIENTISY_H
#define CLIENTISY_H

#include "Commun.h"

/*============================================================================*
 * PROTOTYPES DES FONCTIONS
 *============================================================================*/

/* Initialisation et configuration */
int lire_configuration_client(const char *fichier, ConfigClient *config);

/* Gestion des sockets */
int initialiser_socket_client(void);

/* Menu et interface */
void afficher_menu(void);
int lire_choix(void);

/* Gestion des signaux */
void gestionnaire_sigint_client(int sig);

/* Commandes */
void creer_groupe_cmd(int socket_fd, ConfigClient *config);
void rejoindre_groupe_cmd(int socket_fd, ConfigClient *config);
void lister_groupes_cmd(int socket_fd, ConfigClient *config);
void envoyer_message(int socket_fd, const char *message, const char *groupe);

/* Processus d'affichage */
pid_t lancer_affichage(const char *nom_groupe, int port_groupe, const char *moderateur);

/* Nettoyage */
void terminer_client_proprement(void);

#endif /* CLIENTISY_H */
