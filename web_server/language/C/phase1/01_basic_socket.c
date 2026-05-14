/*
========================================================
Phase 1: Basic TCP Web Server in C
--------------------------------------------------------
INPUT  : index.html
OUTPUT : Access from browser at http://localhost:8080

Source:
web_server/language/C/phase1/main.c
========================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 4096
#define HTML_FILE "source.html"

int main(){
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    int server_fd; 
    int client_fd;
    
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr; 

    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    // ============================================
    // Open socket
    // ============================================
    server_fd = socket(AF_INET,SOCK_STREAM,0);

    if(server_fd < 0)
    {
        perror("socket failed");
        return 1;
    } 
    printf("[INFO] Socket created\n");
    // ============================================
    // Configure server address
    // ============================================
     memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    // ============================================
    // Bind socket
    // ============================================
    if(bind(server_fd, (struct sockaddr*)&server_addr,sizeof(server_addr))< 0){
            perror("bind failed");
        return 1;
    }
     printf("[INFO] Bind success on port %d\n", PORT);
    // ============================================
    // Listen
    // ============================================
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        return 1;
    }

    printf("[INFO] Server listening at http://localhost:%d\n", PORT);
    // ============================================
    // Main loop
    // ============================================
    while(1)
    {
        client_fd = accept(
            server_fd,
            (struct sockaddr*)&client_addr,
            &client_len
        );

        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        printf("[INFO] Client connected\n");
        // ============================================
        // Read request
        // ============================================
        int received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (received > 0) {
            buffer[received] = '\0';
        }

        printf("---------- REQUEST ----------\n");
        printf("%s\n", buffer);
                // ============================================
        // Open HTML file
        // ============================================
        FILE *file = fopen(HTML_FILE, "rb");

        if (!file) {

            const char *not_found =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "404 File Not Found";

            send(client_fd, not_found, strlen(not_found), 0);

        } else {

            // Read HTML content
            fseek(file, 0, SEEK_END);
            size_t file_size = ftell(file);
            rewind(file);

            char *html = (char*)malloc(file_size + 1);

            size_t bytes_read = fread(html, 1, file_size, file);

            if (bytes_read != file_size) {
                perror("fread failed");
            }
            html[file_size] = '\0';

            fclose(file);

            // ============================================
            // HTTP response header
            // ============================================
            char header[512];

            sprintf(
                header,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %ld\r\n"
                "\r\n",
                file_size
            );

            // ============================================
            // Push file into socket
            // ============================================
            send(client_fd, header, strlen(header), 0);
            send(client_fd, html, file_size, 0);

            free(html);

            printf("[INFO] HTML file sent\n");
    }
#ifdef _WIN32
        closesocket(client_fd);
#else
        close(client_fd);
#endif
    }

#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif

    return 0;
}