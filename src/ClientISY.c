/*============================================================================*
 * ISY MESSAGERIE - ClientISY.c
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Interface client pour la gestion des commandes utilisateur
 *============================================================================*/

#include "ClientISY.h"

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
int socket_client = -1;
struct sockaddr_in adresse_serveur;
struct_config_client config;
struct_groupe_client groupes_rejoints[MAX_GROUPES];
int nombre_groupes_rejoints = 0;
int arret_demande = 0;

/*============================================================================*
 * FONCTION MAIN
 *============================================================================*/
int main(int argc, char *argv[]) {
    int choix;
    
    (void)argc;
    (void)argv;
    
    printf("=======================================================\n");
    printf("          ISY MESSAGERIE - CLIENT                     \n");
    printf("=======================================================\n\n");
    
    /* Lecture de la configuration */
    if (lire_configuration(&config) < 0) {
        fprintf(stderr, "Erreur lecture configuration\n");
        strcpy(config.ip_serveur, "127.0.0.1");
        config.port_serveur = PORT_SERVEUR_DEFAUT;
        strcpy(config.nom_utilisateur, "anonyme");
    }
    
    printf("Lecture du fichier de configuration OK, utilisateur %s\n\n", 
           config.nom_utilisateur);
    
    /* Création du socket UDP */
    socket_client = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_client < 0) {
        perror("Erreur création socket");
        return EXIT_FAILURE;
    }
    
    /* Configuration de l'adresse du serveur */
    memset(&adresse_serveur, 0, sizeof(struct sockaddr_in));
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(config.port_serveur);
    adresse_serveur.sin_addr.s_addr = inet_addr(config.ip_serveur);
    
    /* Installation du gestionnaire de signal */
    signal(SIGINT, gestionnaire_sigint);
    
    /* Boucle principale du menu */
    while (!arret_demande) {
        afficher_menu();
        printf("Choix :\n>");
        
        if (scanf("%d", &choix) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        
        switch (choix) {
            case 0:
                creer_groupe();
                break;
            case 1:
                rejoindre_groupe();
                break;
            case 2:
                lister_groupes();
                break;
            case 3:
                dialoguer_groupe();
                break;
            case 4:
                printf("Fin du programme\n");
                arret_demande = 1;
                break;
            default:
                printf("Choix invalide\n");
                break;
        }
    }
    
    /* Nettoyage */
    if (socket_client >= 0) {
        close(socket_client);
    }
    
    return EXIT_SUCCESS;
}

/*============================================================================*
 * IMPLÉMENTATION DES FONCTIONS
 *============================================================================*/

int lire_configuration(struct_config_client *conf) {
    FILE *fichier;
    char ligne[256];
    
    fichier = fopen("client_config.txt", "r");
    if (fichier == NULL) {
        return -1;
    }
    
    strcpy(conf->ip_serveur, "127.0.0.1");
    conf->port_serveur = PORT_SERVEUR_DEFAUT;
    strcpy(conf->nom_utilisateur, "anonyme");
    
    while (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        if (ligne[0] == '#' || ligne[0] == '\n') {
            continue;
        }
        
        if (strncmp(ligne, "NOM=", 4) == 0) {
            sscanf(ligne + 4, "%s", conf->nom_utilisateur);
        } else if (strncmp(ligne, "IP_SERVEUR=", 11) == 0) {
            sscanf(ligne + 11, "%s", conf->ip_serveur);
        } else if (strncmp(ligne, "PORT_SERVEUR=", 13) == 0) {
            sscanf(ligne + 13, "%d", &conf->port_serveur);
        }
    }
    
    fclose(fichier);
    return 0;
}

void afficher_menu() {
    printf("\nChoix des commandes :\n");
    printf("0 Creation de groupe\n");
    printf("1 Rejoindre un groupe\n");
    printf("2 Lister les groupes\n");
    printf("3 Dialoguer sur un groupe\n");
    printf("4 Quitter\n");
}

void creer_groupe() {
    struct_message msg, reponse;
    char nom_groupe[50];
    
    printf("Saisir le nom du groupe\n");
    if (fgets(nom_groupe, sizeof(nom_groupe), stdin) == NULL) {
        return;
    }
    nom_groupe[strcspn(nom_groupe, "\n")] = 0;
    
    /* Préparation du message */
    memset(&msg, 0, sizeof(struct_message));
    strncpy(msg.Ordre, ORDRE_CRG, TAILLE_ORDRE - 1);
    strncpy(msg.Emetteur, config.nom_utilisateur, TAILLE_EMETTEUR - 1);
    strncpy(msg.Texte, nom_groupe, TAILLE_TEXTE - 1);
    
    printf("Envoi de la demande au serveur\n");
    if (envoyer_serveur(&msg) < 0) {
        printf("Erreur envoi\n");
        return;
    }
    
    /* Réception de la réponse */
    if (recevoir_serveur(&reponse) < 0) {
        printf("Erreur réception\n");
        return;
    }
    
    if (strncmp(reponse.Ordre, ORDRE_ACK, 3) == 0) {
        printf("Groupe cree !\n");
    } else {
        printf("Erreur : %s\n", reponse.Texte);
    }
}

void rejoindre_groupe() {
    struct_message msg, reponse;
    char nom_groupe[50];
    int port_groupe;
    char moderateur_groupe[TAILLE_EMETTEUR];
    
    printf("Saisir le nom du groupe\n");
    if (fgets(nom_groupe, sizeof(nom_groupe), stdin) == NULL) {
        return;
    }
    nom_groupe[strcspn(nom_groupe, "\n")] = 0;
    
    /* Demande au serveur */
    memset(&msg, 0, sizeof(struct_message));
    strncpy(msg.Ordre, ORDRE_JGR, TAILLE_ORDRE - 1);
    strncpy(msg.Emetteur, config.nom_utilisateur, TAILLE_EMETTEUR - 1);
    strncpy(msg.Texte, nom_groupe, TAILLE_TEXTE - 1);
    
    if (envoyer_serveur(&msg) < 0 || recevoir_serveur(&reponse) < 0) {
        printf("Erreur communication serveur\n");
        return;
    }
    
    if (strncmp(reponse.Ordre, ORDRE_ERR, 3) == 0) {
        printf("Erreur : %s\n", reponse.Texte);
        return;
    }
    
    /* Parser la réponse : PORT:xxxx,MOD:yyyy */
    if (sscanf(reponse.Texte, "PORT:%d,MOD:%s", &port_groupe, moderateur_groupe) != 2) {
        printf("Erreur parsing réponse\n");
        return;
    }
    
    printf("Connexion au groupe %s realisee, lancement de l affichage\n", nom_groupe);
    
    /* Lancer AffichageISY */
    pid_t pid = fork();
    if (pid == 0) {
        char port_str[10];
        sprintf(port_str, "%d", port_groupe);
        execl("./AffichageISY", "AffichageISY", nom_groupe, port_str, 
              config.nom_utilisateur, NULL);
        perror("Erreur execl AffichageISY");
        exit(EXIT_FAILURE);
    }
    
    /* Enregistrer le groupe */
    if (nombre_groupes_rejoints < MAX_GROUPES) {
        strncpy(groupes_rejoints[nombre_groupes_rejoints].nom, nom_groupe, 49);
        groupes_rejoints[nombre_groupes_rejoints].port = port_groupe;
        strncpy(groupes_rejoints[nombre_groupes_rejoints].moderateur, 
                moderateur_groupe, TAILLE_EMETTEUR - 1);
        groupes_rejoints[nombre_groupes_rejoints].pid_affichage = pid;
        groupes_rejoints[nombre_groupes_rejoints].actif = 1;
        nombre_groupes_rejoints++;
    }
}

void lister_groupes() {
    struct_message msg, reponse;
    
    memset(&msg, 0, sizeof(struct_message));
    strncpy(msg.Ordre, ORDRE_LST, TAILLE_ORDRE - 1);
    strncpy(msg.Emetteur, config.nom_utilisateur, TAILLE_EMETTEUR - 1);
    
    if (envoyer_serveur(&msg) < 0 || recevoir_serveur(&reponse) < 0) {
        printf("Erreur communication serveur\n");
        return;
    }
    
    printf("%s\n", reponse.Texte);
}

void dialoguer_groupe() {
    printf("Fonction dialoguer_groupe non implémentée dans cette version simplifiée\n");
    printf("Utilisez rejoindre_groupe pour lancer l'affichage\n");
}

int envoyer_serveur(struct_message *msg) {
    ssize_t octets;
    
    octets = sendto(socket_client, msg, sizeof(struct_message), 0,
                    (struct sockaddr *)&adresse_serveur, sizeof(struct sockaddr_in));
    
    if (octets < 0) {
        perror("Erreur sendto");
        return -1;
    }
    
    return 0;
}

int recevoir_serveur(struct_message *msg) {
    socklen_t taille = sizeof(struct sockaddr_in);
    ssize_t octets;
    
    octets = recvfrom(socket_client, msg, sizeof(struct_message), 0,
                      (struct sockaddr *)&adresse_serveur, &taille);
    
    if (octets < 0) {
        perror("Erreur recvfrom");
        return -1;
    }
    
    return 0;
}

void gestionnaire_sigint(int sig) {
    (void)sig;
    arret_demande = 1;
    
    /* Terminer tous les processus d'affichage */
    for (int i = 0; i < nombre_groupes_rejoints; i++) {
        if (groupes_rejoints[i].actif && groupes_rejoints[i].pid_affichage > 0) {
            kill(groupes_rejoints[i].pid_affichage, SIGTERM);
        }
    }
}
