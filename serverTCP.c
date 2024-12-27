#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define USERNAME "admin"
#define PASSWORD "password"

void handle_client(int client_sock)
{
    char buffer[BUFFER_SIZE];
    char client_auth[BUFFER_SIZE];
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Authentification
    recv(client_sock, client_auth, BUFFER_SIZE, 0);
    if (strcmp(client_auth, USERNAME ":" PASSWORD) != 0)
    {
        send(client_sock, "AUTH_FAILED", strlen("AUTH_FAILED"), 0);
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    send(client_sock, "AUTH_OK", strlen("AUTH_OK"), 0);

    while (1)
    {
        int choice;
        recv(client_sock, &choice, sizeof(choice), 0);

        if (choice == 5)
            break;

        switch (choice)
        {
        case 1:
        { // Date et heure
            time_t now = time(NULL);
            snprintf(buffer, BUFFER_SIZE, "%s", ctime(&now));
            send(client_sock, buffer, strlen(buffer), 0);
            break;
        }

        case 2:
        { // Liste des fichiers
            DIR *d;
            struct dirent *dir;
            d = opendir(".");
            if (d)
            {
                buffer[0] = '\0';
                while ((dir = readdir(d)) != NULL)
                {
                    strcat(buffer, dir->d_name);
                    strcat(buffer, "\n");
                }
                closedir(d);
            }
            send(client_sock, buffer, strlen(buffer), 0);
            break;
        }

        case 3:
        { // Contenu d'un fichier
            char filename[50];
            recv(client_sock, filename, sizeof(filename), 0);
            FILE *file = fopen(filename, "r");
            if (file)
            {
                fread(buffer, 1, BUFFER_SIZE, file);
                fclose(file);
            }
            else
            {
                snprintf(buffer, BUFFER_SIZE, "Erreur : Fichier introuvable.\n");
            }
            send(client_sock, buffer, strlen(buffer), 0);
            break;
        }

        case 4:
        { // Durée écoulée
            gettimeofday(&end_time, NULL);
            long seconds = end_time.tv_sec - start_time.tv_sec;
            snprintf(buffer, BUFFER_SIZE, "%ld secondes", seconds);
            send(client_sock, buffer, strlen(buffer), 0);
            break;
        }

        default:
            snprintf(buffer, BUFFER_SIZE, "Choix invalide.");
            send(client_sock, buffer, strlen(buffer), 0);
        }
    }

    close(client_sock);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_port = atoi(argv[1]);
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
}


int monoserveurMonoclient() {
    // Configuration du serveur similaire au code précédent...
    int client_sock;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    printf("Serveur TCP en écoute...\n");

    // Acceptation d'une seule connexion
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("Connexion acceptée.\n");

    // Gestion des interactions avec le client
    handle_client(client_sock);

    close(client_sock);
    close(server_sock);
    return 0;
}

#include <pthread.h>

void *client_handler(void *client_sock_ptr) {
    int client_sock = *(int *)client_sock_ptr;
    free(client_sock_ptr);
    handle_client(client_sock);
    close(client_sock);
    return NULL;
}

int multiClientMonoServer() {
    //  le serveur doit gérer plusieurs connexions clients simultanément. Cela nécessite l’utilisation de processus enfants (fork()), des threads, ou d’un multiplexage d’E/S comme select() ou poll().
    // Configuration du serveur similaire...
    while (1) {
        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (*client_sock_ptr < 0) {
            perror("accept");
            free(client_sock_ptr);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_handler, client_sock_ptr);
        pthread_detach(thread_id); // Permet de libérer les ressources automatiquement
    }

    close(server_sock);
    return 0;
}


// // multi client multi serveur: use load balancer
// Approche avec plusieurs serveurs :
// Chaque serveur peut écouter sur un port différent, et les clients peuvent choisir un serveur basé sur des critères spécifiques (exemple : charge, localisation).

// Exemple basique :

// Serveur principal (load balancer) redirige les connexions vers un serveur secondaire.
// Les clients se connectent au serveur principal, qui transfère leur requête à un serveur secondaire.

int multiClientMultiServer() {
    // Configuration similaire au serveur principal...
    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        

        // Redirection vers un serveur secondaire
        int backend_sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in backend_addr = {0};
        backend_addr.sin_family = AF_INET;
        backend_addr.sin_port = htons(BACKEND_PORT); // Port d'un serveur secondaire
        inet_pton(AF_INET, "127.0.0.1", &backend_addr.sin_addr);
connect(backend_sock, (struct sockaddr *)&backend_addr, sizeof(backend_addr));
       
        // Transfert des données entre client et serveur secondaire
        proxy_data(client_sock, backend_sock);
    }
}

// Monoclient/Monoserveur : Simplifie la gestion mais ne supporte qu’un client à la fois.
// Multiclient/Monoserveur : Nécessite parallélisme (processus ou threads) ou multiplexage.
// Multiclient/Multiserveur : Ajoute des serveurs secondaires ou un mécanisme de répartition de charge pour gérer plusieurs connexions efficacement.
