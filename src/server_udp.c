// Standarts libs 
#include <unistd.h> // Unix Standard Header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h> // Sokets libs
#include <netinet/in.h> // Internet Protocol Family Header 
#include <arpa/inet.h> // Definitions for internet operations

#include <sys/epoll.h> // Epoll Header
#include <fcntl.h>     // File Control Header

#include <errno.h> // System Error Numbers Header
#include <ctype.h>     // Character Type Header

#include <signal.h>     // interrupt handler

void signal_handler(int sig_num);

#include "udp_wrapper/udp_func.h"
#include "epoll_wrapper/non_block_socket.h"


#define PORT 8085
#define MAX_EVENTS 10 // Максимальное количество событий, возвращаемых epoll_wait

#define MAX_WORD_PILA 12288
#define OFFSET_PLOT (NUMBER_SEND_WORD32_SOCKET)
// #define OFFSET_PLOT (MAX_WORD_PILA / NUMBER_SEND_WORD32_SOCKET)

int *pila;


#define DEVMEM "/dev/mem"
#define CMA_DMA_PL "/dev/cma_dma_pl"

// fds
int udp_sock_fd;
int epoll_fd;
int blockepollwait = -1;

int main(int argc, char **argv) {

    struct sigaction sia;
    // Инициализируем структуру нулями
    memset(&sia, 0, sizeof(sia));

    // Указываем обработчик
    sia.sa_handler = signal_handler;
    // Устанавливаем флаги (обычно 0)
    sia.sa_flags = 0;
    // Очищаем маску блокируемых сигналов (не блокируем ничего дополнительно)
    sigemptyset(&sia.sa_mask);

    // Регистрируем обработчик для SIGINT
    if (sigaction(SIGINT, &sia, NULL) == -1) {
         perror("sigaction");
         return 1;
    }

    printf("Name programm = %s\n", argv[0]);
    printf("Number arguments = %d\n", argc);

    // if (signal(SIGINT, signal_handler) == SIG_ERR) {
    //     perror("Не удалось установить обработчик сигнала");
    //     return 1;
    // }



    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_len;
    int status_sock;
    ssize_t bytes_received;

    udp_sock_fd = udp_create_socket();
    printf("Successfully: Create socket\n");

    udp_setup_ip_addr_serv(&serv_addr, PORT);
    printf("Successfully: Set ip addr server\n");

    udp_bind(udp_sock_fd, &serv_addr);
    printf("Successfully: bind socket to ip addr\n");

    
    status_sock = set_nonblocking(udp_sock_fd);
    // Установка сокета в неблокирующий режим
    if (status_sock == -1) {
         close(udp_sock_fd);
         exit(EXIT_FAILURE);
    } else {
        printf("Successfully: Set non blocking socket\n");
    }
    
    struct epoll_event ev, events_find[MAX_EVENTS];
    int nfds, n;
    
    epoll_fd = create_epol();
    printf("Successfully: Create epoll\n");
    control_interes_epoll(&ev, epoll_fd, udp_sock_fd, EPOLLIN);
    printf("Successfully: Add table interes socket\n");


    //-----------------------------------------------------------------------------------------------------------------
    

    pila = (int *)malloc(sizeof(int) * MAX_WORD_PILA);
    int val_pila = 0;
    uint32_t curent_addr = 0;
    uint32_t save_addr = 0;
    uint32_t begin_addr = 0;
    uint32_t remains;
    for (int i = 0; i < MAX_WORD_PILA; i++) {
        if (val_pila == 50) {
            val_pila = -50;
        }
        pila[i] = val_pila;
        val_pila += 1;
    }
    

    // char bufer_udp_rec[16];
    printf("Successfully: UDP сервер запущен на порту %d\n", PORT);
    for (;;) {
        nfds = epoll_wait(epoll_fd, events_find, MAX_EVENTS, 10); // Блокируем навсегда (-1)
        if (nfds == -1) {
            perror("epoll_wait");
            if (errno == EINTR) continue;
            break;
        }

        for (n = 0; n < nfds; n++) {
            if ((events_find[n].events & EPOLLERR) || (events_find[n].events & EPOLLHUP) || (!(events_find[n].events & EPOLLIN))) { // Проверка на ошибки или обрыв связи
                fprintf(stderr, "epoll error on fd %d\n", events_find[n].data.fd);
                close(events_find[n].data.fd);
                goto cleanup;
            }
            if (events_find[n].data.fd == udp_sock_fd) {
                while (1) {
                    client_len = sizeof(client_addr);
                    bytes_received = recvfrom(udp_sock_fd, (int*)&blockepollwait, sizeof(blockepollwait), MSG_DONTWAIT, (struct sockaddr*)&client_addr, &client_len);
                    if (bytes_received == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror("recvfrom");
                            goto cleanup;
                        }
                    }
                    printf("Принятые данные: 0x%x\n", blockepollwait);
                    blockepollwait = 0;
                }
            }
        }

        if (curent_addr == MAX_WORD_PILA - 1) {
            curent_addr = 0;
        }
        curent_addr++;

        if (curent_addr >= 256) {
            curent_addr -= 256;
        } else {
            curent_addr = (MAX_WORD_PILA - 256) + curent_addr;
        }
        if (curent_addr > save_addr) {
            begin_addr = curent_addr - save_addr;
        } else {
            begin_addr = (MAX_WORD_PILA - save_addr) + curent_addr;
        }
        if (begin_addr >= OFFSET_PLOT) {
            // curent_addr = algner_addr3(curent_addr);
            begin_addr = save_addr;
            // begin_addr = algner_addr3(begin_addr);
            if (begin_addr < curent_addr) {
                remains = curent_addr - begin_addr;
                sendto(udp_sock_fd, (uint32_t*)(pila + begin_addr), remains * sizeof(uint32_t), 0, (struct sockaddr*)&client_addr, client_len);
            } else {
                remains = MAX_WORD_PILA - begin_addr;
                sendto(udp_sock_fd, (uint32_t*)(pila + begin_addr), remains * sizeof(uint32_t), 0, (struct sockaddr*)&client_addr, client_len);
                if (curent_addr >= 3) sendto(udp_sock_fd, (uint32_t*)pila, curent_addr * sizeof(uint32_t), 0, (struct sockaddr*)&client_addr, client_len);
            }
            save_addr = curent_addr;
        }
    //     for (int i = 0; i < MAX_WORD_PILA; i++) {
    //     if (val_pila == 50) {
    //         val_pila = -50;
    //     }
    //     pila[i] = val_pila + (rand() % 50);
    //     val_pila += 1;
    // }
    }

cleanup:
    printf("Завершение работы сервера...\n");
    close(udp_sock_fd);
    close(epoll_fd);
    free(pila);
    return EXIT_SUCCESS;
}



// обработчик сигнала
void signal_handler(int sig_num) {
    printf("\nПолучен сигнал %d (SIGINT - Ctrl+C)\n", sig_num);
    printf("Программа завершает работу...\n");
    printf("Завершение работы сервера...\n");
    close(udp_sock_fd);
    close(epoll_fd);
    free(pila);
    exit(EXIT_SUCCESS); // Завершаем программу корректно
}