#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>

#define BUFFER_SIZE 1024
#define USERNAME "admin"
#define PASSWORD "password"

// Service 1: heure actuelle
char* handle_client_service1() {
    static char buffer[BUFFER_SIZE];
    time_t now = time(NULL);
    snprintf(buffer, BUFFER_SIZE, "Heure actuelle: %s", ctime(&now));
    return buffer;
}

// Service 2: lister les fichiers dans un dossier
char* handle_client_service2() {
    static char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    DIR *d = opendir(".");
    if (!d) {
        snprintf(buffer, BUFFER_SIZE, "Error: Unable to open directory.\n");
        return buffer;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        strncat(buffer, dir->d_name, BUFFER_SIZE - strlen(buffer) - 2);
        strcat(buffer, "\n");
    }
    closedir(d);
    return buffer;
}

// Service 3: Afficher contenu d'un fichier specifié
char* handle_client_service3(int client_sock) {
    static char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    char filename[50];
    recv(client_sock, filename, sizeof(filename), 0);
    printf("Requested file: %s\n", filename);

    FILE *file = fopen(filename, "r");
    if (file) {
        fread(buffer, 1, BUFFER_SIZE - 1, file);
        fclose(file);
    } else {
        snprintf(buffer, BUFFER_SIZE, "Error: File '%s' not found.\n", filename);
    }
    return buffer;
}

// Service 4: Temps écoulé depuis connexion
char* handle_client_service4(long start_seconds) {
    static char buffer[BUFFER_SIZE];
    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    long elapsed = end_time.tv_sec - start_seconds;
    snprintf(buffer, BUFFER_SIZE, "Elapsed time: %ld seconds\n", elapsed);
    return buffer;
}

void start_server(int port) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    // socket serveur
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        exit(EXIT_FAILURE);
    }

    // config socket: famille/ addresse et port
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // binding du socket au port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // lancer l'écoute sur socket serveur
    if (listen(server_sock, 5) < 0) { //file d'attente max 5
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Serveur à l'écoute sur le port:  %d\n", port);

    while (1) {
        // Accept client connection
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Client socket non accepté");
            continue;
        }

        printf("Client connecté.\n");
        // temps et date actuelles
        struct timeval start_time;
        gettimeofday(&start_time, NULL);

        // Boucle auth
        int authenticated = 0;
        for (int attempt = 0; attempt < 3; ++attempt) {
            memset(buffer, 0, BUFFER_SIZE);
            recv(client_sock, buffer, BUFFER_SIZE, 0);
            // printf("<<<received password is %s",buffer);
            // printf("<<<<expected password is %s",USERNAME ":" PASSWORD);

            if (strcmp(buffer, USERNAME ":" PASSWORD) == 0) {
                authenticated = 1;
                send(client_sock, "AUTH_OK", 8, 0);
                printf("Client authentifié avec succès.\n");
                break;
            } else {
                send(client_sock, "AUTH_FAILED", 12, 0);
                printf("Echec d'auth. Tentative %d/3\n", attempt + 1);
            }
        }

        if (!authenticated) {
            printf("Auth échouée. Connexion sera fermée.\n");
            close(client_sock);
            continue;
        }

        // Boucle principale / si auth réussie
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
                printf("Client déconnecté.\n");
                break;
            }

            char *response = NULL;

            if (strcmp(buffer, "1") == 0) {
                response = handle_client_service1();
            } else if (strcmp(buffer, "2") == 0) {
                response = handle_client_service2();
            } else if (strcmp(buffer, "3") == 0) {
                send(client_sock, "Send filename\n", 14, 0);
                response = handle_client_service3(client_sock);
            } else if (strcmp(buffer, "4") == 0) {
                response = handle_client_service4(start_time.tv_sec);
            } else if (strcmp(buffer, "5") == 0) {
                printf("Client a quitté.\n");
                break;
            } else {
                response = "Option non valide.\n";
            }

            send(client_sock, response, strlen(response), 0);
        }

        close(client_sock);
        printf("connexion fermée.\n");
    }

    close(server_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %d\n", port);
        exit(EXIT_FAILURE);
    }

    start_server(port);
    return 0;
}
