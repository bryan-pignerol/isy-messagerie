/*============================================================================*
 * ISY MESSAGERIE - GroupeISY.c
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Gestion des discussions au sein d'un groupe de messagerie
 *============================================================================*/

#include "GroupeISY.h"

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
int socket_groupe = -1;
struct_shm_groupe *shm = NULL;
int shmid = -1;
char nom_groupe[50];
char moderateur[TAILLE_EMETTEUR];
int arret_demande = 0;

/*============================================================================*
 * FONCTION MAIN
 *============================================================================*/
int main(int argc, char *argv[]) {
    struct sockaddr_in adresse_client;
    struct_message msg_recu;
    socklen_t taille_adresse;
    ssize_t octets_recus;
    key_t cle_shm;
    int port;
    
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <nom_groupe> <moderateur> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    strncpy(nom_groupe, argv[1], 49);
    strncpy(moderateur, argv[2], TAILLE_EMETTEUR - 1);
    port = atoi(argv[3]);
    
    printf("Lancement du groupe de discussion %s, moderateur %s\n", nom_groupe, moderateur);
    
    /* Installation des gestionnaires de signaux */
    signal(SIGINT, gestionnaire_sigint);
    signal(SIGTERM, gestionnaire_sigterm);
    
    /* Attachement à la mémoire partagée */
    cle_shm = ftok(".", 'A' + (port - PORT_GROUPE_BASE));
    shmid = shmget(cle_shm, sizeof(struct_shm_groupe), 0666);
    if (shmid == -1) {
        perror("Erreur shmget");
        return EXIT_FAILURE;
    }
    
    shm = (struct_shm_groupe *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("Erreur shmat");
        return EXIT_FAILURE;
    }
    
    /* Initialisation du socket UDP */
    if (initialiser_groupe(port) < 0) {
        fprintf(stderr, "Erreur initialisation groupe\n");
        shmdt(shm);
        return EXIT_FAILURE;
    }
    
    /* Boucle principale */
    while (!arret_demande) {
        memset(&msg_recu, 0, sizeof(struct_message));
        memset(&adresse_client, 0, sizeof(struct sockaddr_in));
        taille_adresse = sizeof(struct sockaddr_in);
        
        octets_recus = recvfrom(socket_groupe, &msg_recu, sizeof(struct_message),
                                0, (struct sockaddr *)&adresse_client, &taille_adresse);
        
        if (octets_recus < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Erreur recvfrom");
            continue;
        }
        
        if (octets_recus > 0) {
            traiter_message(&msg_recu, &adresse_client);
        }
    }
    
    /* Nettoyage */
    if (socket_groupe >= 0) {
        close(socket_groupe);
    }
    if (shm != NULL) {
        shmdt(shm);
    }
    
    return EXIT_SUCCESS;
}

/*============================================================================*
 * IMPLÉMENTATION DES FONCTIONS
 *============================================================================*/

int initialiser_groupe(int port) {
    struct sockaddr_in adresse_groupe;
    
    socket_groupe = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_groupe < 0) {
        perror("Erreur création socket");
        return -1;
    }
    
    memset(&adresse_groupe, 0, sizeof(struct sockaddr_in));
    adresse_groupe.sin_family = AF_INET;
    adresse_groupe.sin_port = htons(port);
    adresse_groupe.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(socket_groupe, (struct sockaddr *)&adresse_groupe,
             sizeof(struct sockaddr_in)) < 0) {
        perror("Erreur bind");
        close(socket_groupe);
        return -1;
    }
    
    return 0;
}

void traiter_message(struct_message *msg, struct sockaddr_in *adresse_client) {
    /* Vérifier si c'est une demande de connexion au groupe */
    if (strncmp(msg->Ordre, ORDRE_CON, 3) == 0) {
        printf("Reception connexion de %s\n", msg->Emetteur);
        ajouter_membre(msg->Emetteur, adresse_client);
        return;
    }
    
    /* Vérifier si c'est un message à redistribuer */
    if (strncmp(msg->Ordre, ORDRE_MSG, 3) == 0) {
        /* Mettre à jour l'adresse du membre si nécessaire */
        int index = chercher_membre(msg->Emetteur);
        if (index >= 0) {
            shm->membres[index].adresse = *adresse_client;
        } else {
            /* Si le membre n'existe pas, l'ajouter */
            ajouter_membre(msg->Emetteur, adresse_client);
        }
        
        printf("Reception Message %s : %s\n", msg->Emetteur, msg->Texte);
        redistribuer_message(msg);
        return;
    }
    
    /* Déconnexion */
    if (strncmp(msg->Ordre, ORDRE_QGR, 3) == 0) {
        printf("Deconnexion de %s\n", msg->Emetteur);
        retirer_membre(msg->Emetteur);
        return;
    }
}

void redistribuer_message(struct_message *msg) {
    int nb_redistributions = 0;
    
    printf("Redistribution message a ");
    
    for (int i = 0; i < shm->nombre_membres; i++) {
        if (shm->membres[i].actif) {
            sendto(socket_groupe, msg, sizeof(struct_message), 0,
                   (struct sockaddr *)&shm->membres[i].adresse,
                   sizeof(struct sockaddr_in));
            
            if (nb_redistributions > 0) {
                printf(", ");
            }
            printf("%s", shm->membres[i].nom);
            nb_redistributions++;
        }
    }
    printf("\n");
}

int ajouter_membre(const char *nom, struct sockaddr_in *adresse) {
    /* Vérifier si le membre existe déjà */
    int index = chercher_membre(nom);
    if (index >= 0) {
        /* Membre déjà présent, mettre à jour l'adresse */
        shm->membres[index].adresse = *adresse;
        shm->membres[index].actif = 1;
        return index;
    }
    
    /* Chercher un slot libre */
    for (int i = 0; i < MAX_MEMBRES; i++) {
        if (!shm->membres[i].actif) {
            strncpy(shm->membres[i].nom, nom, TAILLE_EMETTEUR - 1);
            shm->membres[i].adresse = *adresse;
            shm->membres[i].actif = 1;
            shm->nombre_membres++;
            return i;
        }
    }
    
    return -1;
}

int retirer_membre(const char *nom) {
    int index = chercher_membre(nom);
    if (index >= 0) {
        shm->membres[index].actif = 0;
        shm->nombre_membres--;
        return 0;
    }
    return -1;
}

int chercher_membre(const char *nom) {
    for (int i = 0; i < MAX_MEMBRES; i++) {
        if (shm->membres[i].actif && strcmp(shm->membres[i].nom, nom) == 0) {
            return i;
        }
    }
    return -1;
}

void gestionnaire_sigint(int sig) {
    (void)sig;
    arret_demande = 1;
}

void gestionnaire_sigterm(int sig) {
    (void)sig;
    arret_demande = 1;
}
