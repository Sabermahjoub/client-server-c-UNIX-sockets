#!/bin/bash

CLIENT_SRC="clientUDP.c"
SERVER_SRC="serveurUDP.c"
CLIENT_EXEC="clientUDP"
SERVER_EXEC="serveurUDP"

echo "Compilation du client..."
gcc -o $CLIENT_EXEC $CLIENT_SRC || { echo "Erreur de compilation pour le client"; exit 1; }

echo "Compilation du serveur..."
gcc -o $SERVER_EXEC $SERVER_SRC || { echo "Erreur de compilation pour le serveur"; exit 1; }

echo "Compilation terminée avec succès."

echo "Pour exécuter le serveur : ./$SERVER_EXEC <port>"
echo "Pour exécuter le client : ./$CLIENT_EXEC <adresse_serveur> <port>"
