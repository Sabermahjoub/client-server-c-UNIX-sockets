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
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char username[50], password[50];

    // creer socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }

    // addresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    // connexion au serveur
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    printf("Connexion réussie\n");

    // Boucle d'auth
    for (int attempt = 0; attempt < 3; attempt++) {
        printf("Saisir nom d'utilisateur: ");
        scanf("%s", username);
        printf("Saisir mot de passe ");
        scanf("%s", password);

        snprintf(buffer, BUFFER_SIZE, "%s:%s", username, password);
        // printf(">>>sent password is %s",buffer);
        send(client_sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (strcmp(buffer, "AUTH_OK") == 0) {
            printf("Auth réussie\n");
            break;
        } else {
            printf("Auth échouée. %d tentatives restantes.\n", 2 - attempt);
            if (attempt == 2) {
                printf("Échec. Connexion annulée.\n");
                close(client_sock);
                exit(EXIT_FAILURE);
            }
        }
    }

    // boucle principale / fonctions à invoquer
    while (1) {
        print_menu();
        int choice;
        scanf("%d", &choice);

        snprintf(buffer, BUFFER_SIZE, "%d", choice);
        send(client_sock, buffer, strlen(buffer), 0);

        if (choice == 5) {
            printf("Exit...\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);
// aperçu contenu du fichier
        if (choice == 3) {
            printf("Saisir le nom du fichier ");
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
