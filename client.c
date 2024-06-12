#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

// #define PORT 8080
#define BUF_SIZE 1024

void communicate_with_server(int socket_desc, struct sockaddr_in server_addr, int client_type) {
    char message[BUF_SIZE];
    char server_reply[BUF_SIZE];
    socklen_t server_addr_size = sizeof(server_addr);

    // Send greeting to server
    snprintf(message, BUF_SIZE, "Hello from section %d", client_type);
    if (sendto(socket_desc, message, strlen(message), 0, (struct sockaddr*)&server_addr, server_addr_size) < 0) {
        perror("Send failed");
        return;
    }

    // Wait for server acknowledgment
    int len = recvfrom(socket_desc, server_reply, BUF_SIZE, 0, NULL, NULL);
    server_reply[len] = '\0';
    printf("Server response: %s\n", server_reply);

    while (1) {
        if (client_type == 1) {
            // Section 1: generate new pin
            snprintf(message, BUF_SIZE, "Pin from section 1");
        } else {
            // Section 2 and 3: receive pin from previous section
            len = recvfrom(socket_desc, server_reply, BUF_SIZE, 0, NULL, NULL);
            server_reply[len] = '\0';
            printf("Received from server: %s\n", server_reply);

            if (client_type == 2) {
                snprintf(message, BUF_SIZE, "Pin from section 2");
            } else if (client_type == 3) {
                snprintf(message, BUF_SIZE, "Pin from section 3");
            }
        }

        if (sendto(socket_desc, message, strlen(message), 0, (struct sockaddr*)&server_addr, server_addr_size) < 0) {
            perror("Send failed");
            return;
        }

        // Random delay to simulate work time
        sleep(rand() % 3 + 1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <PORT> <client_type>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int PORT = atoi(argv[1]);
    int client_type = atoi(argv[2]);
    int socket_desc;
    struct sockaddr_in server_addr;

    srand(time(NULL));

    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    communicate_with_server(socket_desc, server_addr, client_type);

    close(socket_desc);
    return 0;
}
