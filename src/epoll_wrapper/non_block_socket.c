#include "non_block_socket.h"





int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }
    return 0;
}


// Перевод сокета обратно в блокирующий режим
int set_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL failed");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL blocking failed");
        return -1;
    }
    printf("Сокет %d переведен в блокирующий режим.\n", sockfd);
    return 0;
}
// void set_use_setsockopt(int listen_sock, ){
//     if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//         perror("setsockopt");
//         close(listen_sock);
//         exit(EXIT_FAILURE);
//     }
// }


int create_epol() {
    int fd;
    fd = epoll_create1(0); // 0 - флаги, обычно 0
    if (fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    return fd;
}


void control_interes_epoll(struct epoll_event *ev, int epoll_fds, int interes_sock, uint32_t interes_event) {
    ev->events = interes_event; //EPOLLIN; // Интересуемся событием "готов к чтению"
    ev->data.fd = interes_sock; // Связываем событие с дескриптором
    if (epoll_ctl(epoll_fds, EPOLL_CTL_ADD, interes_sock, ev) == -1) {
        perror("epoll_ctl: listen_sock");
        close(interes_sock);
        close(epoll_fds);
        exit(EXIT_FAILURE);
    }
}

