#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// #define PORT 8080
#define BUF_SIZE 1024
#define MAX_CLIENTS 10

typedef struct {
    struct sockaddr_in addr;
    socklen_t addr_len;
    int type; // 1 - участок 1, 2 - участок 2, 3 - участок 3
} client_info;

client_info clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int server_socket = *(int*)arg;
    char buffer[BUF_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        int len = recvfrom(server_socket, buffer, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
        buffer[len] = '\0';
        printf("Recieved: %s\n", buffer);
        pthread_mutex_lock(&client_mutex);

        int client_type = -1;
        for (int i = 0; i< client_count; ++i) {
            if (clients[i].addr.sin_addr.s_addr == client_addr.sin_addr.s_addr && clients[i].addr.sin_port == client_addr.sin_port) {
                client_type = clients[i].type;
                printf("found!");
                break;
            }
        }

        if (client_type == -1) {
            // client_type = (client_type % 3) + 1;
            client_type = buffer[len - 1] - '0';

            clients[client_count].addr = client_addr;
            clients[client_count].addr_len = addr_len;
            clients[client_count].type = client_type;
            client_count++;
        }
        if (client_type == 1) {
            for (int i = 0; i < client_count; i++) {
                if (clients[i].type == 2) {
                    sendto(server_socket, buffer, len, 0, (struct sockaddr*)&clients[i].addr, clients[i].addr_len);
                    break;
                }
            }
        } else if (client_type == 2) {
            for (int i = 0; i < client_count; i++) {
                if (clients[i].type == 3) {
                    sendto(server_socket, buffer, len, 0, (struct sockaddr*)&clients[i].addr, clients[i].addr_len);
                    break;
                }
            }
        } else {
            printf("Quality control done for: %s\n", buffer);
        }

        pthread_mutex_unlock(&client_mutex);
    }

    close(server_socket);
    pthread_exit(NULL);

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int PORT = atoi(argv[1]);
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    for (int i = 0; i < 10; i++) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void*)&server_socket);
    pthread_join(thread_id, NULL);
    close(server_socket);
    return 0;
    }
}
