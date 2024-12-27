#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define NMAX 100
#define BUFFER_SIZE 1024

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
    int n;

    // Création du socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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

    // Générer un nombre aléatoire
    srand(time(NULL));
    n = rand() % NMAX + 1;
    printf("Nombre aléatoire envoyé : %d\n", n);

    // Envoyer le nombre au serveur
    if (sendto(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    // Recevoir les nombres aléatoires du serveur
    socklen_t addr_len = sizeof(server_addr);
    int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
    if (bytes_received < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    printf("Réponse du serveur : ");
    for (int i = 0; i < bytes_received / sizeof(int); i++) {
        printf("%d ", ((int *)buffer)[i]);
    }
    printf("\n");

    close(sockfd);
    return 0;
}
