# 🎉 ISY Messagerie - Projet Complet et Fonctionnel

## 📊 Statistiques du Projet

- **Lignes de code :** ~1940 lignes (C + headers)
- **Fichiers sources :** 4 programmes C + 5 headers
- **Compilé avec :** GCC avec flags stricts (-Wall -Wextra -Werror)
- **Architecture :** 4 processus communicants
- **Technologies :** Sockets UDP, Mémoire partagée, Signaux POSIX

## ✅ Fonctionnalités Implémentées

### Conformité au Sujet

✅ **Architecture en 4 programmes**
- ServeurISY (gestion des groupes)
- GroupeISY (discussion par groupe)
- ClientISY (interface utilisateur)
- AffichageISY (affichage dédié)

✅ **Communication réseau**
- Sockets UDP/UNICAST
- Port serveur : 8000
- Ports groupes : 80xx (8001, 8002, etc.)
- Structure de message : 5 + 20 + 100 octets

✅ **Communication inter-processus**
- Mémoire partagée (SHM) entre ServeurISY et GroupeISY
- Utilisation de ftok, shmget, shmat, shmdt, shmctl
- Synchronisation des données

✅ **Gestion des processus**
- Fork pour créer les processus GroupeISY
- Waitpid pour attendre les fils
- Terminaison propre avec nettoyage

✅ **Gestion des signaux**
- Handler SIGINT (CTRL-C)
- Arrêt propre de tous les processus
- Libération des ressources

✅ **Fichiers de configuration**
- serveur_config.txt (IP, port, protocole)
- client_config.txt (nom, IP serveur, port)
- Lecture au démarrage

✅ **Sécurité**
- Groupes protégés par mot de passe
- Modérateur avec droits spéciaux
- Validation des accès

✅ **Qualité logicielle**
- Code indenté et commenté
- Noms de variables explicites
- Vérification de tous les retours
- Gestion d'erreurs complète
- Compilation sans warnings

### Fonctionnalités Utilisateur

📝 **Menu principal**
- Créer un groupe
- Rejoindre un groupe
- Lister les groupes
- Dialoguer dans un groupe
- Quitter proprement

💬 **Messagerie**
- Envoi/réception en temps réel
- Notifications de connexion/déconnexion
- Messages système colorés
- Affichage dédié par groupe

👑 **Commandes modérateur**
- `DELETE <nom>` : Exclure un membre
- `LIST` : Lister les membres et leurs points

### 💎 Idée Novatrice : Système de Points

Le système de points type Twitch est **entièrement implémenté** :

- ✅ **10 points par message** envoyé
- ✅ **Suivi des points** pour chaque membre
- ✅ **Affichage des points** avec la commande LIST
- ✅ **Prêt pour extension** : couleurs (50pts), highlight (100pts)

## 🎨 Interface Utilisateur

### Couleurs ANSI
- 🔵 Cyan : Menus et titres
- 🟢 Vert : Succès et confirmations
- 🟡 Jaune : Warnings et informations
- 🔴 Rouge : Erreurs et déconnexions
- 🟣 Magenta : Groupes et messages

### Affichage Structuré
```
==========================================
      CLIENT ISY MESSAGERIE
==========================================

========================================
Choix des commandes :
========================================
  0 - Creation de groupe
  1 - Rejoindre un groupe
  2 - Lister les groupes
  3 - Dialoguer sur un groupe
  4 - Quitter
========================================
```

## 🏗️ Architecture Technique

### Processus

```
[ServeurISY] (Port 8000)
    |
    ├── fork → [GroupeISY 1] (Port 8001)
    ├── fork → [GroupeISY 2] (Port 8002)
    └── fork → [GroupeISY N] (Port 800N)

[ClientISY]
    |
    └── fork → [AffichageISY]
```

### Mémoire Partagée

```c
typedef struct {
    int nb_groupes;
    Groupe groupes[MAX_GROUPES];
} MemoirePartagee;

typedef struct {
    char nom[50];
    char mot_passe[50];
    char moderateur[20];
    int port;
    pid_t pid_groupe;
    int nb_membres;
    Membre membres[50];
    int nb_messages;
    int actif;
} Groupe;
```

### Messages Réseau

```c
struct struct_message {
    char Ordre[5];      // CON, DECI, MES, CMD, etc.
    char Emetteur[20];  // Nom de l'émetteur
    char Texte[100];    // Contenu du message
};
```

## 📁 Structure du Projet

```
ISY_Messagerie/
├── src/
│   ├── ServeurISY.c      (400 lignes)
│   ├── GroupeISY.c       (370 lignes)
│   ├── ClientISY.c       (540 lignes)
│   └── AffichageISY.c    (200 lignes)
├── inc/
│   ├── Commun.h          (150 lignes)
│   ├── ServeurISY.h
│   ├── GroupeISY.h
│   ├── ClientISY.h
│   └── AffichageISY.h
├── obj/                  (fichiers .o)
├── Makefile              (complet et fonctionnel)
├── serveur_config.txt
├── client_config.txt
├── client_config2.txt    (pour tests multi-clients)
├── Dockerfile            (déploiement Docker)
├── .dockerignore
├── start.sh              (script de démarrage rapide)
├── README.md             (documentation complète)
└── DOCKER_GUIDE.md       (guide déploiement Docker)
```

## 🚀 Utilisation

### Compilation

```bash
make              # Compile tout
make serveur      # Compile ServeurISY
make client       # Compile ClientISY
make clean        # Nettoie les .o
make fclean       # Supprime tout
make re           # Recompile tout
```

### Lancement

**Terminal 1 - Serveur :**
```bash
./ServeurISY
```

**Terminal 2 - Client Bryan :**
```bash
./ClientISY
```

**Terminal 3 - Client Sophie :**
```bash
# Modifier client_config.txt ou utiliser client_config2.txt
./ClientISY
```

### Script de démarrage rapide

```bash
./start.sh
```

## 🐳 Déploiement Docker

```bash
# Construire l'image
docker build -t isy-messagerie .

# Lancer le serveur
docker run -d -p 8000:8000/udp -p 8001-8020:8001-8020/udp \
  --name isy-serveur isy-messagerie ./ServeurISY

# Lancer un client
docker run -it --network host isy-messagerie ./ClientISY
```

Voir `DOCKER_GUIDE.md` pour plus de détails.

## 🧪 Tests Réalisés

✅ Compilation sans warnings ni erreurs  
✅ Création de groupes multiples  
✅ Connexion simultanée de plusieurs clients  
✅ Envoi/réception de messages  
✅ Système de points fonctionnel  
✅ Commandes de modération  
✅ Arrêt propre avec CTRL-C  
✅ Gestion des erreurs (mauvais mot de passe, etc.)  
✅ Nettoyage des ressources (SHM, sockets, processus)  

## 💡 Points Forts

1. **Code propre et structuré**
   - Headers séparés
   - Fonctions bien découpées
   - Commentaires explicites

2. **Robustesse**
   - Vérification de tous les retours
   - Gestion d'erreurs complète
   - Nettoyage systématique

3. **Respect du sujet**
   - Toutes les exigences respectées
   - Architecture conforme
   - Utilisation des technologies demandées

4. **Qualité professionnelle**
   - Compilation stricte (-Werror)
   - Documentation complète
   - Déploiement Docker

5. **Innovation**
   - Système de points Twitch-style
   - Interface colorée
   - Scripts de facilitation

## 🎓 Concepts Utilisés

### Programmation Système C
- ✅ Sockets UDP (socket, bind, sendto, recvfrom)
- ✅ Mémoire partagée (shmget, shmat, shmdt, shmctl)
- ✅ Processus (fork, waitpid, getpid)
- ✅ Signaux (signal, kill, SIGINT)
- ✅ Fichiers (fopen, fgets, sscanf)
- ✅ Structures et types personnalisés

### Bonnes Pratiques
- ✅ Code portable POSIX
- ✅ Gestion mémoire rigoureuse
- ✅ Éviter les fuites de ressources
- ✅ Commentaires informatifs
- ✅ Nommage cohérent

## 📚 Documentation Fournie

1. **README.md** : Guide utilisateur complet
2. **DOCKER_GUIDE.md** : Déploiement et scénarios
3. **Ce fichier** : Vue d'ensemble du projet
4. **Code commenté** : Explications inline
5. **Headers** : Documentation des fonctions

## 🎯 Prêt pour la Soutenance

Le projet est **100% fonctionnel** et prêt à être présenté. Tous les fichiers sont compilés, testés et documentés.

### Démonstration suggérée

1. **Montrer l'architecture** (schéma)
2. **Compiler** le projet (`make`)
3. **Lancer le serveur** (Terminal 1)
4. **Lancer 2 clients** (Terminaux 2 et 3)
5. **Créer un groupe** (Client 1)
6. **Rejoindre le groupe** (Client 2)
7. **Échanger des messages**
8. **Montrer le système de points** (`cmd` → `list`)
9. **Exclure un membre** (commande modérateur)
10. **Arrêt propre** (CTRL-C)

### Points à souligner

- ✨ Idée novatrice : système de points
- 🎨 Interface utilisateur soignée
- 🔒 Sécurité avec mots de passe
- 🐳 Déploiement Docker ready
- 📝 Code de qualité professionnelle

---

## 🎊 Conclusion

Le projet **ISY Messagerie** est un système de chat complet et fonctionnel qui :

- ✅ Respecte **toutes les exigences** du sujet
- ✅ Utilise correctement **toutes les technologies** demandées
- ✅ Propose une **idée novatrice** avec le système de points
- ✅ Est **robuste et bien testé**
- ✅ Possède une **documentation complète**
- ✅ Est **prêt pour le déploiement**

**Prêt à être utilisé, présenté et déployé ! 🚀**

---

**Auteur :** Bryan  
**Date :** 14 Novembre 2025  
**Projet :** ISY Messagerie - I Seek You  
**Cours :** Programmation Système C - Linux
