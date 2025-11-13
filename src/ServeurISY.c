/*============================================================================*
 * ISY MESSAGERIE - ServeurISY.c
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Serveur principal de gestion des groupes de messagerie
 *                Gestion des connexions clientes et création des processus
 *                GroupeISY via fork() et communication par SHM
 *============================================================================*/

#include "ServeurISY.h"

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
struct_groupe groupes[MAX_GROUPES];
int nombre_groupes = 0;
int socket_serveur = -1;
int arret_demande = 0;

/*============================================================================*
 * FONCTION MAIN
 *============================================================================*/
int main(int argc, char *argv[]) {
    struct_config config;
    struct sockaddr_in adresse_serveur, adresse_client;
    struct_message msg_recu;
    socklen_t taille_adresse;
    ssize_t octets_recus;
    
    printf("=======================================================\n");
    printf("          ISY MESSAGERIE - SERVEUR PRINCIPAL          \n");
    printf("=======================================================\n\n");
    
    // Lecture du fichier de configuration
    if (lire_configuration(&config) < 0) {
        fprintf(stderr, "Erreur: Impossible de lire le fichier de configuration\n");
        fprintf(stderr, "Utilisation des valeurs par défaut\n");
        strcpy(config.ip_serveur, "127.0.0.1");
        config.port_serveur = PORT_SERVEUR_DEFAUT;
    }
    printf("Lecture du fichier de configuration OK\n");
    printf("IP Serveur: %s\n", config.ip_serveur);
    printf("Port Serveur: %d\n\n", config.port_serveur);
    
    // Installation des gestionnaires de signaux
    signal(SIGINT, gestionnaire_sigint);
    signal(SIGCHLD, gestionnaire_sigchld);
    
    // Initialisation du serveur UDP
    if (initialiser_serveur(&config) < 0) {
        fprintf(stderr, "Erreur: Impossible d'initialiser le serveur\n");
        return EXIT_FAILURE;
    }
    
    printf("Serveur initialisé sur le port %d\n", config.port_serveur);
    printf("En attente de connexions...\n\n");
    
    // Boucle principale du serveur
    while (!arret_demande) {
        memset(&msg_recu, 0, sizeof(struct_message));
        memset(&adresse_client, 0, sizeof(struct sockaddr_in));
        taille_adresse = sizeof(struct sockaddr_in);
        
        // Réception d'un message UDP (bloquant)
        octets_recus = recvfrom(socket_serveur, &msg_recu, sizeof(struct_message), 
                                0, (struct sockaddr *)&adresse_client, &taille_adresse);
        
        if (octets_recus < 0) {
            if (errno == EINTR) {
                // Interruption par un signal, on continue
                continue;
            }
            perror("Erreur recvfrom");
            continue;
        }
        
        if (octets_recus > 0) {
            // Traitement du message reçu
            traiter_message(&msg_recu, &adresse_client);
        }
    }
    
    // Arrêt propre du serveur
    printf("\n\nArrêt du serveur en cours...\n");
    arreter_serveur();
    
    printf("Serveur arrêté proprement.\n");
    return EXIT_SUCCESS;
}

/*============================================================================*
 * IMPLÉMENTATION DES FONCTIONS
 *============================================================================*/

/**
 * Lecture du fichier de configuration
 */
int lire_configuration(struct_config *config) {
    FILE *fichier;
    char ligne[256];
    
    fichier = fopen(CONFIG_FILE, "r");
    if (fichier == NULL) {
        return -1;
    }
    
    // Valeurs par défaut
    strcpy(config->ip_serveur, "127.0.0.1");
    config->port_serveur = PORT_SERVEUR_DEFAUT;
    
    // Lecture ligne par ligne
    while (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        // Ignorer les commentaires et lignes vides
        if (ligne[0] == '#' || ligne[0] == '\n') {
            continue;
        }
        
        // Parser les paramètres
        if (strncmp(ligne, "IP=", 3) == 0) {
            sscanf(ligne + 3, "%s", config->ip_serveur);
        } else if (strncmp(ligne, "PORT=", 5) == 0) {
            sscanf(ligne + 5, "%d", &config->port_serveur);
        }
    }
    
    fclose(fichier);
    return 0;
}

/**
 * Initialisation du serveur UDP
 */
int initialiser_serveur(struct_config *config) {
    struct sockaddr_in adresse_serveur;
    
    // Création du socket UDP
    socket_serveur = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_serveur < 0) {
        perror("Erreur création socket");
        return -1;
    }
    
    // Configuration de l'adresse du serveur
    memset(&adresse_serveur, 0, sizeof(struct sockaddr_in));
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(config->port_serveur);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Liaison du socket à l'adresse et au port
    if (bind(socket_serveur, (struct sockaddr *)&adresse_serveur, 
             sizeof(struct sockaddr_in)) < 0) {
        perror("Erreur bind");
        close(socket_serveur);
        return -1;
    }
    
    // Initialisation du tableau de groupes
    for (int i = 0; i < MAX_GROUPES; i++) {
        groupes[i].actif = 0;
        groupes[i].pid_processus = 0;
        groupes[i].shmid = -1;
    }
    
    return 0;
}

/**
 * Gestionnaire du signal SIGINT (CTRL-C)
 */
void gestionnaire_sigint(int sig) {
    arret_demande = 1;
}

/**
 * Gestionnaire du signal SIGCHLD (terminaison d'un processus fils)
 */
void gestionnaire_sigchld(int sig) {
    pid_t pid;
    int status;
    
    // Récupération de tous les processus fils terminés
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Recherche du groupe correspondant
        for (int i = 0; i < nombre_groupes; i++) {
            if (groupes[i].pid_processus == pid) {
                printf("Groupe '%s' terminé (PID: %d)\n", groupes[i].nom, pid);
                groupes[i].actif = 0;
                groupes[i].pid_processus = 0;
                break;
            }
        }
    }
}

/**
 * Recherche d'un groupe par son nom
 */
int trouver_groupe(const char *nom_groupe) {
    for (int i = 0; i < nombre_groupes; i++) {
        if (groupes[i].actif && strcmp(groupes[i].nom, nom_groupe) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Création d'un nouveau groupe
 */
int creer_groupe(const char *nom_groupe, const char *moderateur, int *port_groupe) {
    key_t cle_shm;
    int shmid;
    struct_shm_groupe *shm;
    pid_t pid;
    char port_str[10];
    int index_groupe;
    
    // Vérifier si le groupe existe déjà
    if (trouver_groupe(nom_groupe) >= 0) {
        fprintf(stderr, "Erreur: Le groupe '%s' existe déjà\n", nom_groupe);
        return -1;
    }
    
    // Vérifier le nombre maximum de groupes
    if (nombre_groupes >= MAX_GROUPES) {
        fprintf(stderr, "Erreur: Nombre maximum de groupes atteint\n");
        return -1;
    }
    
    // Recherche d'un slot libre
    index_groupe = -1;
    for (int i = 0; i < MAX_GROUPES; i++) {
        if (!groupes[i].actif) {
            index_groupe = i;
            break;
        }
    }
    
    if (index_groupe < 0) {
        return -1;
    }
    
    // Calcul du port pour le groupe
    *port_groupe = PORT_GROUPE_BASE + index_groupe;
    
    // Création de la clé IPC pour la mémoire partagée
    cle_shm = ftok(".", 'A' + index_groupe);
    if (cle_shm == -1) {
        perror("Erreur ftok");
        return -1;
    }
    
    // Création du segment de mémoire partagée
    shmid = shmget(cle_shm, sizeof(struct_shm_groupe), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur shmget");
        return -1;
    }
    
    // Attachement au segment de mémoire partagée
    shm = (struct_shm_groupe *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("Erreur shmat");
        shmctl(shmid, IPC_RMID, NULL);
        return -1;
    }
    
    // Initialisation de la mémoire partagée
    memset(shm, 0, sizeof(struct_shm_groupe));
    shm->actif = 1;
    shm->nombre_membres = 0;
    strncpy(shm->moderateur, moderateur, TAILLE_EMETTEUR - 1);
    shm->flag_commande = 0;
    
    // Détachement de la mémoire partagée
    shmdt(shm);
    
    // Fork pour créer le processus GroupeISY
    pid = fork();
    
    if (pid < 0) {
        perror("Erreur fork");
        shmctl(shmid, IPC_RMID, NULL);
        return -1;
    }
    
    if (pid == 0) {
        // Processus fils - Exécution de GroupeISY
        sprintf(port_str, "%d", *port_groupe);
        
        // Exécution du programme GroupeISY
        execl("./GroupeISY", "GroupeISY", nom_groupe, moderateur, port_str, NULL);
        
        // Si execl échoue
        perror("Erreur execl GroupeISY");
        exit(EXIT_FAILURE);
    }
    
    // Processus père - Enregistrement du groupe
    strncpy(groupes[index_groupe].nom, nom_groupe, 49);
    strncpy(groupes[index_groupe].moderateur, moderateur, TAILLE_EMETTEUR - 1);
    groupes[index_groupe].port = *port_groupe;
    groupes[index_groupe].pid_processus = pid;
    groupes[index_groupe].actif = 1;
    groupes[index_groupe].nombre_membres = 0;
    groupes[index_groupe].shmid = shmid;
    
    nombre_groupes++;
    
    printf("Groupe '%s' créé (Port: %d, PID: %d, Modérateur: %s)\n", 
           nom_groupe, *port_groupe, pid, moderateur);
    
    return index_groupe;
}

/**
 * Destruction d'un groupe
 */
int detruire_groupe(const char *nom_groupe, const char *demandeur) {
    int index;
    
    index = trouver_groupe(nom_groupe);
    if (index < 0) {
        fprintf(stderr, "Erreur: Groupe '%s' introuvable\n", nom_groupe);
        return -1;
    }
    
    // Vérifier que le demandeur est le modérateur
    if (strcmp(groupes[index].moderateur, demandeur) != 0) {
        fprintf(stderr, "Erreur: Seul le modérateur peut détruire le groupe\n");
        return -1;
    }
    
    // Envoi du signal SIGTERM au processus GroupeISY
    if (groupes[index].pid_processus > 0) {
        kill(groupes[index].pid_processus, SIGTERM);
    }
    
    // Suppression de la mémoire partagée
    if (groupes[index].shmid >= 0) {
        shmctl(groupes[index].shmid, IPC_RMID, NULL);
    }
    
    // Marquer le groupe comme inactif
    groupes[index].actif = 0;
    groupes[index].pid_processus = 0;
    
    printf("Groupe '%s' détruit par %s\n", nom_groupe, demandeur);
    
    return 0;
}

/**
 * Traitement d'une demande de création de groupe
 */
void traiter_creation_groupe(struct_message *msg, struct sockaddr_in *adresse_client) {
    struct_message reponse;
    int port_groupe;
    
    printf("%s : Réception d'une demande de création de groupe '%s'\n", 
           msg->Emetteur, msg->Texte);
    
    // Tentative de création du groupe
    if (creer_groupe(msg->Texte, msg->Emetteur, &port_groupe) >= 0) {
        // Envoi de l'accusé de réception
        memset(&reponse, 0, sizeof(struct_message));
        strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
        strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
        snprintf(reponse.Texte, TAILLE_TEXTE, "Groupe %s créé sur port %d", 
                 msg->Texte, port_groupe);
        
        envoyer_message(&reponse, adresse_client);
        printf("Envoi %s : Groupe '%s' créé\n", msg->Emetteur, msg->Texte);
    } else {
        // Envoi d'un message d'erreur
        memset(&reponse, 0, sizeof(struct_message));
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
        snprintf(reponse.Texte, TAILLE_TEXTE, "Impossible de créer le groupe %s", 
                 msg->Texte);
        
        envoyer_message(&reponse, adresse_client);
    }
}

/**
 * Traitement d'une demande de connexion à un groupe
 */
void traiter_connexion_groupe(struct_message *msg, struct sockaddr_in *adresse_client) {
    struct_message reponse;
    int index;
    
    printf("%s : Réception d'une demande de connexion au groupe '%s'\n", 
           msg->Emetteur, msg->Texte);
    
    // Recherche du groupe
    index = trouver_groupe(msg->Texte);
    
    if (index >= 0) {
        // Envoi des informations de connexion
        memset(&reponse, 0, sizeof(struct_message));
        strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
        strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
        snprintf(reponse.Texte, TAILLE_TEXTE, "PORT:%d,MOD:%s", 
                 groupes[index].port, groupes[index].moderateur);
        
        envoyer_message(&reponse, adresse_client);
        printf("Envoi %s : Informations de connexion au groupe '%s'\n", 
               msg->Emetteur, msg->Texte);
    } else {
        // Groupe introuvable
        memset(&reponse, 0, sizeof(struct_message));
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
        snprintf(reponse.Texte, TAILLE_TEXTE, "Groupe %s introuvable", msg->Texte);
        
        envoyer_message(&reponse, adresse_client);
    }
}

/**
 * Traitement d'une demande de liste des groupes
 */
void traiter_liste_groupes(struct_message *msg, struct sockaddr_in *adresse_client) {
    struct_message reponse;
    char liste[TAILLE_TEXTE] = "";
    int count = 0;
    
    printf("%s : Réception d'une demande de listing des groupes de discussion\n", 
           msg->Emetteur);
    
    // Construction de la liste des groupes
    for (int i = 0; i < nombre_groupes; i++) {
        if (groupes[i].actif) {
            if (count > 0) {
                strncat(liste, ",", TAILLE_TEXTE - strlen(liste) - 1);
            }
            strncat(liste, groupes[i].nom, TAILLE_TEXTE - strlen(liste) - 1);
            count++;
        }
    }
    
    if (count == 0) {
        strcpy(liste, "Aucun groupe");
    }
    
    // Envoi de la liste
    memset(&reponse, 0, sizeof(struct_message));
    strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
    strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
    strncpy(reponse.Texte, liste, TAILLE_TEXTE - 1);
    
    envoyer_message(&reponse, adresse_client);
    printf("Envoi %s : Liste des groupes de discussions\n", msg->Emetteur);
}

/**
 * Traitement d'une demande de destruction de groupe
 */
void traiter_destruction_groupe(struct_message *msg, struct sockaddr_in *adresse_client) {
    struct_message reponse;
    
    printf("%s : Réception d'une demande de destruction du groupe '%s'\n", 
           msg->Emetteur, msg->Texte);
    
    // Tentative de destruction du groupe
    if (detruire_groupe(msg->Texte, msg->Emetteur) == 0) {
        // Envoi de l'accusé de réception
        memset(&reponse, 0, sizeof(struct_message));
        strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
        strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
        snprintf(reponse.Texte, TAILLE_TEXTE, "Groupe %s détruit", msg->Texte);
        
        envoyer_message(&reponse, adresse_client);
    } else {
        // Envoi d'un message d'erreur
        memset(&reponse, 0, sizeof(struct_message));
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
        snprintf(reponse.Texte, TAILLE_TEXTE, "Impossible de détruire le groupe %s", 
                 msg->Texte);
        
        envoyer_message(&reponse, adresse_client);
    }
}

/**
 * Traitement d'une demande de déconnexion
 */
void traiter_deconnexion(struct_message *msg, struct sockaddr_in *adresse_client) {
    struct_message reponse;
    
    printf("%s : Réception d'une demande de déconnexion\n", msg->Emetteur);
    
    // Envoi de l'accusé de réception
    memset(&reponse, 0, sizeof(struct_message));
    strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
    strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
    strcpy(reponse.Texte, "Déconnexion OK");
    
    envoyer_message(&reponse, adresse_client);
}

/**
 * Traitement des messages reçus
 */
void traiter_message(struct_message *msg, struct sockaddr_in *adresse_client) {
    // Vérifier l'ordre du message
    if (strncmp(msg->Ordre, ORDRE_CRG, 3) == 0) {
        traiter_creation_groupe(msg, adresse_client);
    } 
    else if (strncmp(msg->Ordre, ORDRE_JGR, 3) == 0) {
        traiter_connexion_groupe(msg, adresse_client);
    } 
    else if (strncmp(msg->Ordre, ORDRE_LST, 3) == 0) {
        traiter_liste_groupes(msg, adresse_client);
    } 
    else if (strncmp(msg->Ordre, ORDRE_DGR, 3) == 0) {
        traiter_destruction_groupe(msg, adresse_client);
    } 
    else if (strncmp(msg->Ordre, ORDRE_DEC, 3) == 0) {
        traiter_deconnexion(msg, adresse_client);
    } 
    else {
        printf("Ordre inconnu reçu: %s\n", msg->Ordre);
    }
}

/**
 * Envoi d'un message à un client via UDP
 */
int envoyer_message(struct_message *msg, struct sockaddr_in *adresse_client) {
    ssize_t octets_envoyes;
    
    octets_envoyes = sendto(socket_serveur, msg, sizeof(struct_message), 0,
                           (struct sockaddr *)adresse_client, 
                           sizeof(struct sockaddr_in));
    
    if (octets_envoyes < 0) {
        perror("Erreur sendto");
        return -1;
    }
    
    return 0;
}

/**
 * Terminaison propre de tous les groupes
 */
void terminer_tous_groupes() {
    printf("Terminaison de tous les groupes de discussion...\n");
    
    for (int i = 0; i < nombre_groupes; i++) {
        if (groupes[i].actif && groupes[i].pid_processus > 0) {
            printf("Arrêt du groupe '%s' (PID: %d)...\n", 
                   groupes[i].nom, groupes[i].pid_processus);
            
            // Envoi du signal SIGTERM
            kill(groupes[i].pid_processus, SIGTERM);
            
            // Attente de la terminaison
            waitpid(groupes[i].pid_processus, NULL, 0);
            
            // Suppression de la mémoire partagée
            if (groupes[i].shmid >= 0) {
                shmctl(groupes[i].shmid, IPC_RMID, NULL);
            }
        }
    }
}

/**
 * Nettoyage et fermeture propre du serveur
 */
void arreter_serveur() {
    // Terminaison de tous les groupes
    terminer_tous_groupes();
    
    // Fermeture du socket
    if (socket_serveur >= 0) {
        close(socket_serveur);
    }
}

/**
 * Affichage de l'état du serveur
 */
void afficher_etat_serveur() {
    printf("\n=== État du serveur ===\n");
    printf("Nombre de groupes actifs: %d\n", nombre_groupes);
    
    for (int i = 0; i < nombre_groupes; i++) {
        if (groupes[i].actif) {
            printf("  - %s (Port: %d, Modérateur: %s, PID: %d)\n",
                   groupes[i].nom, groupes[i].port, 
                   groupes[i].moderateur, groupes[i].pid_processus);
        }
    }
    printf("=======================\n\n");
}
