#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

void *handle_client(void *client_socket);
void send_response(int client_sock, const char *message);
void list_files(int client_sock);
void send_file_content(int client_sock, const char *filename);
char* handle_client_service4(long start_seconds);
// fonction d'authentification
int authenticate(const char *credentials) {
    return strcmp(credentials, "user:password") == 0;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    // socket serveur
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration addresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // association socket au port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("échec");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Écoute des connexions
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    // accepter clients
    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Nouveau client connecté\n");

        // créer nouveau thread
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&client_sock) != 0) {
            perror("Création de thread a échoué");
            close(client_sock);
            continue;
        }

        // détacher thread
        pthread_detach(thread_id);
    }

    close(server_sock);
    return 0;
}

// handler thread client
void *handle_client(void *client_socket) {
    int client_sock = *(int *)client_socket;
    char buffer[BUFFER_SIZE];
    char username[50], password[50];
    int authenticated = 0;
    struct timeval start_time;

    long start_seconds = start_time.tv_sec;

    // Boucle d'Authentication
    for (int attempt = 0; attempt < 3; attempt++) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_sock, buffer, BUFFER_SIZE, 0);

        if (authenticate(buffer)) {
            send_response(client_sock, "AUTH_OK");
            authenticated = 1;
            break;
        } else {
            send_response(client_sock, "AUTH_FAIL");
        }
    }

    if (!authenticated) {
        printf("Déconnexion suite à échec d'authentification.\n");
        close(client_sock);
        pthread_exit(NULL);
    }

    printf("Client authentifié avec succès\n");

    // Main communication loop
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Client déconnecté\n");
            break;
        }

        int choice = atoi(buffer);

        switch (choice) {
            case 1: {
                time_t now = time(NULL);
                snprintf(buffer, BUFFER_SIZE, "Heure et date actuelle: %s", ctime(&now));
                send_response(client_sock, buffer);
                break;
            }
            case 2:
                list_files(client_sock);
                break;

            case 3: {
                memset(buffer, 0, BUFFER_SIZE);
                recv(client_sock, buffer, BUFFER_SIZE, 0); // reception du nom de fichier sollicité
                send_file_content(client_sock, buffer);
                break;
            }
            case 4: {
                const char *elapsed_time_msg = handle_client_service4(start_seconds);
                send_response(client_sock, elapsed_time_msg);
                break;
    
                // snprintf(buffer, BUFFER_SIZE, "Elapsed time: Feature not yet implemented");
                // send_response(client_sock, buffer);
                break;
            }
            case 5:
                // Exit
                printf("Client requested to exit\n");
                close(client_sock);
                pthread_exit(NULL);
                break;

            default:
                send_response(client_sock, "Invalid choice");
        }
    }

    close(client_sock);
    pthread_exit(NULL);
}

// Helper function to send a response to the client
void send_response(int client_sock, const char *message) {
    send(client_sock, message, strlen(message), 0);
}

// fonction 2
void list_files(int client_sock) {
    DIR *dir;
    struct dirent *entry;
    char buffer[BUFFER_SIZE] = "Files:\n";

    if ((dir = opendir(".")) == NULL) {
        send_response(client_sock, "Echec");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            strcat(buffer, entry->d_name);
            strcat(buffer, "\n");
        }
    }

    closedir(dir);
    send_response(client_sock, buffer);
}

// fonction 3
void send_file_content(int client_sock, const char *filename) {
    FILE *file = fopen(filename, "r");
    char buffer[BUFFER_SIZE];

    if (file == NULL) {
        snprintf(buffer, BUFFER_SIZE, "Fichier '%s' introuvable", filename);
        send_response(client_sock, buffer);
        return;
    }

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        send(client_sock, buffer, strlen(buffer), 0);
    }

    fclose(file);
}

// fonction 4
char* handle_client_service4(long start_seconds) {
    static char buffer[BUFFER_SIZE];
    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    long elapsed = end_time.tv_sec - start_seconds;
    snprintf(buffer, BUFFER_SIZE, "Temps écoulé: %ld secondes\n", elapsed);
    return buffer;
}
