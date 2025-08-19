#include "tcp_func.h"



int tcp_create_socket(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void tcp_setup_ip_addr(struct sockaddr_in* server_addr, int port , char* ip_addr) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);
    server_addr->sin_addr.s_addr = inet_addr(ip_addr);//INADDR_ANY;
}

void tcp_bind(int sockfd, struct sockaddr_in* server_addr) {
    if (bind(sockfd, (struct sockaddr*)server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void tcp_listen(int sockfd) {
    if (listen(sockfd, 3) < 0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

int tcp_accept(int sockfd, struct sockaddr_in* client_addr) {
    socklen_t client_len = sizeof(*client_addr);
    int client_sock = accept(sockfd, (struct sockaddr *)client_addr, &client_len);
    if (client_sock < 0) {
        perror("Accept failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return client_sock;
}

void tcp_connect(int sockfd, struct sockaddr_in* server_addr) {
    if (connect(sockfd, (struct sockaddr*)server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void tcp_client_setup_ip_addr(struct sockaddr_in* server_addr, int port , char* ip_addr) {
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);
    inet_pton(AF_INET, ip_addr, &(server_addr->sin_addr));
}