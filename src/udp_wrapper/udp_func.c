#include "udp_func.h"


int udp_create_socket(void) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // Используем SOCK_DGRAM для UDP [[8]]
    if (sockfd < 0) {
        perror("Socket creation failed");
        // printf("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Настройка адреса сервера
void udp_setup_ip_addr(struct sockaddr_in *addr, uint16_t port, char* ip_addr) {
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(ip_addr);
}


// Настройка адреса
void udp_setup_ip_addr_serv(struct sockaddr_in *addr, uint16_t port) {
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = INADDR_ANY;
}

// Привязка сокета к адресу (bind)
void udp_bind(int sockfd, struct sockaddr_in *addr) {
    if (bind(sockfd, (struct sockaddr*)addr, sizeof(*addr)) < 0) {  // Исправлен sizeof [[8]]
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}


// void udp_connect(int sockfd, struct sockaddr_in* addr) {
//     if (connect(sockfd, (struct sockaddr*)addr, sizeof(*addr)) < 0) {  // Для установки сервера по умолчанию [[3]]
//         perror("Connect failed");
//         close(sockfd);
//         exit(EXIT_FAILURE);
//     }
// }


void udp_setsockopt(int sockfd, int opt) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR");
        // SO_REUSEPORT может не поддерживаться или не быть нужным для UDP
        // perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

