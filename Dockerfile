# syntax=docker/dockerfile:1

FROM alpine:latest as base

# Installer les outils de compilation C
RUN apk add --no-cache gcc musl-dev make

################################################################################
# Stage de compilation
FROM base as build

WORKDIR /app

# Copier les fichiers sources
COPY . .

# Compiler le programme ServeurISY
# Adapte la commande selon ta structure de projet
RUN gcc -o ServeurISY ServeurISY.c -Wall -Wextra
# Ou si tu as un Makefile :
# RUN make ServeurISY

################################################################################
# Stage final
FROM base AS final

# Installer uniquement les dépendances runtime si nécessaire
RUN apk add --no-cache libgcc

# Créer un utilisateur non-privilégié
ARG UID=10001
RUN adduser \
    --disabled-password \
    --gecos "" \
    --home "/nonexistent" \
    --shell "/sbin/nologin" \
    --no-create-home \
    --uid "${UID}" \
    appuser

WORKDIR /app

# Copier l'exécutable compilé depuis le stage build
COPY --from=build /app/ServeurISY /app/

# Donner les permissions appropriées
RUN chown appuser:appuser /app/ServeurISY

USER appuser

# Lancer le serveur
ENTRYPOINT [ "/app/ServeurISY" ]