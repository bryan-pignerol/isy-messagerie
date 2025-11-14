/*============================================================================*
 * ISY MESSAGERIE - Commun.h
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Fichier d'en-tête commun pour tous les programmes ISY
 *                Contient les structures, constantes et définitions communes
 *============================================================================*/

#ifndef COMMUN_H
#define COMMUN_H

/*============================================================================*
 * INCLUDES SYSTEME
 *============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>

/*============================================================================*
 * CONSTANTES GENERALES
 *============================================================================*/
#define TAILLE_ORDRE 5
#define TAILLE_EMETTEUR 20
#define TAILLE_TEXTE 100
#define TAILLE_NOM_GROUPE 50
#define TAILLE_MOT_PASSE 50
#define TAILLE_IP 16

#define PORT_SERVEUR 8000
#define PORT_GROUPE_BASE 8000

#define MAX_GROUPES 20
#define MAX_MEMBRES_GROUPE 50
#define TAILLE_SHM 4096

#define POINTS_PAR_MESSAGE 10
#define COUT_COULEUR 50
#define COUT_HIGHLIGHT 100

/*============================================================================*
 * ORDRES DES MESSAGES
 *============================================================================*/
#define ORDRE_CON "CON"      /* Connexion */
#define ORDRE_DECI "DECI"    /* Deconnexion */
#define ORDRE_MES "MES"      /* Message */
#define ORDRE_CMD "CMD"      /* Commande */
#define ORDRE_CRE "CRE"      /* Creation groupe */
#define ORDRE_DES "DES"      /* Destruction groupe */
#define ORDRE_LST "LST"      /* Liste groupes */
#define ORDRE_ACK "ACK"      /* Accusé de réception */
#define ORDRE_ERR "ERR"      /* Erreur */
#define ORDRE_INF "INF"      /* Information */

/*============================================================================*
 * STRUCTURE DES MESSAGES RESEAU
 *============================================================================*/
struct struct_message
{
    char Ordre[TAILLE_ORDRE];
    char Emetteur[TAILLE_EMETTEUR];
    char Texte[TAILLE_TEXTE];
};

/*============================================================================*
 * STRUCTURE DE CONFIGURATION SERVEUR
 *============================================================================*/
typedef struct
{
    char ip[TAILLE_IP];
    int port;
    char protocole[10];
} ConfigServeur;

/*============================================================================*
 * STRUCTURE DE CONFIGURATION CLIENT
 *============================================================================*/
typedef struct
{
    char nom_utilisateur[TAILLE_EMETTEUR];
    char ip_serveur[TAILLE_IP];
    int port_serveur;
} ConfigClient;

/*============================================================================*
 * STRUCTURE D'UN MEMBRE DE GROUPE
 *============================================================================*/
typedef struct
{
    char nom[TAILLE_EMETTEUR];
    struct sockaddr_in adresse;
    int points;
    int actif;
} Membre;

/*============================================================================*
 * STRUCTURE D'UN GROUPE
 *============================================================================*/
typedef struct
{
    char nom[TAILLE_NOM_GROUPE];
    char mot_passe[TAILLE_MOT_PASSE];
    char moderateur[TAILLE_EMETTEUR];
    int port;
    pid_t pid_groupe;
    int nb_membres;
    Membre membres[MAX_MEMBRES_GROUPE];
    int nb_messages;
    int actif;
} Groupe;

/*============================================================================*
 * STRUCTURE DE LA MEMOIRE PARTAGEE
 *============================================================================*/
typedef struct
{
    int nb_groupes;
    Groupe groupes[MAX_GROUPES];
} MemoirePartagee;

/*============================================================================*
 * CODES COULEUR ANSI
 *============================================================================*/
#define COULEUR_RESET   "\033[0m"
#define COULEUR_ROUGE   "\033[31m"
#define COULEUR_VERT    "\033[32m"
#define COULEUR_JAUNE   "\033[33m"
#define COULEUR_BLEU    "\033[34m"
#define COULEUR_MAGENTA "\033[35m"
#define COULEUR_CYAN    "\033[36m"
#define COULEUR_BLANC   "\033[37m"
#define STYLE_GRAS      "\033[1m"
#define STYLE_HIGHLIGHT "\033[7m"

#endif /* COMMUN_H */
