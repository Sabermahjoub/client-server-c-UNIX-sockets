#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#define BUFFER_SIZE 1024

//Service 1
char* handle_client_service1() {
    static char buffer[BUFFER_SIZE];  // Use static buffer instead of dynamic allocation
    memset(buffer, 0, BUFFER_SIZE);   // Clear the buffer
    
    time_t now = time(NULL);
    strncpy(buffer, ctime(&now), BUFFER_SIZE - 1);
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

    // Accept and handle client connections
    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");
        
        while (1) {
            // Clear receive buffer
            memset(buffer, 0, BUFFER_SIZE);
            
            // Read client request
            int bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_read <= 0) {
                if (bytes_read == 0) {
                    printf("Client disconnected normally\n");
                } else {
                    perror("recv failed");
                }
                break;
            }

            // Get response
            char* response = handle_client_service1();
            size_t response_len = strlen(response);
            
            printf("Sending response (%zu bytes): %s", response_len, response);
            
            // Send response with actual length
            ssize_t bytes_sent = send(client_sock, response, response_len, 0);
            if (bytes_sent < 0) {
                perror("send failed");
                break;
            }
            printf("Sent %zd bytes to client\n", bytes_sent);
        }

        printf("Closing client connection.\n");
        close(client_sock);
    }

    close(server_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    start_server(port);

    return 0;
}