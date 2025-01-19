#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 1024
#define MAX_SERVERS 10
#define MAX_CLIENTS 100

#define USERNAME "admin"
#define PASSWORD "password"

// Mutex for thread-safe operations
pthread_mutex_t server_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t client_semaphore;

typedef struct {
    int client_sock;
    int *server_ports;
    int server_index;
    int num_servers;
} thread_data_t;

// Connection pool structure
typedef struct {
    int *connections;
    int size;
    pthread_mutex_t mutex;
} connection_pool_t;

connection_pool_t *create_connection_pool(int *server_ports, int num_servers) {
    connection_pool_t *pool = malloc(sizeof(connection_pool_t));
    pool->connections = malloc(sizeof(int) * num_servers);
    pool->size = num_servers;
    pthread_mutex_init(&pool->mutex, NULL);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    for (int i = 0; i < num_servers; i++) {
        pool->connections[i] = socket(AF_INET, SOCK_STREAM, 0);
        server_addr.sin_port = htons(server_ports[i]);
        
        if (connect(pool->connections[i], (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Server connection failed in pool");
            // Handle error more gracefully in production
            close(pool->connections[i]);
            pool->connections[i] = -1;
        }
    }
    
    return pool;
}

int get_server_connection(connection_pool_t *pool, int server_index) {
    pthread_mutex_lock(&pool->mutex);
    int connection = pool->connections[server_index];
    pthread_mutex_unlock(&pool->mutex);
    return connection;
}

void *handle_client(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int client_sock = data->client_sock;
    int *server_ports = data->server_ports;
    int num_servers = data->num_servers;
    
    // Authentication
    int attempt = 0;
    char client_auth[BUFFER_SIZE];
    
    while (attempt < 3) {
        memset(client_auth, 0, BUFFER_SIZE);
        if (recv(client_sock, client_auth, BUFFER_SIZE, 0) <= 0) {
            printf("Client disconnected during auth\n");
            goto cleanup;
        }
        
        if (strcmp(client_auth, USERNAME ":" PASSWORD) != 0) {
            attempt++;
            send(client_sock, "AUTH_FAILED", strlen("AUTH_FAILED"), 0);
            if (attempt == 3) {
                goto cleanup;
            }
        } else {
            send(client_sock, "AUTH_OK", strlen("AUTH_OK"), 0);
            break;
        }
    }

    char choice_buffer[2];
    char buffer[BUFFER_SIZE];
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    time_t start_seconds = start_time.tv_sec;

    // Main client handling loop
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        // Read client choice
        int bytes_read = recv(client_sock, choice_buffer, 1, 0);
        if (bytes_read <= 0) {
            printf("Client disconnected\n");
            break;
        }

        choice_buffer[1] = '\0';
        int choice = choice_buffer[0] - '0';
        
        if (choice == 5) {
            printf("Client quit\n");
            break;
        }

        if (choice >= 1 && choice <= 4) {
            // Get connection from pool for the specific server
            pthread_mutex_lock(&server_mutex);
            int server_sock = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            server_addr.sin_port = htons(server_ports[choice-1]);
            
            if (connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Server connection failed");
                pthread_mutex_unlock(&server_mutex);
                continue;
            }
            pthread_mutex_unlock(&server_mutex);

            // Send choice to server
            if (send(server_sock, &choice_buffer, 1, 0) < 0) {
                perror("Failed to send to server");
                close(server_sock);
                continue;
            }

            // Handle special case for choice 4
            if (choice == 4) {
                if (recv(server_sock, buffer, BUFFER_SIZE, 0) <= 0) {
                    close(server_sock);
                    continue;
                }
                
                if (send(server_sock, &start_seconds, sizeof(time_t), 0) < 0) {
                    close(server_sock);
                    continue;
                }
            }

            // Read server response with timeout
            struct timeval tv;
            tv.tv_sec = 5;  // 5 second timeout
            tv.tv_usec = 0;
            setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            memset(buffer, 0, BUFFER_SIZE);
            bytes_read = recv(server_sock, buffer, BUFFER_SIZE, 0);
            
            if (bytes_read > 0) {
                send(client_sock, buffer, bytes_read, 0);
            }

            close(server_sock);
        }
    }

cleanup:
    close(client_sock);
    free(data);
    sem_post(&client_semaphore);  // Release a client slot
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <load_balancer_port> <server1_port> [server2_port ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Initialize semaphore for client limit
    sem_init(&client_semaphore, 0, MAX_CLIENTS);

    int lb_port = atoi(argv[1]);
    int server_ports[MAX_SERVERS];
    int num_servers = argc - 2;

    for (int i = 0; i < num_servers; i++) {
        server_ports[i] = atoi(argv[i + 2]);
    }

    int lb_sock;
    struct sockaddr_in lb_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create and setup load balancer socket
    if ((lb_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options for reuse
    int opt = 1;
    if (setsockopt(lb_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(lb_port);

    if (bind(lb_sock, (struct sockaddr *)&lb_addr, sizeof(lb_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(lb_sock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Load Balancer running on port %d\n", lb_port);

    while (1) {
        // Wait for a client slot to be available
        sem_wait(&client_semaphore);

        int client_sock = accept(lb_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            sem_post(&client_semaphore);
            continue;
        }

        printf("New client connected\n");

        thread_data_t *thread_data = malloc(sizeof(thread_data_t));
        thread_data->client_sock = client_sock;
        thread_data->server_ports = server_ports;
        thread_data->num_servers = num_servers;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, thread_data) != 0) {
            perror("Thread creation failed");
            free(thread_data);
            close(client_sock);
            sem_post(&client_semaphore);
            continue;
        }
        pthread_detach(client_thread);
    }

    // Cleanup (though this point is never reached in this example)
    close(lb_sock);
    sem_destroy(&client_semaphore);
    pthread_mutex_destroy(&server_mutex);
    
    return 0;
}