#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

void display_menu() {
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
        fprintf(stderr, "Usage: %s <server_address> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_address = argv[1];
    int server_port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char username[50], password[50];
    int choice;

    // Création du socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connexion au serveur réussie.\n");

    // Authentification
    printf("Nom d'utilisateur : ");
    scanf("%s", username);
    printf("Mot de passe : ");
    scanf("%s", password);

    snprintf(buffer, BUFFER_SIZE, "%s:%s", username, password);
    send(sockfd, buffer, strlen(buffer), 0);

    // Vérification de l'authentification
    recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (strcmp(buffer, "AUTH_OK") != 0) {
        printf("Authentification échouée.\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Authentification réussie.\n");

    // Interaction avec le serveur
    while (1) {
        display_menu();
        scanf("%d", &choice);

        send(sockfd, &choice, sizeof(choice), 0);
        if (choice == 5) break;

        switch (choice) {
            case 1: // Date et heure
                recv(sockfd, buffer, BUFFER_SIZE, 0);
                printf("Date et heure du serveur : %s\n", buffer);
                break;

            case 2: // Liste des fichiers
                recv(sockfd, buffer, BUFFER_SIZE, 0);
                printf("Liste des fichiers :\n%s\n", buffer);
                break;

            case 3: { // Contenu d'un fichier
                char filename[50];
                printf("Entrez le nom du fichier : ");
                scanf("%s", filename);
                send(sockfd, filename, strlen(filename), 0);
                recv(sockfd, buffer, BUFFER_SIZE, 0);
                printf("Contenu du fichier :\n%s\n", buffer);
                break;
            }

            case 4: // Durée de connexion
                recv(sockfd, buffer, BUFFER_SIZE, 0);
                printf("Durée écoulée : %s\n", buffer);
                break;

            default:
                printf("Choix invalide.\n");
        }
    }

    close(sockfd);
    printf("Déconnexion.\n");
    return 0;
}
