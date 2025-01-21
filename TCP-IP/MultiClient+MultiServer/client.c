#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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

    // Authentification
    char username[50], password[50];
    int attempt = 0;
    while (attempt<3) {
        memset(buffer, 0, BUFFER_SIZE);

        printf("Username / Nom d'utilisateur : ");
        scanf("%s", username);
        printf("Password / Mot de passe : ");
        scanf("%s", password);

        snprintf(buffer, BUFFER_SIZE, "%s:%s", username, password);
        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);

        // Vérification de l'authentification -- Authentication verification
        recv(sock, buffer, BUFFER_SIZE-1, 0);
        buffer[BUFFER_SIZE] = '\0';
        if (strcmp(buffer, "AUTH_OK") != 0) {
            printf("Invalid credentials / Authentification échouée.\n");
            attempt++;
            printf("Tentative/Attempt n°: %d \n", attempt);
            if (attempt == 3){
                printf("Nombre de tentatives est dépassé / Number of attempts is exceeded :\n");
                close(sock);
                exit(EXIT_FAILURE);
            }
        }
        else {
            printf("Successful authentication / authentification réussie.\n");
            break;
        }

    }


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
        // Send choice to load balancer
        send(sock, buffer, strlen(buffer), 0);
        if (strcmp(buffer, "3") == 0){
            char filename[250];
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = read(sock, buffer, BUFFER_SIZE);
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
            scanf("%s", filename);
            if (send(sock, filename, strlen(filename), 0) <= 0){
                perror("Error occurred while trying to send filename to server !\n");
                continue;
            }
        }
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(sock, buffer, BUFFER_SIZE);
        // buffer[bytes_received] = '\0';
        printf("Server replied: %s\n", buffer);
    }

    close(sock);
    return 0;
}
