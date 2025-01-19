#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 8666
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

// Function declarations
void *handle_client(void *client_socket);
void send_response(int client_sock, const char *message);
void list_files(int client_sock);
void send_file_content(int client_sock, const char *filename);

// Authentication function (mock for simplicity)
int authenticate(const char *credentials) {
    // Example: username:password
    return strcmp(credentials, "user:password") == 0;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to the port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    // Accept clients in a loop
    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected\n");

        // Create a new thread to handle the client
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&client_sock) != 0) {
            perror("Thread creation failed");
            close(client_sock);
            continue;
        }

        // Detach thread to automatically reclaim resources
        pthread_detach(thread_id);
    }

    close(server_sock);
    return 0;
}

// Thread function to handle a client
void *handle_client(void *client_socket) {
    int client_sock = *(int *)client_socket;
    char buffer[BUFFER_SIZE];
    char username[50], password[50];
    int authenticated = 0;

    // Authentication
    // for (int attempt = 0; attempt < 3; attempt++) {
    //     memset(buffer, 0, BUFFER_SIZE);
    //     recv(client_sock, buffer, BUFFER_SIZE, 0);

    //     if (authenticate(buffer)) {
    //         send_response(client_sock, "AUTH_OK");
    //         authenticated = 1;
    //         break;
    //     } else {
    //         send_response(client_sock, "AUTH_FAIL");
    //     }
    // }

    // if (!authenticated) {
    //     printf("Client failed authentication. Disconnecting.\n");
    //     close(client_sock);
    //     pthread_exit(NULL);
    // }

    printf("Client authenticated successfully\n");

    // Main communication loop
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0) {
            printf("Client disconnected\n");
            break;
        }

        int choice = atoi(buffer);

        switch (choice) {
            case 1: {
                // Current date and time
                time_t now = time(NULL);
                snprintf(buffer, BUFFER_SIZE, "Current date and time: %s", ctime(&now));
                send_response(client_sock, buffer);
                break;
            }
            case 2:
                // List files
                list_files(client_sock);
                break;

            case 3: {
                // Read a specific file
                memset(buffer, 0, BUFFER_SIZE);
                recv(client_sock, buffer, BUFFER_SIZE, 0); // Receive filename
                send_file_content(client_sock, buffer);
                break;
            }
            case 4: {
                // Elapsed time (mock implementation)
                snprintf(buffer, BUFFER_SIZE, "Elapsed time: Feature not yet implemented");
                send_response(client_sock, buffer);
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

// List files in the current directory
void list_files(int client_sock) {
    DIR *dir;
    struct dirent *entry;
    char buffer[BUFFER_SIZE] = "Files:\n";

    if ((dir = opendir(".")) == NULL) {
        send_response(client_sock, "Failed to list files");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Only regular files
            strcat(buffer, entry->d_name);
            strcat(buffer, "\n");
        }
    }

    closedir(dir);
    send_response(client_sock, buffer);
}

// Send the content of a specific file
void send_file_content(int client_sock, const char *filename) {
    FILE *file = fopen(filename, "r");
    char buffer[BUFFER_SIZE];

    if (file == NULL) {
        snprintf(buffer, BUFFER_SIZE, "File '%s' not found", filename);
        send_response(client_sock, buffer);
        return;
    }

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        send(client_sock, buffer, strlen(buffer), 0);
    }

    fclose(file);
}
