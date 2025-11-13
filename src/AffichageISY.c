/*============================================================================*
 * ISY MESSAGERIE - AffichageISY.c
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Affichage des messages reçus d'un groupe de discussion
 *============================================================================*/

#include "AffichageISY.h"

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
int socket_affichage = -1;
int arret_demande = 0;
char nom_groupe[50];
char nom_utilisateur[TAILLE_EMETTEUR];

/*============================================================================*
 * FONCTION MAIN
 *============================================================================*/
int main(int argc, char *argv[]) {
    struct sockaddr_in adresse_groupe, adresse_locale;
    struct_message msg_connexion, msg_recu;
    socklen_t taille_adresse;
    ssize_t octets_recus;
    int port_groupe;
    
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <nom_groupe> <port_groupe> <nom_utilisateur>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    strncpy(nom_groupe, argv[1], 49);
    port_groupe = atoi(argv[2]);
    strncpy(nom_utilisateur, argv[3], TAILLE_EMETTEUR - 1);
    
    printf("Groupe de discussion %s\n", nom_groupe);
    
    /* Installation des gestionnaires de signaux */
    signal(SIGINT, gestionnaire_sigint);
    signal(SIGTERM, gestionnaire_sigterm);
    
    /* Création du socket UDP */
    socket_affichage = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_affichage < 0) {
        perror("Erreur création socket");
        return EXIT_FAILURE;
    }
    
    /* Bind sur un port quelconque pour recevoir les messages */
    memset(&adresse_locale, 0, sizeof(struct sockaddr_in));
    adresse_locale.sin_family = AF_INET;
    adresse_locale.sin_port = 0;
    adresse_locale.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(socket_affichage, (struct sockaddr *)&adresse_locale,
             sizeof(struct sockaddr_in)) < 0) {
        perror("Erreur bind");
        close(socket_affichage);
        return EXIT_FAILURE;
    }
    
    /* Configuration de l'adresse du groupe */
    memset(&adresse_groupe, 0, sizeof(struct sockaddr_in));
    adresse_groupe.sin_family = AF_INET;
    adresse_groupe.sin_port = htons(port_groupe);
    adresse_groupe.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    /* Envoi d'un message de connexion au groupe */
    memset(&msg_connexion, 0, sizeof(struct_message));
    strncpy(msg_connexion.Ordre, ORDRE_CON, TAILLE_ORDRE - 1);
    strncpy(msg_connexion.Emetteur, nom_utilisateur, TAILLE_EMETTEUR - 1);
    
    sendto(socket_affichage, &msg_connexion, sizeof(struct_message), 0,
           (struct sockaddr *)&adresse_groupe, sizeof(struct sockaddr_in));
    
    /* Boucle de réception des messages */
    while (!arret_demande) {
        memset(&msg_recu, 0, sizeof(struct_message));
        taille_adresse = sizeof(struct sockaddr_in);
        
        octets_recus = recvfrom(socket_affichage, &msg_recu, sizeof(struct_message),
                                0, (struct sockaddr *)&adresse_groupe, &taille_adresse);
        
        if (octets_recus < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Erreur recvfrom");
            break;
        }
        
        if (octets_recus > 0) {
            afficher_message(&msg_recu);
        }
    }
    
    /* Envoi d'un message de déconnexion */
    memset(&msg_connexion, 0, sizeof(struct_message));
    strncpy(msg_connexion.Ordre, ORDRE_QGR, TAILLE_ORDRE - 1);
    strncpy(msg_connexion.Emetteur, nom_utilisateur, TAILLE_EMETTEUR - 1);
    
    sendto(socket_affichage, &msg_connexion, sizeof(struct_message), 0,
           (struct sockaddr *)&adresse_groupe, sizeof(struct sockaddr_in));
    
    /* Nettoyage */
    if (socket_affichage >= 0) {
        close(socket_affichage);
    }
    
    printf("Affichage clos\n");
    
    return EXIT_SUCCESS;
}

/*============================================================================*
 * IMPLÉMENTATION DES FONCTIONS
 *============================================================================*/

void afficher_message(struct_message *msg) {
    if (strncmp(msg->Ordre, ORDRE_MSG, 3) == 0) {
        printf("Message de %s : %s\n", msg->Emetteur, msg->Texte);
    }
}

void gestionnaire_sigint(int sig) {
    (void)sig;
    arret_demande = 1;
}

void gestionnaire_sigterm(int sig) {
    (void)sig;
    arret_demande = 1;
}
