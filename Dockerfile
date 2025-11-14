#=============================================================================#
# ISY MESSAGERIE - Dockerfile
#=============================================================================#
# Auteur       : Bryan
# Date         : 14/11/2025
# Version      : 1.0
#-----------------------------------------------------------------------------#
# Description  : Image Docker pour deployer le serveur ISY Messagerie
#=============================================================================#

# Image de base Ubuntu
FROM ubuntu:24.04

# Mise a jour et installation des dependances
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

# Creation du repertoire de travail
WORKDIR /app

# Copie des sources
COPY src/ ./src/
COPY inc/ ./inc/
COPY Makefile .
COPY serveur_config.txt .
COPY client_config.txt .

# Creation du repertoire obj
RUN mkdir -p obj

# Compilation
RUN make all

# Exposition des ports
# Port 8000 pour le serveur principal
# Ports 8001-8020 pour les groupes
EXPOSE 8000/udp
EXPOSE 8001-8020/udp

# Le serveur tourne dans Docker
# Les clients tournent sur la machine hote (pas dans Docker)
# 
# Lancer le serveur :
#   docker run -d -p 8000:8000/udp -p 8001-8020:8001-8020/udp \
#     --name isy-serveur isy-messagerie
#
# Lancer le client (sur la machine hote, PAS en Docker) :
#   ./ClientISY

CMD ["./ServeurISY"]
