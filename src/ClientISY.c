/*============================================================================*
 * ISY MESSAGERIE - ClientISY.c
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Interface client pour la gestion des commandes utilisateur
 *============================================================================*/

#include <fcntl.h>
#include <sys/stat.h>
#include "../inc/ClientISY.h"

/*============================================================================*
 * PROTOTYPES DES FONCTIONS INTERNES
 *============================================================================*/
void dialoguer_groupe(int socket_fd, ConfigClient *config);

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
int g_socket_client = -1;
int g_continuer_client = 1;
ConfigClient g_config;
struct sockaddr_in g_addr_serveur;
pid_t g_pid_affichage = -1;
int g_dans_groupe = 0;
char g_nom_groupe_actuel[TAILLE_NOM_GROUPE];
int g_port_groupe_actuel = 0;
struct sockaddr_in g_addr_groupe;

/*============================================================================*
 * FONCTION : gestionnaire_sigint_client
 * DESCRIPTION : Gere le signal SIGINT pour le client
 *============================================================================*/
void gestionnaire_sigint_client(int sig)
{
    (void)sig;
    printf("\n%sReception SIGINT, arret du client...%s\n",
           COULEUR_JAUNE, COULEUR_RESET);
    g_continuer_client = 0;
}

/*============================================================================*
 * FONCTION : lire_configuration_client
 * DESCRIPTION : Lit le fichier de configuration du client
 *============================================================================*/
int lire_configuration_client(const char *fichier, ConfigClient *config)
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
            
        if (strncmp(ligne, "NOM=", 4) == 0)
        {
            sscanf(ligne, "NOM=%s", config->nom_utilisateur);
        }
        else if (strncmp(ligne, "IP_SERVEUR=", 11) == 0)
        {
            sscanf(ligne, "IP_SERVEUR=%s", config->ip_serveur);
        }
        else if (strncmp(ligne, "PORT_SERVEUR=", 13) == 0)
        {
            sscanf(ligne, "PORT_SERVEUR=%d", &config->port_serveur);
        }
    }
    
    fclose(f);
    return 0;
}

/*============================================================================*
 * FONCTION : initialiser_socket_client
 * DESCRIPTION : Cree le socket UDP du client
 *============================================================================*/
int initialiser_socket_client(void)
{
    int sockfd;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Erreur creation socket client");
        return -1;
    }
    
    return sockfd;
}

/*============================================================================*
 * FONCTION : afficher_menu
 * DESCRIPTION : Affiche le menu principal du client
 *============================================================================*/
void afficher_menu(void)
{
    printf("\n%s========================================%s\n",
           COULEUR_CYAN, COULEUR_RESET);
    printf("%sChoix des commandes :%s\n", COULEUR_CYAN, COULEUR_RESET);
    printf("%s========================================%s\n",
           COULEUR_CYAN, COULEUR_RESET);
    printf("  %s0%s - Creation de groupe\n", COULEUR_VERT, COULEUR_RESET);
    printf("  %s1%s - Rejoindre un groupe\n", COULEUR_VERT, COULEUR_RESET);
    printf("  %s2%s - Lister les groupes\n", COULEUR_VERT, COULEUR_RESET);
    printf("  %s3%s - Quitter\n", COULEUR_ROUGE, COULEUR_RESET);
    printf("%s========================================%s\n",
           COULEUR_CYAN, COULEUR_RESET);
    printf("Choix : ");
}

/*============================================================================*
 * FONCTION : lire_choix
 * DESCRIPTION : Lit et valide le choix de l'utilisateur
 *============================================================================*/
int lire_choix(void)
{
    char ligne[10];
    int choix;
    
    if (fgets(ligne, sizeof(ligne), stdin) == NULL)
    {
        return -1;
    }
    
    if (sscanf(ligne, "%d", &choix) != 1)
    {
        return -1;
    }
    
    return choix;
}

/*============================================================================*
 * FONCTION : creer_groupe_cmd
 * DESCRIPTION : Cree un nouveau groupe de discussion
 *============================================================================*/
void creer_groupe_cmd(int socket_fd, ConfigClient *config)
{
    char nom_groupe[TAILLE_NOM_GROUPE];
    char mot_passe[TAILLE_MOT_PASSE];
    struct struct_message msg;
    struct struct_message reponse;
    socklen_t len_addr;
    ssize_t n;
    
    printf("\n%sSaisir le nom du groupe : %s", COULEUR_CYAN, COULEUR_RESET);
    if (fgets(nom_groupe, sizeof(nom_groupe), stdin) == NULL)
    {
        return;
    }
    nom_groupe[strcspn(nom_groupe, "\n")] = 0;
    
    if (strlen(nom_groupe) == 0)
    {
        printf("%sNom de groupe invalide%s\n", COULEUR_ROUGE, COULEUR_RESET);
        return;
    }
    
    printf("%sSaisir le mot de passe : %s", COULEUR_CYAN, COULEUR_RESET);
    if (fgets(mot_passe, sizeof(mot_passe), stdin) == NULL)
    {
        return;
    }
    mot_passe[strcspn(mot_passe, "\n")] = 0;
    
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.Ordre, ORDRE_CRE, TAILLE_ORDRE - 1);
    strncpy(msg.Emetteur, config->nom_utilisateur, TAILLE_EMETTEUR - 1);
    sprintf(msg.Texte, "%s %s", nom_groupe, mot_passe);
    
    printf("%sEnvoi de la demande au serveur...%s\n",
           COULEUR_JAUNE, COULEUR_RESET);
    
    sendto(socket_fd, &msg, sizeof(msg), 0,
           (struct sockaddr *)&g_addr_serveur, sizeof(g_addr_serveur));
    
    len_addr = sizeof(g_addr_serveur);
    n = recvfrom(socket_fd, &reponse, sizeof(reponse), 0,
                (struct sockaddr *)&g_addr_serveur, &len_addr);
    
    if (n > 0)
    {
        if (strcmp(reponse.Ordre, ORDRE_ACK) == 0)
        {
            printf("%s%s%s\n", COULEUR_VERT, reponse.Texte, COULEUR_RESET);
        }
        else
        {
            printf("%sErreur : %s%s\n", COULEUR_ROUGE, reponse.Texte, COULEUR_RESET);
        }
    }
}

/*============================================================================*
 * FONCTION : rejoindre_groupe_cmd
 * DESCRIPTION : Rejoint un groupe existant
 *============================================================================*/
void rejoindre_groupe_cmd(int socket_fd, ConfigClient *config)
{
    char nom_groupe[TAILLE_NOM_GROUPE];
    char mot_passe[TAILLE_MOT_PASSE];
    struct struct_message msg;
    struct struct_message reponse;
    struct struct_message msg_connexion;
    socklen_t len_addr;
    ssize_t n;
    int port_groupe;
    char moderateur[TAILLE_EMETTEUR];
    
    printf("\n%sSaisir le nom du groupe : %s", COULEUR_CYAN, COULEUR_RESET);
    if (fgets(nom_groupe, sizeof(nom_groupe), stdin) == NULL)
    {
        return;
    }
    nom_groupe[strcspn(nom_groupe, "\n")] = 0;
    
    if (strlen(nom_groupe) == 0)
    {
        printf("%sNom de groupe invalide%s\n", COULEUR_ROUGE, COULEUR_RESET);
        return;
    }
    
    printf("%sSaisir le mot de passe : %s", COULEUR_CYAN, COULEUR_RESET);
    if (fgets(mot_passe, sizeof(mot_passe), stdin) == NULL)
    {
        return;
    }
    mot_passe[strcspn(mot_passe, "\n")] = 0;
    
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.Ordre, ORDRE_CON, TAILLE_ORDRE - 1);
    strncpy(msg.Emetteur, config->nom_utilisateur, TAILLE_EMETTEUR - 1);
    sprintf(msg.Texte, "%s %s", nom_groupe, mot_passe);
    
    sendto(socket_fd, &msg, sizeof(msg), 0,
           (struct sockaddr *)&g_addr_serveur, sizeof(g_addr_serveur));
    
    len_addr = sizeof(g_addr_serveur);
    n = recvfrom(socket_fd, &reponse, sizeof(reponse), 0,
                (struct sockaddr *)&g_addr_serveur, &len_addr);
    
    if (n > 0)
    {
        if (strcmp(reponse.Ordre, ORDRE_ACK) == 0)
        {
            sscanf(reponse.Texte, "PORT=%d MODERATEUR=%s", &port_groupe, moderateur);
            
            printf("%sConnexion au groupe %s realisee%s\n",
                   COULEUR_VERT, nom_groupe, COULEUR_RESET);
            
            strncpy(g_nom_groupe_actuel, nom_groupe, TAILLE_NOM_GROUPE - 1);
            g_port_groupe_actuel = port_groupe;
            
            memset(&g_addr_groupe, 0, sizeof(g_addr_groupe));
            g_addr_groupe.sin_family = AF_INET;
            g_addr_groupe.sin_port = htons(port_groupe);
            g_addr_groupe.sin_addr.s_addr = inet_addr(config->ip_serveur);
            
            memset(&msg_connexion, 0, sizeof(msg_connexion));
            strncpy(msg_connexion.Ordre, ORDRE_CON, TAILLE_ORDRE - 1);
            strncpy(msg_connexion.Emetteur, config->nom_utilisateur, TAILLE_EMETTEUR - 1);
            sprintf(msg_connexion.Texte, "Connexion au groupe");
            
            sendto(socket_fd, &msg_connexion, sizeof(msg_connexion), 0,
                   (struct sockaddr *)&g_addr_groupe, sizeof(g_addr_groupe));
            
            printf("%sLancement de l affichage...%s\n",
                   COULEUR_JAUNE, COULEUR_RESET);
            
            g_pid_affichage = lancer_affichage(nom_groupe, port_groupe);
            g_dans_groupe = 1;
            
            /* Lancer directement le dialogue dans le groupe */
            dialoguer_groupe(socket_fd, config);
        }
        else
        {
            printf("%sErreur : %s%s\n", COULEUR_ROUGE, reponse.Texte, COULEUR_RESET);
        }
    }
}

/*============================================================================*
 * FONCTION : lister_groupes_cmd
 * DESCRIPTION : Liste tous les groupes disponibles
 *============================================================================*/
void lister_groupes_cmd(int socket_fd, ConfigClient *config)
{
    struct struct_message msg;
    struct struct_message reponse;
    socklen_t len_addr;
    ssize_t n;
    
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.Ordre, ORDRE_LST, TAILLE_ORDRE - 1);
    strncpy(msg.Emetteur, config->nom_utilisateur, TAILLE_EMETTEUR - 1);
    strcpy(msg.Texte, "Liste");
    
    sendto(socket_fd, &msg, sizeof(msg), 0,
           (struct sockaddr *)&g_addr_serveur, sizeof(g_addr_serveur));
    
    len_addr = sizeof(g_addr_serveur);
    n = recvfrom(socket_fd, &reponse, sizeof(reponse), 0,
                (struct sockaddr *)&g_addr_serveur, &len_addr);
    
    if (n > 0)
    {
        printf("\n%s========== LISTE DES GROUPES ==========%s\n",
               COULEUR_CYAN, COULEUR_RESET);
        printf("%s\n", reponse.Texte);
        printf("%s========================================%s\n",
               COULEUR_CYAN, COULEUR_RESET);
    }
}

/*============================================================================*
 * FONCTION : dialoguer_groupe
 * DESCRIPTION : Permet de dialoguer dans le groupe actuel
 *============================================================================*/
void dialoguer_groupe(int socket_fd, ConfigClient *config)
{
    char ligne[TAILLE_TEXTE];
    struct struct_message msg;
    
    if (!g_dans_groupe)
    {
        printf("%sVous devez d abord rejoindre un groupe%s\n",
               COULEUR_ROUGE, COULEUR_RESET);
        return;
    }
    
    printf("\n%s========================================%s\n",
           COULEUR_MAGENTA, COULEUR_RESET);
    printf("%sGroupe : %s%s\n", COULEUR_MAGENTA, g_nom_groupe_actuel, COULEUR_RESET);
    printf("%s========================================%s\n",
           COULEUR_MAGENTA, COULEUR_RESET);
    printf("Tapez %squit%s pour revenir au menu\n",
           COULEUR_ROUGE, COULEUR_RESET);
    printf("Tapez %scmd%s pour entrer une commande\n",
           COULEUR_JAUNE, COULEUR_RESET);
    printf("%s========================================%s\n\n",
           COULEUR_MAGENTA, COULEUR_RESET);
    
    while (1)
    {
        printf("Message : ");
        if (fgets(ligne, sizeof(ligne), stdin) == NULL)
        {
            break;
        }
        ligne[strcspn(ligne, "\n")] = 0;
        
        if (strlen(ligne) == 0)
        {
            continue;
        }
        
        if (strcmp(ligne, "quit") == 0)
        {
            memset(&msg, 0, sizeof(msg));
            memcpy(msg.Ordre, ORDRE_DECI, strlen(ORDRE_DECI));
            msg.Ordre[TAILLE_ORDRE - 1] = '\0';
            strncpy(msg.Emetteur, config->nom_utilisateur, TAILLE_EMETTEUR - 1);
            strcpy(msg.Texte, "Deconnexion");
            
            sendto(socket_fd, &msg, sizeof(msg), 0,
                   (struct sockaddr *)&g_addr_groupe, sizeof(g_addr_groupe));
            
            if (g_pid_affichage > 0)
            {
                kill(g_pid_affichage, SIGTERM);
            }
            
            g_dans_groupe = 0;
            break;
        }
        else if (strcmp(ligne, "cmd") == 0)
        {
            printf("Commande : ");
            if (fgets(ligne, sizeof(ligne), stdin) == NULL)
            {
                break;
            }
            ligne[strcspn(ligne, "\n")] = 0;
            
            memset(&msg, 0, sizeof(msg));
            strncpy(msg.Ordre, ORDRE_CMD, TAILLE_ORDRE - 1);
            strncpy(msg.Emetteur, config->nom_utilisateur, TAILLE_EMETTEUR - 1);
            strncpy(msg.Texte, ligne, TAILLE_TEXTE - 1);
            
            sendto(socket_fd, &msg, sizeof(msg), 0,
                   (struct sockaddr *)&g_addr_groupe, sizeof(g_addr_groupe));
            
            printf("Commande : ");
        }
        else
        {
            memset(&msg, 0, sizeof(msg));
            strncpy(msg.Ordre, ORDRE_MES, TAILLE_ORDRE - 1);
            strncpy(msg.Emetteur, config->nom_utilisateur, TAILLE_EMETTEUR - 1);
            strncpy(msg.Texte, ligne, TAILLE_TEXTE - 1);
            
            sendto(socket_fd, &msg, sizeof(msg), 0,
                   (struct sockaddr *)&g_addr_groupe, sizeof(g_addr_groupe));
        }
    }
}

/*============================================================================*
 * FONCTION : lancer_affichage
 * DESCRIPTION : Lance le processus AffichageISY
 *============================================================================*/
pid_t lancer_affichage(const char *nom_groupe, int port_groupe)
{
    pid_t pid;
    char port_str[10];
    int devnull;
    
    sprintf(port_str, "%d", port_groupe);
    
    pid = fork();
    if (pid == -1)
    {
        perror("Erreur fork");
        return -1;
    }
    else if (pid == 0)
    {
        /* Rediriger stdout et stderr vers /dev/null pour ne pas polluer le terminal */
        devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1)
        {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        
        execl("./AffichageISY", "AffichageISY", nom_groupe, port_str, NULL);
        perror("Erreur execl");
        exit(1);
    }
    
    return pid;
}

/*============================================================================*
 * FONCTION : terminer_client_proprement
 * DESCRIPTION : Libere les ressources du client
 *============================================================================*/
void terminer_client_proprement(void)
{
    if (g_dans_groupe && g_pid_affichage > 0)
    {
        struct struct_message msg;
        
        memset(&msg, 0, sizeof(msg));
        memcpy(msg.Ordre, ORDRE_DECI, strlen(ORDRE_DECI));
        msg.Ordre[TAILLE_ORDRE - 1] = '\0';
        strncpy(msg.Emetteur, g_config.nom_utilisateur, TAILLE_EMETTEUR - 1);
        strcpy(msg.Texte, "Deconnexion");
        
        sendto(g_socket_client, &msg, sizeof(msg), 0,
               (struct sockaddr *)&g_addr_groupe, sizeof(g_addr_groupe));
        
        kill(g_pid_affichage, SIGTERM);
    }
    
    if (g_socket_client != -1)
    {
        close(g_socket_client);
    }
    
    printf("\n%sFin du programme%s\n", COULEUR_VERT, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : main
 * DESCRIPTION : Point d'entree du client ISY
 *============================================================================*/
int main(int argc, char **argv, char **envp)
{
    int choix;
    
    (void)argc;
    (void)argv;
    (void)envp;
    
    printf("%s========================================%s\n",
           COULEUR_CYAN, COULEUR_RESET);
    printf("%s      CLIENT ISY MESSAGERIE           %s\n",
           COULEUR_CYAN, COULEUR_RESET);
    printf("%s========================================%s\n\n",
           COULEUR_CYAN, COULEUR_RESET);
    
    if (signal(SIGINT, gestionnaire_sigint_client) == SIG_ERR)
    {
        perror("Erreur signal");
        exit(EXIT_FAILURE);
    }
    
    if (lire_configuration_client("client_config.txt", &g_config) < 0)
    {
        fprintf(stderr, "Erreur lecture configuration\n");
        exit(EXIT_FAILURE);
    }
    
    printf("%sLecture du fichier de configuration OK, utilisateur %s%s\n",
           COULEUR_VERT, g_config.nom_utilisateur, COULEUR_RESET);
    
    g_socket_client = initialiser_socket_client();
    if (g_socket_client < 0)
    {
        exit(EXIT_FAILURE);
    }
    
    memset(&g_addr_serveur, 0, sizeof(g_addr_serveur));
    g_addr_serveur.sin_family = AF_INET;
    g_addr_serveur.sin_port = htons(g_config.port_serveur);
    g_addr_serveur.sin_addr.s_addr = inet_addr(g_config.ip_serveur);
    
    while (g_continuer_client)
    {
        afficher_menu();
        choix = lire_choix();
        
        switch (choix)
        {
            case 0:
                creer_groupe_cmd(g_socket_client, &g_config);
                break;
            case 1:
                rejoindre_groupe_cmd(g_socket_client, &g_config);
                break;
            case 2:
                lister_groupes_cmd(g_socket_client, &g_config);
                break;
            case 3:
                g_continuer_client = 0;
                break;
            default:
                printf("%sChoix invalide%s\n", COULEUR_ROUGE, COULEUR_RESET);
                break;
        }
    }
    
    terminer_client_proprement();
    
    exit(EXIT_SUCCESS);
}
