/*============================================================================*
 * ISY MESSAGERIE - AffichageISY.c
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Programme d'affichage des messages d'un groupe
 *============================================================================*/

#include "AffichageISY.h"

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
 * NOTE : En UDP client, pas besoin de bind ! Le système assigne un port auto.
 *============================================================================*/
int initialiser_socket_affichage(int port_groupe)
{
    int sockfd;
    
    (void)port_groupe;  /* Port groupe pas nécessaire ici */
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Erreur creation socket affichage");
        return -1;
    }
    
    /* Pas de bind() nécessaire pour un client UDP !
     * Le système assignera automatiquement un port éphémère
     * lors du premier recvfrom()
     */
    
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
    char moderateur[TAILLE_EMETTEUR];
    char ip_serveur[TAILLE_IP];
    char nom_utilisateur[TAILLE_EMETTEUR];
    char nom_unique[TAILLE_EMETTEUR];
    struct sockaddr_in addr_groupe;
    struct struct_message msg_connexion;
    FILE *f;
    char ligne[256];
    
    (void)envp;
    
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <nom_groupe> <port_groupe>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    strncpy(g_nom_groupe_aff, argv[1], TAILLE_NOM_GROUPE - 1);
    port_groupe = atoi(argv[2]);
    
    /* Le modérateur sera affiché simplement comme le nom du groupe */
    strncpy(moderateur, g_nom_groupe_aff, TAILLE_EMETTEUR - 1);
    moderateur[TAILLE_EMETTEUR - 1] = '\0';
    
    /* Lire l'IP du serveur ET le nom d'utilisateur depuis client_config.txt */
    strcpy(ip_serveur, "127.0.0.1");  /* Valeur par défaut */
    strcpy(nom_utilisateur, "User");   /* Valeur par défaut */
    f = fopen("client_config.txt", "r");
    if (f != NULL)
    {
        while (fgets(ligne, sizeof(ligne), f) != NULL)
        {
            if (strncmp(ligne, "IP_SERVEUR=", 11) == 0)
            {
                sscanf(ligne, "IP_SERVEUR=%s", ip_serveur);
            }
            else if (strncmp(ligne, "NOM=", 4) == 0)
            {
                sscanf(ligne, "NOM=%s", nom_utilisateur);
            }
        }
        fclose(f);
    }
    
    /* Créer un nom unique pour l'affichage : "nom_utilisateur_AFF" */
    /* Limiter à 15 caractères pour laisser place au suffixe "_AFF" */
    char nom_court[16];
    strncpy(nom_court, nom_utilisateur, 15);
    nom_court[15] = '\0';
    snprintf(nom_unique, TAILLE_EMETTEUR, "%s_AFF", nom_court);
    
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
    
    /* S'enregistrer auprès de GroupeISY pour recevoir les messages */
    memset(&addr_groupe, 0, sizeof(addr_groupe));
    addr_groupe.sin_family = AF_INET;
    addr_groupe.sin_port = htons(port_groupe);
    addr_groupe.sin_addr.s_addr = inet_addr(ip_serveur);  /* IP du serveur ! */
    
    /* Envoyer message de connexion au groupe avec nom unique */
    memset(&msg_connexion, 0, sizeof(msg_connexion));
    strncpy(msg_connexion.Ordre, ORDRE_CON, TAILLE_ORDRE - 1);
    strncpy(msg_connexion.Emetteur, nom_unique, TAILLE_EMETTEUR - 1);
    sprintf(msg_connexion.Texte, "Connexion affichage");
    
    sendto(g_socket_affichage, &msg_connexion, sizeof(msg_connexion), 0,
           (struct sockaddr *)&addr_groupe, sizeof(addr_groupe));
    
    boucle_reception(g_socket_affichage, g_nom_groupe_aff, moderateur);
    
    terminer_affichage_proprement();
    
    exit(EXIT_SUCCESS);
}