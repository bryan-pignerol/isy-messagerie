/*============================================================================*
 * ISY MESSAGERIE - AffichageISY.h
 *============================================================================*
 * Auteur       : Votre Nom
 * Date         : 07/11/2025
 * Version      : 1.0
 *----------------------------------------------------------------------------*
 * Description  : Affichage des messages reçus d'un groupe de discussion
 *============================================================================*/

#ifndef AFFICHAGEISY_H
#define AFFICHAGEISY_H

#include "Commun.h"

/*============================================================================*
 * PROTOTYPES
 *============================================================================*/

void afficher_message(struct_message *msg);
void gestionnaire_sigint(int sig);
void gestionnaire_sigterm(int sig);

#endif /* AFFICHAGEISY_H */
