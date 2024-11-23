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
#include "protocol.h"

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return EXIT_FAILURE;
    }
    #endif

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[64];
    char password[64];

    // Create the client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);

    #ifdef _WIN32
    // Se InetPton non è disponibile, usa inet_addr
    unsigned long addr = inet_addr(DEFAULT_IP);
    if (addr == INADDR_NONE) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_addr.s_addr = addr;
    #else
    // Su Linux/Unix, usa inet_pton
    if (inet_pton(AF_INET, DEFAULT_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    #endif

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", DEFAULT_IP, DEFAULT_PORT);

    while (1) {
        printf("Enter password type and length (e.g., n 8) or 'q' to quit: ");
        fgets(buffer, sizeof(buffer), stdin);

        // Exit if user enters 'q'
        if (buffer[0] == 'q') {
            break;
        }

        // Send request to server
        send(client_socket, buffer, strlen(buffer), 0);

        // Receive response
        memset(password, 0, sizeof(password));
        recv(client_socket, password, sizeof(password) - 1, 0);

        printf("Generated password: %s\n", password);
    }

    #ifdef _WIN32
    	WSACleanup();
    #endif

    return 0;
}
