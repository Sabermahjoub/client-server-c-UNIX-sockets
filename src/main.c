#include "./pages/login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <load_balancer_ip> <load_balancer_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *lb_ip = argv[1];
    int lb_port = atoi(argv[2]);
    int sock;
    struct sockaddr_in lb_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    lb_addr.sin_family = AF_INET;
    lb_addr.sin_port = htons(lb_port);
    if (inet_pton(AF_INET, lb_ip, &lb_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Connect to load balancer
    if (connect(sock, (struct sockaddr *)&lb_addr, sizeof(lb_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to load balancer on port %d\n", lb_port);

    int authentication = authenticate(sock);
    
    // Unsuccefsull authentication
    if (authentication == 0) exit(EXIT_FAILURE);

    // Communicate with server through load balancer
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        printf("Enter your choice / Votre choix : ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer,"1") != 0 && strcmp(buffer,"2") != 0 && strcmp(buffer,"3") != 0 && strcmp(buffer,"4") != 0 && strcmp(buffer,"5") != 0 ){
            printf("Invalid input of choice / choix invalide :\n");
            continue;
        }
        send(sock, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(sock, buffer, BUFFER_SIZE);
        buffer[bytes_received] = '\0';
        printf("Server replied: %s\n", buffer);
    }

    close(sock);
    return 0;
}
