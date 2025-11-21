/*============================================================================*
 * ISY MESSAGERIE - GroupeISY.c
 *============================================================================*
 * Auteur       : Bryan
 * Date         : 14/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Gestionnaire d'un groupe de discussion
 *============================================================================*/

#include "GroupeISY.h"

/*============================================================================*
 * VARIABLES GLOBALES
 *============================================================================*/
int g_socket_groupe = -1;
int g_continuer_groupe = 1;
char g_nom_groupe[TAILLE_NOM_GROUPE];
char g_moderateur[TAILLE_EMETTEUR];
int g_nb_membres = 0;
Membre g_membres[MAX_MEMBRES_GROUPE];

/*============================================================================*
 * FONCTION : gestionnaire_sigint_groupe
 * DESCRIPTION : Gere le signal SIGINT pour le processus groupe
 *============================================================================*/
void gestionnaire_sigint_groupe(int sig)
{
    (void)sig;
    printf("\n%s[Groupe %s] Reception SIGINT, arret du groupe...%s\n",
           COULEUR_JAUNE, g_nom_groupe, COULEUR_RESET);
    g_continuer_groupe = 0;
}

/*============================================================================*
 * FONCTION : initialiser_socket_groupe
 * DESCRIPTION : Cree et configure le socket UDP du groupe
 *============================================================================*/
int initialiser_socket_groupe(int port)
{
    int sockfd;
    struct sockaddr_in servaddr;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Erreur creation socket groupe");
        return -1;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Erreur bind groupe");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

/*============================================================================*
 * FONCTION : ajouter_membre
 * DESCRIPTION : Ajoute un membre au groupe
 *============================================================================*/
int ajouter_membre(const char *nom, struct sockaddr_in *addr)
{
    int i;
    
    for (i = 0; i < g_nb_membres; i++)
    {
        if (strcmp(g_membres[i].nom, nom) == 0)
        {
            g_membres[i].adresse = *addr;
            g_membres[i].actif = 1;
            return i;
        }
    }
    
    if (g_nb_membres >= MAX_MEMBRES_GROUPE)
    {
        return -1;
    }
    
    strncpy(g_membres[g_nb_membres].nom, nom, TAILLE_EMETTEUR - 1);
    g_membres[g_nb_membres].adresse = *addr;
    g_membres[g_nb_membres].points = 0;
    g_membres[g_nb_membres].actif = 1;
    g_nb_membres++;
    
    return g_nb_membres - 1;
}

/*============================================================================*
 * FONCTION : retirer_membre
 * DESCRIPTION : Retire un membre du groupe
 *============================================================================*/
int retirer_membre(const char *nom)
{
    int i;
    
    for (i = 0; i < g_nb_membres; i++)
    {
        if (strcmp(g_membres[i].nom, nom) == 0)
        {
            g_membres[i].actif = 0;
            return 0;
        }
    }
    
    return -1;
}

/*============================================================================*
 * FONCTION : trouver_membre
 * DESCRIPTION : Trouve l'index d'un membre par son nom
 *============================================================================*/
int trouver_membre(const char *nom)
{
    int i;
    
    for (i = 0; i < g_nb_membres; i++)
    {
        if (strcmp(g_membres[i].nom, nom) == 0 && g_membres[i].actif)
        {
            return i;
        }
    }
    
    return -1;
}

/*============================================================================*
 * FONCTION : ajouter_points
 * DESCRIPTION : Ajoute des points a un membre
 *============================================================================*/
void ajouter_points(const char *nom, int points)
{
    int idx;
    
    idx = trouver_membre(nom);
    if (idx != -1)
    {
        g_membres[idx].points += points;
    }
}

/*============================================================================*
 * FONCTION : redistribuer_message
 * DESCRIPTION : Envoie un message a tous les membres actifs
 *============================================================================*/
void redistribuer_message(struct struct_message *msg, int socket_fd)
{
    int i;
    int nb_destinataires = 0;
    char liste_membres[TAILLE_TEXTE] = "";
    
    for (i = 0; i < g_nb_membres; i++)
    {
        if (g_membres[i].actif)
        {
            sendto(socket_fd, msg, sizeof(*msg), 0,
                  (struct sockaddr *)&g_membres[i].adresse,
                  sizeof(g_membres[i].adresse));
            
            if (nb_destinataires > 0)
                strcat(liste_membres, " , ");
            strcat(liste_membres, g_membres[i].nom);
            nb_destinataires++;
        }
    }
    
    if (nb_destinataires > 0)
    {
        printf("Redistribution message a %s\n", liste_membres);
    }
}

/*============================================================================*
 * FONCTION : traiter_commande
 * DESCRIPTION : Traite une commande envoyee par le moderateur
 *============================================================================*/
void traiter_commande(struct struct_message *msg, int socket_fd)
{
    char commande[20];
    char cible[TAILLE_EMETTEUR];
    struct struct_message reponse;
    
    sscanf(msg->Texte, "%s %s", commande, cible);
    
    if (strcmp(commande, "DELETE") == 0 || strcmp(commande, "delete") == 0)
    {
        if (strcmp(msg->Emetteur, g_moderateur) == 0)
        {
            if (retirer_membre(cible) == 0)
            {
                memset(&reponse, 0, sizeof(reponse));
                strncpy(reponse.Ordre, ORDRE_INF, TAILLE_ORDRE - 1);
                strncpy(reponse.Emetteur, "SYSTEME", TAILLE_EMETTEUR - 1);
                snprintf(reponse.Texte, TAILLE_TEXTE, 
                        "%s a exclu %s du groupe", msg->Emetteur, cible);
                
                redistribuer_message(&reponse, socket_fd);
                
                printf("%s%s a exclu %s du groupe%s\n",
                       COULEUR_JAUNE, msg->Emetteur, cible, COULEUR_RESET);
            }
        }
    }
    else if (strcmp(commande, "LIST") == 0 || strcmp(commande, "list") == 0)
    {
        int i;
        char liste[TAILLE_TEXTE] = "Membres actifs:\n";
        
        for (i = 0; i < g_nb_membres; i++)
        {
            if (g_membres[i].actif)
            {
                char ligne[50];
                sprintf(ligne, "- %s (Points: %d)\n", 
                        g_membres[i].nom, g_membres[i].points);
                strcat(liste, ligne);
            }
        }
        
        memset(&reponse, 0, sizeof(reponse));
        strncpy(reponse.Ordre, ORDRE_INF, TAILLE_ORDRE - 1);
        strncpy(reponse.Emetteur, "SYSTEME", TAILLE_EMETTEUR - 1);
        strncpy(reponse.Texte, liste, TAILLE_TEXTE - 1);
        
        sendto(socket_fd, &reponse, sizeof(reponse), 0,
              (struct sockaddr *)&g_membres[trouver_membre(msg->Emetteur)].adresse,
              sizeof(g_membres[trouver_membre(msg->Emetteur)].adresse));
    }
}

/*============================================================================*
 * FONCTION : traiter_message
 * DESCRIPTION : Traite un message recu et le redistribue
 *============================================================================*/
void traiter_message(struct struct_message *msg, struct sockaddr_in *addr_client, int socket_fd)
{
    if (strcmp(msg->Ordre, ORDRE_CON) == 0)
    {
        struct struct_message notif;
        
        ajouter_membre(msg->Emetteur, addr_client);
        
        printf("%s%s s est connecte au groupe %s%s\n",
               COULEUR_VERT, msg->Emetteur, g_nom_groupe, COULEUR_RESET);
        
        memset(&notif, 0, sizeof(notif));
        strncpy(notif.Ordre, ORDRE_INF, TAILLE_ORDRE - 1);
        strncpy(notif.Emetteur, "SYSTEME", TAILLE_EMETTEUR - 1);
        sprintf(notif.Texte, "%s a rejoint le groupe", msg->Emetteur);
        
        redistribuer_message(&notif, socket_fd);
    }
    else if (strcmp(msg->Ordre, ORDRE_DECI) == 0)
    {
        struct struct_message notif;
        
        retirer_membre(msg->Emetteur);
        
        printf("%s%s s est deconnecte du groupe %s%s\n",
               COULEUR_ROUGE, msg->Emetteur, g_nom_groupe, COULEUR_RESET);
        
        memset(&notif, 0, sizeof(notif));
        strncpy(notif.Ordre, ORDRE_INF, TAILLE_ORDRE - 1);
        strncpy(notif.Emetteur, "SYSTEME", TAILLE_EMETTEUR - 1);
        sprintf(notif.Texte, "%s a quitte le groupe", msg->Emetteur);
        
        redistribuer_message(&notif, socket_fd);
    }
    else if (strcmp(msg->Ordre, ORDRE_MES) == 0)
    {
        printf("Reception Message %s : %s\n", msg->Emetteur, msg->Texte);
        
        ajouter_points(msg->Emetteur, POINTS_PAR_MESSAGE);
        
        redistribuer_message(msg, socket_fd);
    }
    else if (strcmp(msg->Ordre, ORDRE_CMD) == 0)
    {
        printf("Reception Commande %s : %s\n", msg->Emetteur, msg->Texte);
        traiter_commande(msg, socket_fd);
    }
    else if (strcmp(msg->Ordre, ORDRE_MERGE) == 0)
    {
        printf("%sFusion de groupe demandee%s\n", COULEUR_MAGENTA, COULEUR_RESET);
        /* Redistribuer aux clients pour qu'ils changent de groupe */
        redistribuer_message(msg, socket_fd);
        
        /* Attendre que les clients reçoivent le message */
        sleep(1);
        
        /* Fermer le groupe */
        printf("%sFermeture du groupe apres fusion%s\n", 
               COULEUR_JAUNE, COULEUR_RESET);
        g_continuer_groupe = 0;
    }
}

/*============================================================================*
 * FONCTION : terminer_groupe_proprement
 * DESCRIPTION : Libere les ressources du groupe
 *============================================================================*/
void terminer_groupe_proprement(void)
{
    if (g_socket_groupe != -1)
    {
        close(g_socket_groupe);
    }
    
    printf("%s[Groupe %s] Termine proprement%s\n",
           COULEUR_VERT, g_nom_groupe, COULEUR_RESET);
}

/*============================================================================*
 * FONCTION : main
 * DESCRIPTION : Point d'entree du gestionnaire de groupe
 *============================================================================*/
int main(int argc, char **argv, char **envp)
{
    int port;
    struct struct_message msg;
    struct sockaddr_in addr_client;
    socklen_t len_addr;
    ssize_t n;
    
    (void)envp;
    
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s <nom_groupe> <port> <moderateur>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    strncpy(g_nom_groupe, argv[1], TAILLE_NOM_GROUPE - 1);
    port = atoi(argv[2]);
    strncpy(g_moderateur, argv[3], TAILLE_EMETTEUR - 1);
    
    printf("\n%s========================================%s\n",
           COULEUR_MAGENTA, COULEUR_RESET);
    printf("%sLancement du groupe de discussion %s%s\n",
           COULEUR_MAGENTA, g_nom_groupe, COULEUR_RESET);
    printf("%sModerateur : %s%s\n",
           COULEUR_MAGENTA, g_moderateur, COULEUR_RESET);
    printf("%s========================================%s\n\n",
           COULEUR_MAGENTA, COULEUR_RESET);
    
    if (signal(SIGINT, gestionnaire_sigint_groupe) == SIG_ERR)
    {
        perror("Erreur signal");
        exit(EXIT_FAILURE);
    }
    
    g_socket_groupe = initialiser_socket_groupe(port);
    if (g_socket_groupe < 0)
    {
        exit(EXIT_FAILURE);
    }
    
    printf("%sGroupe en ecoute sur le port %d...%s\n\n",
           COULEUR_VERT, port, COULEUR_RESET);
    
    while (g_continuer_groupe)
    {
        len_addr = sizeof(addr_client);
        n = recvfrom(g_socket_groupe, &msg, sizeof(msg), 0,
                    (struct sockaddr *)&addr_client, &len_addr);
        
        if (n < 0)
        {
            if (errno == EINTR)
            {
                break;
            }
            perror("Erreur recvfrom groupe");
            continue;
        }
        
        traiter_message(&msg, &addr_client, g_socket_groupe);
    }
    
    terminer_groupe_proprement();
    
    exit(EXIT_SUCCESS);
}