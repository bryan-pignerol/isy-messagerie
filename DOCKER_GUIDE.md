# Guide de Déploiement Docker - ISY Messagerie

## 🐳 Présentation

Ce guide explique comment déployer ISY Messagerie avec Docker.

**⚠️ IMPORTANT : Architecture recommandée**
- **Serveur** → Dans Docker ✅
- **Clients** → Sur la machine hôte (PAS dans Docker) ✅

Cette architecture est la plus simple et la plus pratique car :
1. Le serveur est isolé dans Docker
2. Les clients ont accès direct au terminal pour l'interaction
3. Le processus AffichageISY fonctionne naturellement avec fork()
4. Pas de complications avec les terminaux interactifs dans Docker

## 📋 Prérequis

- Docker installé et fonctionnel
- Les ports 8000-8020 disponibles
- Les sources du projet compilées sur l'hôte

## 🏗️ Construction de l'image (pour le serveur uniquement)

```bash
# Dans le répertoire du projet
docker build -t isy-messagerie .
```

## 🚀 Déploiement Recommandé

### Étape 1 : Démarrer le serveur dans Docker

```bash
docker run -d \
  --name isy-serveur \
  -p 8000:8000/udp \
  -p 8001-8020:8001-8020/udp \
  isy-messagerie
```

### Étape 2 : Compiler les clients sur l'hôte

```bash
# Sur votre machine hôte
make
```

### Étape 3 : Lancer les clients sur l'hôte

**Client 1 (Bryan) :**
```bash
./ClientISY
```

**Client 2 (Sophie) - dans un autre terminal :**
```bash
# Créer une config pour Sophie
cat > client_config_sophie.txt << EOF
NOM=Sophie
IP_SERVEUR=127.0.0.1
PORT_SERVEUR=8000
EOF

# Lancer avec cette config
cp client_config_sophie.txt client_config.txt
./ClientISY
```

### Étape 4 : Utilisation normale

Les clients communiquent avec le serveur Docker via localhost:8000 !

## 🔍 Gestion du serveur Docker

### Voir les logs du serveur

```bash
docker logs -f isy-serveur
```

### Arrêter proprement le serveur

```bash
docker stop isy-serveur
docker rm isy-serveur
```

### Redémarrer le serveur

```bash
docker restart isy-serveur
```

## 📝 Configuration

### Pour un serveur sur une autre machine

**Sur la machine serveur (ex: 192.168.1.100) :**
```bash
docker run -d \
  --name isy-serveur \
  -p 8000:8000/udp \
  -p 8001-8020:8001-8020/udp \
  isy-messagerie
```

**Sur les machines clientes :**

Modifier `client_config.txt` :
```ini
NOM=VotreNom
IP_SERVEUR=192.168.1.100
PORT_SERVEUR=8000
```

Puis lancer :
```bash
./ClientISY
```

## ⚠️ Pourquoi NE PAS lancer le client dans Docker ?

### Problèmes si on met le client dans Docker :

1. **Fork d'AffichageISY ne fonctionne pas bien**
   - Docker complique le fork de processus
   - Les terminaux multiples sont difficiles à gérer

2. **Interaction utilisateur limitée**
   - Besoin de `-it` pour l'interactivité
   - Un seul terminal par conteneur

3. **Complexité inutile**
   - Besoin de `--network host`
   - Configuration plus compliquée

### Solution Simple = Serveur Docker + Clients Hôte ✅

C'est la meilleure approche :
- Simple à configurer
- Fonctionne parfaitement
- Serveur isolé dans Docker
- Clients avec terminaux natifs

## 🐛 Dépannage

### Le client ne peut pas se connecter au serveur

**Vérifier que le serveur Docker écoute :**
```bash
docker logs isy-serveur
# Doit afficher : "Serveur en ecoute sur le port 8000..."
```

**Vérifier les ports :**
```bash
docker port isy-serveur
# Doit afficher : 8000/udp -> 0.0.0.0:8000
```

**Tester la connexion :**
```bash
nc -u localhost 8000
# Ou utiliser netcat pour tester UDP
```

### Le serveur ne démarre pas

**Voir les erreurs :**
```bash
docker logs isy-serveur
```

**Vérifier si le port est déjà utilisé :**
```bash
netstat -ulnp | grep 8000
```

### Nettoyer les segments de mémoire partagée

Si le serveur crash, nettoyer la SHM :
```bash
docker exec isy-serveur ipcs -m
docker exec isy-serveur ipcrm -m <shmid>
```

Ou redémarrer le conteneur :
```bash
docker restart isy-serveur
```

## 📊 Scénario de Test Complet

```bash
# 1. Build de l'image serveur
docker build -t isy-messagerie .

# 2. Démarrage du serveur
docker run -d --name isy-serveur \
  -p 8000:8000/udp -p 8001-8020:8001-8020/udp \
  isy-messagerie

# 3. Vérifier que le serveur tourne
docker logs isy-serveur

# 4. Compiler les clients (sur l'hôte)
make

# 5. Terminal 1 - Client Bryan
./ClientISY
# → Créer un groupe "Test" avec mot de passe "1234"

# 6. Terminal 2 - Client Sophie
cp client_config2.txt client_config.txt
./ClientISY
# → Rejoindre le groupe "Test" avec mot de passe "1234"

# 7. Échanger des messages !

# 8. Cleanup
docker stop isy-serveur
docker rm isy-serveur
```

## 📚 Commandes Utiles

```bash
# Voir tous les conteneurs ISY
docker ps -a | grep isy

# Voir les ressources utilisées
docker stats isy-serveur

# Inspecter le conteneur
docker inspect isy-serveur

# Accéder au shell du serveur
docker exec -it isy-serveur /bin/bash

# Voir les processus dans le serveur
docker exec isy-serveur ps aux

# Rebuild rapide
docker stop isy-serveur && docker rm isy-serveur
docker build -t isy-messagerie .
docker run -d --name isy-serveur \
  -p 8000:8000/udp -p 8001-8020:8001-8020/udp \
  isy-messagerie
```

## ✅ Checklist de déploiement

- [ ] Image Docker construite (`docker build`)
- [ ] Serveur Docker démarré (`docker run`)
- [ ] Logs serveur OK (`docker logs`)
- [ ] Ports exposés (8000-8020/udp)
- [ ] Clients compilés sur l'hôte (`make`)
- [ ] Configuration client adaptée (`client_config.txt`)
- [ ] Connexion testée avec un client
- [ ] Groupe créé et messages échangés

---

## 🎯 Résumé Architecture

```
┌─────────────────────────────────┐
│     MACHINE HÔTE / SERVEUR      │
│                                 │
│  ┌───────────────────────────┐  │
│  │   DOCKER CONTAINER        │  │
│  │                           │  │
│  │   ServeurISY (8000)       │  │
│  │   ├─ GroupeISY (8001)     │  │
│  │   ├─ GroupeISY (8002)     │  │
│  │   └─ GroupeISY (800x)     │  │
│  │                           │  │
│  └───────────────────────────┘  │
│          ↑                       │
│          │ UDP 8000-8020         │
│          ↓                       │
│  ┌───────────────────────────┐  │
│  │   HÔTE (pas Docker)       │  │
│  │                           │  │
│  │   ./ClientISY             │  │
│  │   └─ AffichageISY (fork)  │  │
│  │                           │  │
│  └───────────────────────────┘  │
└─────────────────────────────────┘

┌─────────────────────────────────┐
│     MACHINE CLIENT              │
│                                 │
│   ./ClientISY                   │
│   └─ AffichageISY (fork)        │
│                                 │
│   client_config.txt:            │
│   IP_SERVEUR=192.168.1.100      │
└─────────────────────────────────┘
```

---

**Auteur :** Bryan  
**Date :** Novembre 2025  
**Version :** 2.0 (Corrigée - Architecture recommandée)

## 📝 Configuration

### Modifier la configuration du serveur

Créer un fichier `serveur_config_custom.txt` :
```ini
IP=0.0.0.0
PORT=8000
PROTOCOLE=UDP
```

Utiliser :
```bash
docker run -d \
  --name isy-serveur \
  -v $(pwd)/serveur_config_custom.txt:/app/serveur_config.txt \
  -p 8000:8000/udp \
  -p 8001-8020:8001-8020/udp \
  isy-messagerie ./ServeurISY
```

### Modifier la configuration du client

```bash
docker run -it \
  --name isy-client \
  -v $(pwd)/client_config.txt:/app/client_config.txt \
  --network host \
  isy-messagerie ./ClientISY
```

## 🔍 Debugging

### Voir les processus dans le conteneur

```bash
docker exec -it isy-serveur ps aux
```

### Accéder au shell du conteneur

```bash
docker exec -it isy-serveur /bin/bash
```

### Voir les segments de mémoire partagée

```bash
docker exec -it isy-serveur ipcs -m
```

### Nettoyer les ressources

```bash
# Arrêter tous les conteneurs ISY
docker stop $(docker ps -a | grep isy | awk '{print $1}')

# Supprimer tous les conteneurs ISY
docker rm $(docker ps -a | grep isy | awk '{print $1}')

# Supprimer l'image
docker rmi isy-messagerie

# Supprimer le réseau
docker network rm isy-network
```

## 🎯 Commandes utiles

### Docker Compose (optionnel)

Créer `docker-compose.yml` :
```yaml
version: '3'
services:
  serveur:
    build: .
    command: ./ServeurISY
    ports:
      - "8000:8000/udp"
      - "8001-8020:8001-8020/udp"
    networks:
      - isy-network

  client1:
    build: .
    command: ./ClientISY
    stdin_open: true
    tty: true
    depends_on:
      - serveur
    networks:
      - isy-network

networks:
  isy-network:
    driver: bridge
```

Utiliser :
```bash
docker-compose up -d serveur
docker-compose run client1
```

## ⚠️ Notes importantes

1. **Ports UDP** : Assurez-vous que les ports UDP sont bien exposés
2. **Mémoire partagée** : Fonctionne correctement dans Docker
3. **Signaux** : CTRL+C fonctionne pour arrêter proprement
4. **Network host** : Peut être nécessaire pour certains cas d'usage

## 🐛 Problèmes courants

### Le client ne peut pas se connecter au serveur

**Solution :**
```bash
# Utiliser --network host pour le client
docker run -it --network host isy-messagerie ./ClientISY
```

### Les ports UDP ne fonctionnent pas

**Vérifier :**
```bash
# Sur l'hôte
netstat -ulnp | grep 8000

# Firewall
sudo ufw allow 8000:8020/udp
```

### Erreur de mémoire partagée

**Nettoyer :**
```bash
docker exec -it isy-serveur ipcs -m | grep $(whoami) | awk '{print $2}' | xargs -I {} ipcrm -m {}
```

## 📚 Ressources

- Documentation Docker : https://docs.docker.com/
- Guide UDP Docker : https://docs.docker.com/network/
- Projet GitHub : (votre repo)

## ✅ Checklist de déploiement

- [ ] Image Docker construite
- [ ] Ports 8000-8020 ouverts
- [ ] Configuration serveur adaptée
- [ ] Configuration client adaptée
- [ ] Firewall configuré
- [ ] Tests de connexion effectués

---

**Auteur :** Bryan  
**Date :** Novembre 2025  
**Version :** 1.0
