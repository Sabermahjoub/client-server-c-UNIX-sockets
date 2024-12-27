#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int n;

    // Création du socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    // Associer le socket à l'adresse et au port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("Serveur UDP en écoute sur le port %d...\n", server_port);

    socklen_t client_len = sizeof(client_addr);
    while (1) {
        // Recevoir le nombre du client
        if (recvfrom(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&client_addr, &client_len) < 0) {
            perror("recvfrom");
            continue;
        }

        printf("Reçu du client : %d\n", n);

        // Générer n nombres aléatoires
        srand(time(NULL));
        int random_numbers[n];
        for (int i = 0; i < n; i++) {
            random_numbers[i] = rand() % 100;  // Nombres entre 0 et 99
        }

        // Envoyer les nombres au client
        if (sendto(sockfd, random_numbers, n * sizeof(int), 0, (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("sendto");
            continue;
        }
        printf("Envoyé %d nombres aléatoires au client.\n", n);
    }

    close(sockfd);
    return 0;
}
