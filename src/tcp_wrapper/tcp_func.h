
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/socket.h> // Sokets libs
#include <netinet/in.h> // Internet Protocol Family Header 
#include <arpa/inet.h> 

#include <string.h>
#include <errno.h>


int tcp_create_socket(void);

void tcp_setup_ip_addr(struct sockaddr_in* server_addr, int port , char* ip_addr);

void tcp_bind(int sockfd, struct sockaddr_in* server_addr);

void tcp_listen(int sockfd);

int tcp_accept(int sockfd, struct sockaddr_in* client_addr);

void tcp_connect(int sockfd, struct sockaddr_in* server_addr);

void tcp_client_setup_ip_addr(struct sockaddr_in* server_addr, int port , char* ip_addr);