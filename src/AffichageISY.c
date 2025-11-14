/*============================================================================*
 * ISY MESSAGERIE - AffichageISY.c
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Programme d'affichage des messages d'un groupe
 *============================================================================*/

#include "../inc/AffichageISY.h"

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
int g_socket_affichage = -1;
int g_continuer_affichage = 1;
char g_nom_groupe_aff[TAILLE_NOM_GROUPE];

/*============================================================================*
 * FONCTION : gestionnaire_sigint_affichage
 * DESCRIPTION : Gere le signal SIGINT pour le processus d'affichage
 *============================================================================*/
void gestionnaire_sigint_affichage(int sig)
{
    (void)sig;
    g_continuer_affichage = 0;
}

/*============================================================================*
 * FONCTION : initialiser_socket_affichage
 * DESCRIPTION : Cree le socket UDP pour recevoir les messages
 *============================================================================*/
int initialiser_socket_affichage(int port_groupe)
{
    int sockfd;
    struct sockaddr_in servaddr;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Erreur creation socket affichage");
        return -1;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_groupe);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Erreur bind affichage");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

/*============================================================================*
 * FONCTION : afficher_message
 * DESCRIPTION : Affiche un message recu
 *============================================================================*/
void afficher_message(struct struct_message *msg)
{
    printf("%sMessage de %s : %s%s%s\n",
           COULEUR_CYAN, msg->Emetteur,
           COULEUR_BLANC, msg->Texte, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : afficher_connexion
 * DESCRIPTION : Affiche une notification de connexion
 *============================================================================*/
void afficher_connexion(const char *nom)
{
    printf("%s>>> %s a rejoint le groupe%s\n",
           COULEUR_VERT, nom, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : afficher_deconnexion
 * DESCRIPTION : Affiche une notification de deconnexion
 *============================================================================*/
void afficher_deconnexion(const char *nom)
{
    printf("%s<<< %s a quitte le groupe%s\n",
           COULEUR_ROUGE, nom, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : afficher_information
 * DESCRIPTION : Affiche une information systeme
 *============================================================================*/
void afficher_information(const char *texte)
{
    printf("%s[SYSTEME] %s%s\n",
           COULEUR_JAUNE, texte, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : boucle_reception
 * DESCRIPTION : Boucle principale de reception et affichage des messages
 *============================================================================*/
void boucle_reception(int socket_fd, const char *nom_groupe, const char *moderateur)
{
    struct struct_message msg;
    struct sockaddr_in addr_expediteur;
    socklen_t len_addr;
    ssize_t n;
    
    printf("\n%s===========================================%s\n",
           COULEUR_MAGENTA, COULEUR_RESET);
    printf("%sGroupe de discussion : %s%s\n",
           COULEUR_MAGENTA, nom_groupe, COULEUR_RESET);
    printf("%sModerateur : %s%s\n",
           COULEUR_MAGENTA, moderateur, COULEUR_RESET);
    printf("%s===========================================%s\n\n",
           COULEUR_MAGENTA, COULEUR_RESET);
    
    while (g_continuer_affichage)
    {
        len_addr = sizeof(addr_expediteur);
        n = recvfrom(socket_fd, &msg, sizeof(msg), 0,
                    (struct sockaddr *)&addr_expediteur, &len_addr);
        
        if (n < 0)
        {
            if (errno == EINTR)
            {
                break;
            }
            perror("Erreur recvfrom affichage");
            continue;
        }
        
        if (strcmp(msg.Ordre, ORDRE_MES) == 0)
        {
            afficher_message(&msg);
        }
        else if (strcmp(msg.Ordre, ORDRE_INF) == 0)
        {
            afficher_information(msg.Texte);
        }
        else if (strcmp(msg.Ordre, ORDRE_CON) == 0)
        {
            afficher_connexion(msg.Emetteur);
        }
        else if (strcmp(msg.Ordre, ORDRE_DECI) == 0)
        {
            afficher_deconnexion(msg.Emetteur);
        }
    }
}

/*============================================================================*
 * FONCTION : terminer_affichage_proprement
 * DESCRIPTION : Libere les ressources de l'affichage
 *============================================================================*/
void terminer_affichage_proprement(void)
{
    if (g_socket_affichage != -1)
    {
        close(g_socket_affichage);
    }
    
    printf("\n%s[Affichage] Termine%s\n", COULEUR_VERT, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : main
 * DESCRIPTION : Point d'entree du programme d'affichage
 *============================================================================*/
int main(int argc, char **argv, char **envp)
{
    int port_groupe;
    char moderateur[TAILLE_EMETTEUR] = "Unknown";
    
    (void)envp;
    
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <nom_groupe> <port_groupe>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    strncpy(g_nom_groupe_aff, argv[1], TAILLE_NOM_GROUPE - 1);
    port_groupe = atoi(argv[2]);
    
    if (signal(SIGINT, gestionnaire_sigint_affichage) == SIG_ERR)
    {
        perror("Erreur signal");
        exit(EXIT_FAILURE);
    }
    
    g_socket_affichage = initialiser_socket_affichage(port_groupe);
    if (g_socket_affichage < 0)
    {
        exit(EXIT_FAILURE);
    }
    
    boucle_reception(g_socket_affichage, g_nom_groupe_aff, moderateur);
    
    terminer_affichage_proprement();
    
    exit(EXIT_SUCCESS);
}
