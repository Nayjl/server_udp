#ifndef NON_BLOCK_SOCKET_H
#define NON_BLOCK_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/epoll.h>


#define BLOCING_EPOLL_WAIT -1
#define UNBLOCING_EPOLL_WAIT 0



#define MAX_EVENTS 10 // Максимальное количество событий, возвращаемых epoll_wait

int set_nonblocking(int fd);

int create_epol();

void control_interes_epoll(struct epoll_event ev, int epoll_fds, int interes_sock, uint32_t interes_event);


#endif

