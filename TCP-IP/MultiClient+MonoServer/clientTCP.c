#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void print_menu() {
    printf("\nServices disponibles :\n");
    printf("1. Afficher la date et l'heure du serveur\n");
    printf("2. Afficher la liste des fichiers d'un répertoire\n");
    printf("3. Afficher le contenu d'un fichier\n");
    printf("4. Afficher la durée écoulée depuis le début de la connexion\n");
    printf("5. Quitter\n");
    printf("Entrez votre choix : ");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Utilisation : %s <adresse_ip_serveur> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char username[50], password[50];

    // Création du socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Échec de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Adresse du serveur invalide");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Échec de la connexion");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    printf("Connecté au serveur\n");

    // Boucle d'authentification
    for (int attempt = 0; attempt < 3; attempt++) {
        printf("Entrez votre nom d'utilisateur : ");
        scanf("%s", username);
        printf("Entrez votre mot de passe : ");
        scanf("%s", password);

        snprintf(buffer, BUFFER_SIZE, "%s:%s", username, password);
        send(client_sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (strcmp(buffer, "AUTH_OK") == 0) {
            printf("Authentification réussie\n");
            break;
        } else {
            printf("Authentification échouée. Il reste %d tentatives.\n", 2 - attempt);
            if (attempt == 2) {
                printf("Trop de tentatives échouées. Fermeture.\n");
                close(client_sock);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Boucle principale
    while (1) {
        print_menu();
        int choice;
        scanf("%d", &choice);

        snprintf(buffer, BUFFER_SIZE, "%d", choice);
        send(client_sock, buffer, strlen(buffer), 0);

        if (choice == 5) {
            printf("Fermeture...\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (choice == 3) {
            // Demander le contenu d'un fichier
            printf("Entrez le nom du fichier : ");
            char filename[50];
            scanf("%s", filename);
            send(client_sock, filename, strlen(filename), 0);

            memset(buffer, 0, BUFFER_SIZE);
            recv(client_sock, buffer, BUFFER_SIZE, 0);
        }

        printf("Réponse du serveur :\n%s\n", buffer);
    }

    close(client_sock);
    return 0;
}
