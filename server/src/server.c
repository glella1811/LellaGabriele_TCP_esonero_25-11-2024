#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
//#pragma comment(lib, "ws2_32.lib") ho inserito la libreria ws2_32 nella lista di librerie del linker MinGW nelle impostazioni del progetto
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "protocol.h"

// Password generation functions
void generate_numeric(char *password, int length);
void generate_alpha(char *password, int length);
void generate_mixed(char *password, int length);
void generate_secure(char *password, int length);

void handle_client(int client_socket);

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return EXIT_FAILURE;
    }
    #endif

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    srand(time(NULL)); // Seed the random number generator

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_socket, QLEN) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d\n", DEFAULT_IP, DEFAULT_PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Print client info
        printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Handle client in a separate function
        handle_client(client_socket);
    }

    // Close the server socket
    close(server_socket);

    #ifdef _WIN32
    WSACleanup();
    #endif

    return 0;
}

void handle_client(int client_socket) {
    char buffer[64];
    char password[MAX_PASSWORD_LENGTH + 1];
    int length;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(password, 0, sizeof(password));

        // Receive client request
        ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            break;
        }

        // Parse request
        char type = buffer[0];
        length = atoi(buffer + 2);

        // Validate length
        if (length < MIN_PASSWORD_LENGTH || length > MAX_PASSWORD_LENGTH) {
            send(client_socket, "Error: Invalid length\n", 23, 0);
            continue;
        }

        // Generate password
        switch (type) {
            case NUMERIC:
                generate_numeric(password, length);
                break;
            case ALPHABETIC:
                generate_alpha(password, length);
                break;
            case MIXED:
                generate_mixed(password, length);
                break;
            case SECURE:
                generate_secure(password, length);
                break;
            default:
                send(client_socket, "Error: Invalid type\n", 20, 0);
                continue;
        }

        // Send password to client
        send(client_socket, password, strlen(password), 0);
    }

    close(client_socket);
}

void generate_numeric(char *password, int length) {
    for (int i = 0; i < length; i++) {
        password[i] = '0' + rand() % 10;
    }
    password[length] = '\0';
}

void generate_alpha(char *password, int length) {
    for (int i = 0; i < length; i++) {
        password[i] = 'a' + rand() % 26;
    }
    password[length] = '\0';
}

void generate_mixed(char *password, int length) {
    for (int i = 0; i < length; i++) {
        if (rand() % 2) {
            password[i] = 'a' + rand() % 26;
        } else {
            password[i] = '0' + rand() % 10;
        }
    }
    password[length] = '\0';
}

void generate_secure(char *password, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
    int charset_size = sizeof(charset) - 1;
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % charset_size];
    }
    password[length] = '\0';
}
