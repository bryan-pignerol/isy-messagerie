/*============================================================================*
 * ISY MESSAGERIE - ServeurISY.c
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Serveur principal pour la gestion des groupes de discussion
 *============================================================================*/

#include "ServeurISY.h"

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
int g_socket_serveur = -1;
int g_shmid = -1;
MemoirePartagee *g_shm = NULL;
int g_continuer = 1;

/*============================================================================*
 * FONCTION : gestionnaire_sigint
 * DESCRIPTION : Gere le signal SIGINT (CTRL-C) pour terminaison propre
 *============================================================================*/
void gestionnaire_sigint(int sig)
{
    (void)sig;
    printf("\n%sReception du signal SIGINT, arret du serveur...%s\n", 
           COULEUR_JAUNE, COULEUR_RESET);
    g_continuer = 0;
}

/*============================================================================*
 * FONCTION : lire_configuration
 * DESCRIPTION : Lit le fichier de configuration du serveur
 *============================================================================*/
int lire_configuration(const char *fichier, ConfigServeur *config)
{
    FILE *f;
    char ligne[256];
    
    f = fopen(fichier, "r");
    if (f == NULL)
    {
        perror("Erreur ouverture fichier configuration");
        return -1;
    }
    
    while (fgets(ligne, sizeof(ligne), f) != NULL)
    {
        if (ligne[0] == '#' || ligne[0] == '\n')
            continue;
            
        if (strncmp(ligne, "IP=", 3) == 0)
        {
            sscanf(ligne, "IP=%s", config->ip);
        }
        else if (strncmp(ligne, "PORT=", 5) == 0)
        {
            sscanf(ligne, "PORT=%d", &config->port);
        }
        else if (strncmp(ligne, "PROTOCOLE=", 10) == 0)
        {
            sscanf(ligne, "PROTOCOLE=%s", config->protocole);
        }
    }
    
    fclose(f);
    return 0;
}

/*============================================================================*
 * FONCTION : initialiser_socket_serveur
 * DESCRIPTION : Cree et configure le socket UDP du serveur
 *============================================================================*/
int initialiser_socket_serveur(ConfigServeur *config)
{
    int sockfd;
    struct sockaddr_in servaddr;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Erreur creation socket");
        return -1;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config->port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Erreur bind");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

/*============================================================================*
 * FONCTION : initialiser_memoire_partagee
 * DESCRIPTION : Cree et initialise la memoire partagee
 *============================================================================*/
int initialiser_memoire_partagee(void)
{
    key_t cle;
    
    cle = ftok("serveur_config.txt", '1');
    if (cle == -1)
    {
        perror("Erreur ftok");
        return -1;
    }
    
    g_shmid = shmget(cle, sizeof(MemoirePartagee), IPC_CREAT | 0666);
    if (g_shmid == -1)
    {
        perror("Erreur shmget");
        return -1;
    }
    
    g_shm = (MemoirePartagee *)shmat(g_shmid, NULL, 0);
    if (g_shm == (void *)-1)
    {
        perror("Erreur shmat");
        return -1;
    }
    
    memset(g_shm, 0, sizeof(MemoirePartagee));
    g_shm->nb_groupes = 0;
    
    return 0;
}

/*============================================================================*
 * FONCTION : creer_groupe
 * DESCRIPTION : Cree un nouveau groupe de discussion
 *============================================================================*/
int creer_groupe(const char *nom, const char *mot_passe, const char *moderateur)
{
    int i;
    pid_t pid;
    
    if (g_shm->nb_groupes >= MAX_GROUPES)
    {
        return -1;
    }
    
    for (i = 0; i < g_shm->nb_groupes; i++)
    {
        if (strcmp(g_shm->groupes[i].nom, nom) == 0)
        {
            return -2;
        }
    }
    
    i = g_shm->nb_groupes;
    strncpy(g_shm->groupes[i].nom, nom, TAILLE_NOM_GROUPE - 1);
    strncpy(g_shm->groupes[i].mot_passe, mot_passe, TAILLE_MOT_PASSE - 1);
    strncpy(g_shm->groupes[i].moderateur, moderateur, TAILLE_EMETTEUR - 1);
    g_shm->groupes[i].port = PORT_GROUPE_BASE + i + 1;
    g_shm->groupes[i].nb_membres = 0;
    g_shm->groupes[i].nb_messages = 0;
    g_shm->groupes[i].actif = 1;
    
    pid = fork();
    if (pid == -1)
    {
        perror("Erreur fork");
        return -1;
    }
    else if (pid == 0)
    {
        char port_str[10];
        sprintf(port_str, "%d", g_shm->groupes[i].port);
        execl("./GroupeISY", "GroupeISY", nom, port_str, moderateur, NULL);
        perror("Erreur execl");
        exit(1);
    }
    
    g_shm->groupes[i].pid_groupe = pid;
    g_shm->nb_groupes++;
    
    return i;
}

/*============================================================================*
 * FONCTION : trouver_groupe
 * DESCRIPTION : Trouve l'index d'un groupe par son nom
 *============================================================================*/
int trouver_groupe(const char *nom)
{
    int i;
    
    for (i = 0; i < g_shm->nb_groupes; i++)
    {
        if (strcmp(g_shm->groupes[i].nom, nom) == 0 && g_shm->groupes[i].actif)
        {
            return i;
        }
    }
    
    return -1;
}

/*============================================================================*
 * FONCTION : lister_groupes
 * DESCRIPTION : Cree une liste des groupes actifs
 *============================================================================*/
void lister_groupes(char *buffer)
{
    int i;
    
    buffer[0] = '\0';
    
    if (g_shm->nb_groupes == 0)
    {
        strcpy(buffer, "Aucun groupe");
        return;
    }
    
    for (i = 0; i < g_shm->nb_groupes; i++)
    {
        if (g_shm->groupes[i].actif)
        {
            char ligne[100];
            sprintf(ligne, "- %s (Moderateur: %s, Membres: %d)\n", 
                    g_shm->groupes[i].nom,
                    g_shm->groupes[i].moderateur,
                    g_shm->groupes[i].nb_membres);
            strcat(buffer, ligne);
        }
    }
}

/*============================================================================*
 * FONCTION : traiter_demande_creation
 * DESCRIPTION : Traite une demande de creation de groupe
 *============================================================================*/
void traiter_demande_creation(struct struct_message *msg, struct sockaddr_in *addr_client)
{
    char nom_groupe[TAILLE_NOM_GROUPE];
    char mot_passe[TAILLE_MOT_PASSE];
    struct struct_message reponse;
    int resultat;
    
    sscanf(msg->Texte, "%s %s", nom_groupe, mot_passe);
    
    printf("%s%s : Reception d une demande de creation de groupe %s%s\n",
           COULEUR_VERT, msg->Emetteur, nom_groupe, COULEUR_RESET);
    
    resultat = creer_groupe(nom_groupe, mot_passe, msg->Emetteur);
    
    memset(&reponse, 0, sizeof(reponse));
    strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
    strncpy(reponse.Emetteur, "ServeurISY", TAILLE_EMETTEUR - 1);
    
    if (resultat >= 0)
    {
        sprintf(reponse.Texte, "Groupe %s cree sur le port %d", 
                nom_groupe, g_shm->groupes[resultat].port);
        printf("%sGroupe %s cree%s\n", COULEUR_VERT, nom_groupe, COULEUR_RESET);
    }
    else if (resultat == -2)
    {
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        sprintf(reponse.Texte, "Le groupe %s existe deja", nom_groupe);
    }
    else
    {
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        strcpy(reponse.Texte, "Erreur creation groupe");
    }
    
    sendto(g_socket_serveur, &reponse, sizeof(reponse), 0,
           (struct sockaddr *)addr_client, sizeof(*addr_client));
}

/*============================================================================*
 * FONCTION : traiter_demande_connexion
 * DESCRIPTION : Traite une demande de connexion a un groupe
 *============================================================================*/
void traiter_demande_connexion(struct struct_message *msg, struct sockaddr_in *addr_client)
{
    char nom_groupe[TAILLE_NOM_GROUPE];
    char mot_passe[TAILLE_MOT_PASSE];
    struct struct_message reponse;
    int idx_groupe;
    
    sscanf(msg->Texte, "%s %s", nom_groupe, mot_passe);
    
    printf("%s%s : Reception d une demande de connexion au groupe %s%s\n",
           COULEUR_CYAN, msg->Emetteur, nom_groupe, COULEUR_RESET);
    
    idx_groupe = trouver_groupe(nom_groupe);
    
    memset(&reponse, 0, sizeof(reponse));
    strncpy(reponse.Emetteur, "ServeurISY", TAILLE_EMETTEUR - 1);
    
    if (idx_groupe == -1)
    {
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        sprintf(reponse.Texte, "Le groupe %s n existe pas", nom_groupe);
    }
    else if (strcmp(g_shm->groupes[idx_groupe].mot_passe, mot_passe) != 0)
    {
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        strcpy(reponse.Texte, "Mot de passe incorrect");
    }
    else
    {
        strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
        sprintf(reponse.Texte, "PORT=%d MODERATEUR=%s", 
                g_shm->groupes[idx_groupe].port,
                g_shm->groupes[idx_groupe].moderateur);
        printf("%sEnvoi %s : Informations de connexion au groupe%s\n",
               COULEUR_VERT, msg->Emetteur, COULEUR_RESET);
    }
    
    sendto(g_socket_serveur, &reponse, sizeof(reponse), 0,
           (struct sockaddr *)addr_client, sizeof(*addr_client));
}

/*============================================================================*
 * FONCTION : traiter_demande_liste
 * DESCRIPTION : Traite une demande de liste des groupes
 *============================================================================*/
void traiter_demande_liste(struct struct_message *msg, struct sockaddr_in *addr_client)
{
    struct struct_message reponse;
    char liste[TAILLE_TEXTE];
    
    printf("%s%s : Reception d une demande de listing des groupes de discussion%s\n",
           COULEUR_CYAN, msg->Emetteur, COULEUR_RESET);
    
    lister_groupes(liste);
    
    memset(&reponse, 0, sizeof(reponse));
    strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
    strncpy(reponse.Emetteur, "ServeurISY", TAILLE_EMETTEUR - 1);
    strncpy(reponse.Texte, liste, TAILLE_TEXTE - 1);
    
    sendto(g_socket_serveur, &reponse, sizeof(reponse), 0,
           (struct sockaddr *)addr_client, sizeof(*addr_client));
    
    printf("%sEnvoi %s : liste des groupes de discussions%s\n",
           COULEUR_VERT, msg->Emetteur, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : terminer_proprement
 * DESCRIPTION : Termine tous les processus fils et libere les ressources
 *============================================================================*/
void terminer_proprement(void)
{
    int i;
    int status;
    
    printf("\n%sTerminaison de tous les groupes...%s\n", 
           COULEUR_JAUNE, COULEUR_RESET);
    
    for (i = 0; i < g_shm->nb_groupes; i++)
    {
        if (g_shm->groupes[i].actif && g_shm->groupes[i].pid_groupe > 0)
        {
            printf("Arret du groupe %s (PID: %d)\n", 
                   g_shm->groupes[i].nom, g_shm->groupes[i].pid_groupe);
            kill(g_shm->groupes[i].pid_groupe, SIGINT);
        }
    }
    
    for (i = 0; i < g_shm->nb_groupes; i++)
    {
        if (g_shm->groupes[i].actif && g_shm->groupes[i].pid_groupe > 0)
        {
            waitpid(g_shm->groupes[i].pid_groupe, &status, 0);
            printf("Groupe %s termine\n", g_shm->groupes[i].nom);
        }
    }
    
    if (g_shm != NULL)
    {
        shmdt(g_shm);
    }
    
    if (g_shmid != -1)
    {
        shmctl(g_shmid, IPC_RMID, NULL);
    }
    
    if (g_socket_serveur != -1)
    {
        close(g_socket_serveur);
    }
    
    printf("%sServeur arrete proprement%s\n", COULEUR_VERT, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : traiter_demande_fusion
 * DESCRIPTION : Fusionne deux groupes (seul le modérateur peut fusionner)
 *============================================================================*/
void traiter_demande_fusion(struct struct_message *msg, struct sockaddr_in *addr_client)
{
    char nom_groupe_source[TAILLE_NOM_GROUPE];
    char nom_groupe_cible[TAILLE_NOM_GROUPE];
    struct struct_message reponse;
    struct struct_message notif_transfert;
    int i, j;
    int idx_source = -1, idx_cible = -1;
    
    sscanf(msg->Texte, "%s %s", nom_groupe_source, nom_groupe_cible);
    
    /* Chercher les deux groupes */
    for (i = 0; i < g_shm->nb_groupes; i++)
    {
        if (g_shm->groupes[i].actif && strcmp(g_shm->groupes[i].nom, nom_groupe_source) == 0)
        {
            idx_source = i;
        }
        if (g_shm->groupes[i].actif && strcmp(g_shm->groupes[i].nom, nom_groupe_cible) == 0)
        {
            idx_cible = i;
        }
    }
    
    /* Vérifications */
    memset(&reponse, 0, sizeof(reponse));
    strncpy(reponse.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
    
    if (idx_source == -1)
    {
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        sprintf(reponse.Texte, "Groupe source %s introuvable", nom_groupe_source);
        sendto(g_socket_serveur, &reponse, sizeof(reponse), 0,
               (struct sockaddr *)addr_client, sizeof(*addr_client));
        return;
    }
    
    if (idx_cible == -1)
    {
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        sprintf(reponse.Texte, "Groupe cible %s introuvable", nom_groupe_cible);
        sendto(g_socket_serveur, &reponse, sizeof(reponse), 0,
               (struct sockaddr *)addr_client, sizeof(*addr_client));
        return;
    }
    
    /* Vérifier que l'émetteur est bien le modérateur du groupe source */
    if (strcmp(g_shm->groupes[idx_source].moderateur, msg->Emetteur) != 0)
    {
        strncpy(reponse.Ordre, ORDRE_ERR, TAILLE_ORDRE - 1);
        sprintf(reponse.Texte, "Seul le moderateur peut fusionner");
        sendto(g_socket_serveur, &reponse, sizeof(reponse), 0,
               (struct sockaddr *)addr_client, sizeof(*addr_client));
        return;
    }
    
    printf("%sFusion %s -> %s demandee par %s%s\n",
           COULEUR_MAGENTA, nom_groupe_source, nom_groupe_cible, 
           msg->Emetteur, COULEUR_RESET);
    
    /* Envoyer notification de transfert à tous les membres du groupe source */
    memset(&notif_transfert, 0, sizeof(notif_transfert));
    memcpy(notif_transfert.Ordre, ORDRE_MERGE, strlen(ORDRE_MERGE));
    notif_transfert.Ordre[TAILLE_ORDRE - 1] = '\0';
    strncpy(notif_transfert.Emetteur, "SERVEUR", TAILLE_EMETTEUR - 1);
    sprintf(notif_transfert.Texte, "TRANSFER %s %d", 
            nom_groupe_cible, g_shm->groupes[idx_cible].port);
    
    /* Envoyer le message de transfert au GroupeISY source pour qu'il redistribue */
    struct sockaddr_in addr_groupe_source;
    memset(&addr_groupe_source, 0, sizeof(addr_groupe_source));
    addr_groupe_source.sin_family = AF_INET;
    addr_groupe_source.sin_port = htons(g_shm->groupes[idx_source].port);
    addr_groupe_source.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    sendto(g_socket_serveur, &notif_transfert, sizeof(notif_transfert), 0,
           (struct sockaddr *)&addr_groupe_source, sizeof(addr_groupe_source));
    
    /* Attendre un peu pour que les clients reçoivent la notification */
    sleep(1);
    
    /* Détruire le groupe source */
    printf("%sDestruction du groupe source %s%s\n",
           COULEUR_JAUNE, nom_groupe_source, COULEUR_RESET);
    
    if (g_shm->groupes[idx_source].pid_groupe > 0)
    {
        kill(g_shm->groupes[idx_source].pid_groupe, SIGINT);
        waitpid(g_shm->groupes[idx_source].pid_groupe, NULL, 0);
    }
    
    g_shm->groupes[idx_source].actif = 0;
    
    /* Réorganiser le tableau si nécessaire */
    for (j = idx_source; j < g_shm->nb_groupes - 1; j++)
    {
        g_shm->groupes[j] = g_shm->groupes[j + 1];
    }
    g_shm->nb_groupes--;
    
    /* Confirmer au modérateur */
    strncpy(reponse.Ordre, ORDRE_ACK, TAILLE_ORDRE - 1);
    strcpy(reponse.Texte, "Fusion reussie");
    sendto(g_socket_serveur, &reponse, sizeof(reponse), 0,
           (struct sockaddr *)addr_client, sizeof(*addr_client));
    
    printf("%sFusion terminee avec succes%s\n", COULEUR_VERT, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : main
 * DESCRIPTION : Point d'entree du serveur ISY
 *============================================================================*/
int main(int argc, char **argv, char **envp)
{
    ConfigServeur config;
    struct struct_message msg;
    struct sockaddr_in addr_client;
    socklen_t len_addr;
    ssize_t n;
    
    (void)argc;
    (void)argv;
    (void)envp;
    
    printf("%s========================================%s\n", 
           COULEUR_CYAN, COULEUR_RESET);
    printf("%s      SERVEUR ISY MESSAGERIE          %s\n", 
           COULEUR_CYAN, COULEUR_RESET);
    printf("%s========================================%s\n\n", 
           COULEUR_CYAN, COULEUR_RESET);
    
    if (signal(SIGINT, gestionnaire_sigint) == SIG_ERR)
    {
        perror("Erreur signal");
        exit(EXIT_FAILURE);
    }
    
    if (lire_configuration("serveur_config.txt", &config) < 0)
    {
        fprintf(stderr, "Erreur lecture configuration\n");
        exit(EXIT_FAILURE);
    }
    
    printf("%sLecture du fichier de configuration OK%s\n", 
           COULEUR_VERT, COULEUR_RESET);
    printf("IP: %s, Port: %d, Protocole: %s\n\n", 
           config.ip, config.port, config.protocole);
    
    if (initialiser_memoire_partagee() < 0)
    {
        fprintf(stderr, "Erreur initialisation memoire partagee\n");
        exit(EXIT_FAILURE);
    }
    
    g_socket_serveur = initialiser_socket_serveur(&config);
    if (g_socket_serveur < 0)
    {
        terminer_proprement();
        exit(EXIT_FAILURE);
    }
    
    printf("%sServeur en ecoute sur le port %d...%s\n\n", 
           COULEUR_VERT, config.port, COULEUR_RESET);
    
    while (g_continuer)
    {
        len_addr = sizeof(addr_client);
        n = recvfrom(g_socket_serveur, &msg, sizeof(msg), 0,
                    (struct sockaddr *)&addr_client, &len_addr);
        
        if (n < 0)
        {
            if (errno == EINTR)
            {
                break;
            }
            perror("Erreur recvfrom");
            continue;
        }
        
        if (strcmp(msg.Ordre, ORDRE_CRE) == 0)
        {
            traiter_demande_creation(&msg, &addr_client);
        }
        else if (strcmp(msg.Ordre, ORDRE_CON) == 0)
        {
            traiter_demande_connexion(&msg, &addr_client);
        }
        else if (strcmp(msg.Ordre, ORDRE_LST) == 0)
        {
            traiter_demande_liste(&msg, &addr_client);
        }
        else if (strcmp(msg.Ordre, ORDRE_MERGE) == 0)
        {
            traiter_demande_fusion(&msg, &addr_client);
        }
    }
    
    terminer_proprement();
    
    exit(EXIT_SUCCESS);
}