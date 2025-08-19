#ifndef UDP_FUNC_H
#define UDP_FUNC_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h> // Sokets libs
#include <netinet/in.h> // Internet Protocol Family Header 
#include <arpa/inet.h> 

#include <string.h>
#include <errno.h>



int udp_create_socket(void);

// Настройка адреса 
void udp_setup_ip_addr(struct sockaddr_in *addr, uint16_t port, char* ip_addr);


// Настройка адреса сервера
void udp_setup_ip_addr_serv(struct sockaddr_in *addr, uint16_t port);


// Привязка сокета к адресу (bind)
void udp_bind(int sockfd, struct sockaddr_in *addr);

// void udp_connect(int sockfd, struct sockaddr_in* addr);

void udp_setsockopt(int sockfd, int opt);

#endif