# ISY Messagerie - I Seek You

> Système de messagerie instantanée en ligne de commande inspiré d'ICQ

## 📋 Description

ISY Messagerie est une application de chat en temps réel développée en C, utilisant des sockets UDP et de la mémoire partagée pour permettre des discussions de groupe sécurisées par mot de passe.

### Fonctionnalités principales

- ✅ Création de groupes de discussion protégés par mot de passe
- ✅ Connexion/déconnexion aux groupes
- ✅ Envoi et réception de messages en temps réel
- ✅ Système de points (10 points par message)
- ✅ Commandes spéciales pour modérateurs
- ✅ Affichage coloré des messages
- ✅ Gestion propre des signaux (CTRL-C)

## 🏗️ Architecture

Le projet est composé de **4 programmes** :

### 1. ServeurISY (Port 8000)
- Gère les demandes de création/connexion aux groupes
- Lance un processus `GroupeISY` pour chaque nouveau groupe
- Utilise la mémoire partagée pour communiquer avec les groupes

### 2. GroupeISY (Ports 80xx)
- Un processus par groupe de discussion
- Redistribue les messages à tous les membres
- Gère le système de points
- Traite les commandes de modération

### 3. ClientISY
- Interface en ligne de commande pour l'utilisateur
- Envoie les commandes au serveur
- Lance les processus d'affichage

### 4. AffichageISY
- Un processus par groupe rejoint
- Affiche les messages reçus en temps réel
- Terminal dédié à l'affichage

## 📁 Structure du projet

```
ISY_Messagerie/
├── src/
│   ├── ServeurISY.c
│   ├── GroupeISY.c
│   ├── ClientISY.c
│   └── AffichageISY.c
├── inc/
│   ├── Commun.h
│   ├── ServeurISY.h
│   ├── GroupeISY.h
│   ├── ClientISY.h
│   └── AffichageISY.h
├── obj/
│   └── (fichiers .o générés)
├── Makefile
├── serveur_config.txt
├── client_config.txt
├── README.md
└── Dockerfile
```

## 🚀 Compilation

```bash
# Compiler tous les programmes
make

# Compiler un programme spécifique
make serveur
make groupe
make client
make affichage

# Nettoyer
make clean   # Supprime les .o
make fclean  # Supprime tout
make re      # Recompile tout
```

## 🎮 Utilisation

### Lancer le serveur

```bash
./ServeurISY
```

Le serveur lit sa configuration depuis `serveur_config.txt` et démarre sur le port 8000.

### Lancer un client

```bash
./ClientISY
```

Le client lit sa configuration depuis `client_config.txt`.

### Menu principal

```
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

### Créer un groupe

1. Choisir l'option `0`
2. Saisir le nom du groupe
3. Saisir le mot de passe

### Rejoindre un groupe

1. Choisir l'option `1`
2. Saisir le nom du groupe
3. Saisir le mot de passe
4. Un terminal d'affichage se lance automatiquement

### Dialoguer dans un groupe

1. Choisir l'option `3`
2. Taper vos messages
3. Commandes spéciales :
   - `quit` : Quitter le groupe
   - `cmd` : Entrer une commande de modération

## 🎯 Commandes de modération

Seul le modérateur (créateur du groupe) peut utiliser ces commandes :

- `DELETE <nom>` : Exclure un membre du groupe
- `LIST` : Lister tous les membres avec leurs points

## 💎 Système de points

- **10 points** gagnés par message envoyé
- Les points peuvent être utilisés pour :
  - Messages en couleur (50 points)
  - Messages surlignés (100 points)

## 📝 Format des messages

Structure : `struct_message`
- **4 octets** : Ordre (CON, DECI, MES, CMD, etc.)
- **20 octets** : Nom de l'émetteur
- **100 octets** : Texte du message

### Ordres disponibles

| Ordre | Signification |
|-------|---------------|
| CON   | Connexion |
| DECI  | Déconnexion |
| MES   | Message |
| CMD   | Commande |
| CRE   | Création de groupe |
| DES   | Destruction de groupe |
| LST   | Liste des groupes |
| ACK   | Accusé de réception |
| ERR   | Erreur |
| INF   | Information |

## 🐳 Déploiement Docker

### Construire l'image

```bash
docker build -t isy-messagerie .
```

### Lancer le serveur

```bash
docker run -d -p 8000:8000/udp --name isy-serveur isy-messagerie ./ServeurISY
```

### Lancer un client

```bash
docker run -it --network host isy-messagerie ./ClientISY
```

## 🔧 Configuration

### serveur_config.txt

```ini
IP=127.0.0.1
PORT=8000
PROTOCOLE=UDP
```

### client_config.txt

```ini
NOM=VotreNom
IP_SERVEUR=127.0.0.1
PORT_SERVEUR=8000
```

## 🛠️ Technologies utilisées

- **Langage** : C (C99)
- **Sockets** : UDP/UNICAST
- **IPC** : Mémoire partagée (SHM) et sémaphores
- **Processus** : fork(), waitpid()
- **Signaux** : SIGINT pour terminaison propre

## ⚠️ Gestion des erreurs

- Vérification de tous les retours de fonctions système
- Gestion des signaux pour terminaison propre
- Nettoyage des ressources (sockets, SHM, processus fils)
- Messages d'erreur explicites avec `perror()`

## 📚 Conformité au sujet

✅ Architecture en 4 programmes  
✅ Communication UDP  
✅ Mémoire partagée entre ServeurISY et GroupeISY  
✅ Fichiers de configuration  
✅ Gestion du CTRL-C  
✅ Processus fork pour les groupes  
✅ Protection par mot de passe  
✅ Idée novatrice : Système de points  

## 👥 Auteur

Bryan - Projet ISY Messagerie - Novembre 2025

## 📄 Licence

Projet académique - ISEN
