#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
// Pour le multithread -- Norme POSIX
#include <pthread.h>

#define BUFFER_SIZE 1024
#define MAX_SERVERS 10

// Login credentials -- Données pour l'authentification.
#define USERNAME "admin"
#define PASSWORD "password"

// Structure to pass multiple parameters to the thread
typedef struct {
    // Client Socket (after accepting the client connexion)
    int client_sock;
    int *server_ports; // All available server ports to use
    int server_index; // Current server index
    int num_servers;
} thread_data_t;

// La routine qui sera lancée et exécutée qui est créée par le load balancer à chaque réception d'une nouvelle connexion client
void *handle_client(void *arg) {
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    time_t start_seconds = start_time.tv_sec;


    thread_data_t *data = (thread_data_t *)arg;  // Cast to the correct type

    int client_sock = data->client_sock;
    int *server_ports = data->server_ports;
    int server_index = data->server_index;
    int num_servers = data->num_servers;

    free(data);  // Free the allocated memory for thread data structure    free(arg);

    // Connect to the next server in round-robin
    // int server_port = server_ports[server_index];
    // server_index = (server_index + 1) % num_servers;

    // Authentification
    int attempt = 0;
    char client_auth[BUFFER_SIZE]; // BUFFER pour l'authentification.
    while(attempt<3){
        memset(client_auth,0,BUFFER_SIZE);
        if (recv(client_sock, client_auth, BUFFER_SIZE, 0) <=0) {
            printf("Client disconnected\n");
            close(client_sock);
            exit(EXIT_FAILURE);
        }
        if (strcmp(client_auth, USERNAME ":" PASSWORD) != 0)
        {
            attempt++;
            send(client_sock, "AUTH_FAILED", strlen("AUTH_FAILED"), 0);
            if (attempt == 3){
                close(client_sock);
                exit(EXIT_FAILURE);
            }
        }
        else {
            send(client_sock, "AUTH_OK", strlen("AUTH_OK"), 0);
            break;
        }

    }
    // END Authentication

    // Client choice 1, 2 , ... 5
    // Read initial choice from client
    char choice_buffer[2];  // Buffer for the choice character and null terminator
    
    int server_sock;
    struct sockaddr_in server_addr;


    server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(server_ports[choice-1]);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int first_iteration = 1;

    // Create all the server sockets before the while loop and connect to them.
    int allServers[num_servers];
    for (int i=0; i<num_servers;i++){
        if ( (allServers[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket Server n° : %d failed",i+1);
            perror("Socket Server failed");
            close(client_sock);
            exit(EXIT_FAILURE);
        }
        server_addr.sin_port = htons(server_ports[i]);
        if (connect(allServers[i], (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
            printf("Server n° : %d connection failed",i+1);
            perror("Server connection failed");
            close(client_sock);
            close(server_sock);
            exit(EXIT_FAILURE);   
        }
    }

    // Forward data between client and server
    fd_set fds;
    char buffer[BUFFER_SIZE];
    char temp_buffer[BUFFER_SIZE];

    while (1) {
        memset(temp_buffer, 0, BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);

        // First, read the choice from client
        int bytes_read = recv(client_sock, choice_buffer, 1, 0);
        if (bytes_read <= 0) {
            printf("Client disconnected\n");
            break;
        }

        choice_buffer[1] = '\0';
        int choice = choice_buffer[0] - '0';
        printf("\n=== New Request ===\n");
        printf("RECEIVED CHOICE IS: %d\n", choice);

        if (choice == 5) {
            printf("Client quit\n");
            break;
        }

        if (choice >= 1 && choice <= 4) {
            int server_sock = allServers[choice-1];
            
            // Send to server
            int send_result;
            send_result = send(server_sock, &choice_buffer, 1, 0);
            if (send_result < 0) {
                perror("Failed to send to server");
                break;
            }
            size_t total_bytes = 0;

    if (choice == 4) {
        printf("CHOICE IS 4 :#######\n");
        
        // Create separate buffers for confirmation and response
        char confirm_buffer[BUFFER_SIZE];
        char response_buffer[BUFFER_SIZE];
        memset(confirm_buffer, 0, BUFFER_SIZE);
        memset(response_buffer, 0, BUFFER_SIZE);

        // First read: Get confirmation message
        bytes_read = recv(server_sock, confirm_buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) {
            printf("Server 4 didn't confirm the message\n");
            break;
        }
        printf("Confirmation message from server 4 is: %s\n", confirm_buffer);

        // Send the start time
        send_result = send(server_sock, &start_seconds, sizeof(time_t), 0);
        if (send_result < 0) {
            perror("Failed to send to server");
            break;
        }
        printf("Successfully sent the starting time to server 4: %ld\n", start_seconds);

        // Wait a small amount of time for the server to process
        usleep(100000);  // 100ms delay

        // Second read: Get the actual response
        size_t total_bytes = 0;
        int attempts = 0;
        int max_attempts = 10;

        while (attempts < max_attempts) {
            bytes_read = recv(server_sock, temp_buffer, BUFFER_SIZE - total_bytes - 1, MSG_DONTWAIT);
            
            if (bytes_read > 0) {
                memcpy(response_buffer + total_bytes, temp_buffer, bytes_read);
                total_bytes += bytes_read;
                response_buffer[total_bytes] = '\0';
                
                // Check if we have a complete message
                if (strchr(response_buffer, '\n') != NULL) {
                    break;
                }
                memset(temp_buffer, 0, BUFFER_SIZE);
            } 
            else if (bytes_read == 0) {
                printf("Server closed connection\n");
                break;
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Error reading from server");
                break;
            }
            
            usleep(50000);
            attempts++;
        }

        printf("Read %zu bytes after %d attempts\n", total_bytes, attempts);
        
        // Only send the response (not the confirmation) to the client
        if (total_bytes > 0) {
            printf("Server response: %s\n", response_buffer);
            
            int send_bytes = send(client_sock, response_buffer, total_bytes, 0);
            if (send_bytes < 0) {
                perror("Failed to send to client");
                break;
            }
            printf("Sent %d bytes to client\n", send_bytes);
        } else {
            printf("Warning: No data received from server after %d attempts\n", attempts);
        }
    }


            // Clear buffers
            memset(buffer, 0, BUFFER_SIZE);
            memset(temp_buffer, 0, BUFFER_SIZE);
            
            // Read response with a small delay between attempts
            int attempts = 0;
            int max_attempts = 10;  // Maximum number of read attempts
            
            while (attempts < max_attempts) {
                bytes_read = recv(server_sock, temp_buffer, BUFFER_SIZE - total_bytes - 1, MSG_DONTWAIT);
                
                if (bytes_read > 0) {
                    memcpy(buffer + total_bytes, temp_buffer, bytes_read);
                    total_bytes += bytes_read;
                    buffer[total_bytes] = '\0';
                    
                    // Check if we have a complete message
                    if (strchr(buffer, '\n') != NULL) {
                        break;
                    }
                } 
                else if (bytes_read == 0) {
                    printf("Server closed connection\n");
                    break;
                }
                else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("Error reading from server");
                    break;
                }
                
                usleep(50000);  // Wait 50ms between attempts
                attempts++;
            }

            printf("Read %zu bytes after %d attempts\n", total_bytes, attempts);
            
            // Send response from server to client 
            if (total_bytes > 0) {
                printf("Server response: %s \n", buffer);
                
                // Send response to client
                int send_bytes = send(client_sock, buffer, total_bytes, 0);
                if (send_bytes < 0) {
                    perror("Failed to send to client");
                    break;
                }
                printf("Sent %d bytes to client\n", send_bytes);
            } else {
                printf("Warning: No data received from server after %d attempts\n", attempts);
            }
        }
    }

    printf("Cleaning up and closing connections...\n");
    close(client_sock);
    for(int i = 0; i < num_servers; i++) {
        close(allServers[i]);
    }
    printf("Thread terminated\n");

    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <load_balancer_port> <server1_port> [server2_port ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int lb_port = atoi(argv[1]);
    int server_ports[MAX_SERVERS];
    int num_servers = argc - 2;

    for (int i = 0; i < num_servers; i++) {
        server_ports[i] = atoi(argv[i + 2]);
    }

    int server_index = 0;
    int lb_sock, client_sock;
    struct sockaddr_in lb_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create load balancer socket
    if ((lb_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(lb_port);

    // Bind load balancer socket
    if (bind(lb_sock, (struct sockaddr *)&lb_addr, sizeof(lb_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(lb_sock, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Load Balancer running on port %d\n", lb_port);

    // Main loop to accept and forward clients
    while (1) {

        // Allocation of a File Descripter (socket)
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(lb_sock, (struct sockaddr *)&client_addr, &client_len);

        if (*client_sock < 0) {
            perror("Accept failed");
            free(client_sock);
            continue;
        }

        printf("New client connected. Creating a new thread and redirecting...\n");

        thread_data_t *thread_data = malloc(sizeof(thread_data_t));
        thread_data->client_sock = *client_sock;
        thread_data->server_ports = server_ports;
        thread_data->server_index = server_index;
        thread_data->num_servers = num_servers;
        // Creating a new thread to handle the client -- Création d'un nouveau thread pour communiquer avec le client.
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, thread_data);
        pthread_detach(client_thread);  // Detach the thread to avoid join overhead
    }

    close(lb_sock);
    return 0;
}
