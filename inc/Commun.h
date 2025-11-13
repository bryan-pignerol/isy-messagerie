/*============================================================================*
 * ISY MESSAGERIE - Commun.h
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Définitions communes à tous les programmes ISY
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
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

/*============================================================================*
 * CONSTANTES
 *============================================================================*/
#define PORT_SERVEUR_DEFAUT 8000
#define PORT_GROUPE_BASE 8100
#define MAX_GROUPES 50
#define MAX_MEMBRES 20

/* Taille de la structure message */
#define TAILLE_ORDRE 4
#define TAILLE_EMETTEUR 20
#define TAILLE_TEXTE 100

/* Ordres */
#define ORDRE_CON "CON"
#define ORDRE_DEC "DEC"
#define ORDRE_CRG "CRG"
#define ORDRE_DGR "DGR"
#define ORDRE_LST "LST"
#define ORDRE_JGR "JGR"
#define ORDRE_QGR "QGR"
#define ORDRE_MSG "MSG"
#define ORDRE_ACK "ACK"
#define ORDRE_ERR "ERR"

/*============================================================================*
 * STRUCTURES
 *============================================================================*/

/* Structure des messages échangés */
typedef struct {
    char Ordre[TAILLE_ORDRE];
    char Emetteur[TAILLE_EMETTEUR];
    char Texte[TAILLE_TEXTE];
} struct_message;

/* Structure d'un membre */
typedef struct {
    char nom[TAILLE_EMETTEUR];
    struct sockaddr_in adresse;
    int actif;
} struct_membre;

#endif /* COMMUN_H */
