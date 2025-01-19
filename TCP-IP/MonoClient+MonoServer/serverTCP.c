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

// Service 1: Current date and time
char* handle_client_service1() {
    static char buffer[BUFFER_SIZE];
    time_t now = time(NULL);
    snprintf(buffer, BUFFER_SIZE, "Current date and time: %s", ctime(&now));
    return buffer;
}

// Service 2: List of files in the current directory
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

// Service 3: Content of a specified file
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

// Service 4: Elapsed time since connection
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
    
    // Create server socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        // Accept client connection
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");
        struct timeval start_time;
        gettimeofday(&start_time, NULL);

        // Authentication
        int authenticated = 0;
        for (int attempt = 0; attempt < 3; ++attempt) {
            memset(buffer, 0, BUFFER_SIZE);
            recv(client_sock, buffer, BUFFER_SIZE, 0);

            if (strcmp(buffer, USERNAME ":" PASSWORD) == 0) {
                authenticated = 1;
                send(client_sock, "AUTH_OK\n", 8, 0);
                printf("Client authenticated successfully.\n");
                break;
            } else {
                send(client_sock, "AUTH_FAILED\n", 12, 0);
                printf("Invalid credentials. Attempt %d/3\n", attempt + 1);
            }
        }

        if (!authenticated) {
            printf("Authentication failed. Closing connection.\n");
            close(client_sock);
            continue;
        }

        // Handle services
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
                printf("Client disconnected.\n");
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
                printf("Client chose to exit.\n");
                break;
            } else {
                response = "Invalid option.\n";
            }

            send(client_sock, response, strlen(response), 0);
        }

        close(client_sock);
        printf("Client connection closed.\n");
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
