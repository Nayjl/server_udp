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
#include "driver_linux_map/mapping_memory.h"
#include "config_fpga.h"
#include "config_cma.h"


#define PORT 8085
#define MAX_EVENTS 10 // Максимальное количество событий, возвращаемых epoll_wait


#pragma pack(push, 1)
struct protocol_command_udp {
    uint8_t command;
    size_t quantity_byte;
    uint32_t data_client;
};

#pragma pack(pop)

struct config_cma cma_dma = {.speed_sample = 500000, .sizeshot = 1, .number_chennal = 4, .calculate_size_byte_ptr = calculate_size_byte};
uint32_t *cma_ddr;
unsigned long phys_addr_cma_ddr;

uint32_t number_word;
uint32_t current_address_read = 0;
uint32_t begin_addr;
uint32_t remains;


void calculating_address(size_t number_byte);


#define DEVMEM "/dev/mem"
#define CMA_DMA_PL "/dev/cma_dma_pl"

// fds
int udp_sock;
int epoll_fd;
int fd_mem;
int fd_cma;

int main(int argc, char **argv) {

    // struct sigaction sa;
    // // Инициализируем структуру нулями
    // memset(&sa, 0, sizeof(sa));

    // // Указываем обработчик
    // sa.sa_handler = signal_handler;
    // // Устанавливаем флаги (обычно 0)
    // sa.sa_flags = 0;
    // // Очищаем маску блокируемых сигналов (не блокируем ничего дополнительно)
    // sigemptyset(&sa.sa_mask);

    // // Регистрируем обработчик для SIGINT
    // if (sigaction(SIGINT, &sa, NULL) == -1) {
    //      perror("sigaction");
    //      return 1;
    // }

    printf("Name programm = %s\n", argv[0]);
    printf("Number arguments = %d\n", argc);

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Не удалось установить обработчик сигнала");
        return 1;
    }


    // int udp_sock;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_len;
    int status_sock;
    ssize_t bytes_received;
    struct protocol_command_udp proto_upd;
    
    // int epoll_fd;
    struct epoll_event ev, events_find[MAX_EVENTS];

    int nfds, n;

    printf("Create socket\n");
    udp_sock = udp_create_socket();

    printf("Set ip addr server\n");
    udp_setup_ip_addr_serv(&serv_addr, PORT);

    printf("bind socket to ip addr\n");
    udp_bind(udp_sock, &serv_addr);

    printf("Set non blocking socket\n");
    status_sock = set_nonblocking(udp_sock);
    // Установка сокета в неблокирующий режим
    if (status_sock == -1) {
         close(udp_sock);
         exit(EXIT_FAILURE);
    }
    printf("UDP сервер запущен на порту %d\n", PORT);

    printf("Create epoll\n");
    epoll_fd = create_epol();

    printf("Add table interes socket\n");
    control_interes_epoll(ev, epoll_fd, udp_sock, EPOLLIN);


    //-----------------------------------------------------------------------------------------------------------------
    fd_mem = open(DEVMEM, O_RDWR | O_SYNC);
    fd_cma = open(CMA_DMA_PL, O_RDWR | O_SYNC);


    
    cma_dma.calculate_size_byte_ptr(&cma_dma);
    DMAmemory_alloc(fd_cma, &(cma_dma.size_ddr));
    DMAmemory_get_phys_addr(fd_cma, &phys_addr_cma_ddr);
    cma_ddr = (uint32_t*)maping(fd_cma, phys_addr_cma_ddr, cma_dma.size_ddr);



    for (;;) {
        nfds = epoll_wait(epoll_fd, events_find, MAX_EVENTS, -1); // Блокируем навсегда (-1)
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

            if (events_find[n].data.fd == udp_sock) {
                while (1) {
                    client_len = sizeof(client_addr);
                    bytes_received = recvfrom(udp_sock, (struct protocol_command_udp*)&proto_upd, sizeof(proto_upd), MSG_WAITALL, (struct sockaddr*)&client_addr, &client_len);
                    if (bytes_received == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror("recvfrom");
                            goto cleanup;
                        }
                    }

                    switch (proto_upd.command) {
                    case 0x0: {
                            current_address_read = 0;
                            calculating_address(proto_upd.quantity_byte);
                            if (begin_addr < current_address_read) {
                                remains = current_address_read - begin_addr;

                                proto_upd.command = 0x0;
                                proto_upd.quantity_byte = sizeof(uint32_t) * remains;
                                status_sock = sendto(udp_sock, (struct protocol_command_udp*)&proto_upd, sizeof(proto_upd), 0, (struct sockaddr*)&client_addr, client_len);
                                if (status_sock == -1) {
                                    perror("sendto");
                                }
                                status_sock = sendto(udp_sock, (uint32_t*)(cma_ddr + begin_addr), proto_upd.quantity_byte, 0, (struct sockaddr*)&client_addr, client_len);
                                if (status_sock == -1) {
                                    perror("sendto");
                                }
                            } else {
                                remains = cma_dma.last_number_word - begin_addr;

                                proto_upd.command = 0x0;
                                proto_upd.quantity_byte = sizeof(uint32_t) * remains;
                                status_sock = sendto(udp_sock, (struct protocol_command_udp*)&proto_upd, sizeof(proto_upd), 0, (struct sockaddr*)&client_addr, client_len);
                                if (status_sock == -1) {
                                    perror("sendto");
                                }
                                status_sock = sendto(udp_sock, (uint32_t*)(cma_ddr + begin_addr), proto_upd.quantity_byte, 0, (struct sockaddr*)&client_addr, client_len);
                                if (status_sock == -1) {
                                    perror("sendto");
                                }
                                
                                proto_upd.quantity_byte = sizeof(uint32_t) * current_address_read;
                                status_sock = sendto(udp_sock, (struct protocol_command_udp*)&proto_upd, sizeof(proto_upd), 0, (struct sockaddr*)&client_addr, client_len);
                                if (status_sock == -1) {
                                    perror("sendto");
                                }
                                status_sock = sendto(udp_sock, (uint32_t*)cma_ddr, proto_upd.quantity_byte, 0, (struct sockaddr*)&client_addr, client_len);
                                if (status_sock == -1) {
                                    perror("sendto");
                                }
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }



cleanup:
    printf("Завершение работы сервера...\n");
    close(udp_sock);
    close(epoll_fd);

    DMAmemory_free(fd_cma);
    munmap(cma_ddr, cma_dma.size_ddr);

    close(fd_mem);
    close(fd_cma);
    return EXIT_SUCCESS;
}


void calculating_address(size_t number_byte) {
    number_word = number_byte / 4;
    if (current_address_read >= number_word) {
        begin_addr = current_address_read - number_word;
    } else {
        begin_addr = (cma_dma.last_number_word + current_address_read) - number_word;
    }
}


// обработчик сигнала
void signal_handler(int sig_num) {
    printf("\nПолучен сигнал %d (SIGINT - Ctrl+C)\n", sig_num);
    printf("Программа завершает работу...\n");
    printf("Завершение работы сервера...\n");
    close(udp_sock);
    close(epoll_fd);

    DMAmemory_free(fd_cma);
    munmap(cma_ddr, cma_dma.size_ddr);

    close(fd_mem);
    close(fd_cma);
    exit(EXIT_SUCCESS); // Завершаем программу корректно
}