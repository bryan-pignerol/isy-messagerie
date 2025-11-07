/*============================================================================*
 * ISY MESSAGERIE - ServeurISY.h
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Serveur principal de gestion des groupes de messagerie
 *                Gestion des connexions clientes et création des processus
 *                GroupeISY via fork() et communication par SHM
 *============================================================================*/

#ifndef SERVEURISY_H
#define SERVEURISY_H

/*============================================================================*
 * INCLUDES SYSTEME
 *============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>

/*============================================================================*
 * CONSTANTES ET DEFINES
 *============================================================================*/
#define PORT_SERVEUR_DEFAUT 8000
#define PORT_GROUPE_BASE 8100
#define MAX_GROUPES 50
#define MAX_MEMBRES 20
#define BUFFER_SIZE 1024
#define CONFIG_FILE "serveur_config.txt"

// Taille de la structure message
#define TAILLE_ORDRE 4
#define TAILLE_EMETTEUR 20
#define TAILLE_TEXTE 100

// Ordres possibles
#define ORDRE_CON "CON"     // Connexion
#define ORDRE_DEC "DEC"     // Déconnexion
#define ORDRE_CRG "CRG"     // Création de groupe
#define ORDRE_DGR "DGR"     // Destruction de groupe
#define ORDRE_LST "LST"     // Liste des groupes
#define ORDRE_JGR "JGR"     // Joindre un groupe
#define ORDRE_QGR "QGR"     // Quitter un groupe
#define ORDRE_ACK "ACK"     // Accusé de réception
#define ORDRE_ERR "ERR"     // Erreur

/*============================================================================*
 * STRUCTURES
 *============================================================================*/

/**
 * Structure des messages échangés sur le réseau
 */
typedef struct {
    char Ordre[TAILLE_ORDRE];
    char Emetteur[TAILLE_EMETTEUR];
    char Texte[TAILLE_TEXTE];
} struct_message;

/**
 * Structure représentant un membre d'un groupe
 */
typedef struct {
    char nom[TAILLE_EMETTEUR];
    struct sockaddr_in adresse;
    int actif;
} struct_membre;

/**
 * Structure représentant un groupe de discussion
 */
typedef struct {
    char nom[50];
    char moderateur[TAILLE_EMETTEUR];
    int port;
    pid_t pid_processus;
    int actif;
    int nombre_membres;
    int shmid;  // ID de la mémoire partagée
} struct_groupe;

/**
 * Structure de la mémoire partagée entre ServeurISY et GroupeISY
 */
typedef struct {
    int actif;
    int nombre_membres;
    char moderateur[TAILLE_EMETTEUR];
    struct_membre membres[MAX_MEMBRES];
    char commande[100];  // Commandes du serveur vers le groupe
    int flag_commande;   // 1 si nouvelle commande disponible
} struct_shm_groupe;

/**
 * Structure de configuration du serveur
 */
typedef struct {
    char ip_serveur[16];
    int port_serveur;
} struct_config;

/*============================================================================*
 * PROTOTYPES DE FONCTIONS
 *============================================================================*/

/**
 * Initialisation du serveur UDP
 */
int initialiser_serveur(struct_config *config);

/**
 * Lecture du fichier de configuration
 */
int lire_configuration(struct_config *config);

/**
 * Gestion du signal SIGINT (CTRL-C)
 */
void gestionnaire_sigint(int sig);

/**
 * Gestion du signal SIGCHLD (terminaison d'un processus fils)
 */
void gestionnaire_sigchld(int sig);

/**
 * Création d'un nouveau groupe
 */
int creer_groupe(const char *nom_groupe, const char *moderateur, int *port_groupe);

/**
 * Destruction d'un groupe
 */
int detruire_groupe(const char *nom_groupe, const char *demandeur);

/**
 * Recherche d'un groupe par son nom
 */
int trouver_groupe(const char *nom_groupe);

/**
 * Traitement d'une demande de création de groupe
 */
void traiter_creation_groupe(struct_message *msg, struct sockaddr_in *adresse_client);

/**
 * Traitement d'une demande de connexion à un groupe
 */
void traiter_connexion_groupe(struct_message *msg, struct sockaddr_in *adresse_client);

/**
 * Traitement d'une demande de liste des groupes
 */
void traiter_liste_groupes(struct_message *msg, struct sockaddr_in *adresse_client);

/**
 * Traitement d'une demande de déconnexion
 */
void traiter_deconnexion(struct_message *msg, struct sockaddr_in *adresse_client);

/**
 * Traitement d'une demande de destruction de groupe
 */
void traiter_destruction_groupe(struct_message *msg, struct sockaddr_in *adresse_client);

/**
 * Traitement des messages reçus
 */
void traiter_message(struct_message *msg, struct sockaddr_in *adresse_client);

/**
 * Envoi d'un message à un client via UDP
 */
int envoyer_message(struct_message *msg, struct sockaddr_in *adresse_client);

/**
 * Affichage de l'état du serveur
 */
void afficher_etat_serveur();

/**
 * Nettoyage et fermeture propre du serveur
 */
void arreter_serveur();

/**
 * Terminaison propre de tous les groupes
 */
void terminer_tous_groupes();

#endif /* SERVEURISY_H */
