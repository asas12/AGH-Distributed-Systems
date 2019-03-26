#ifndef TOKEN_RING_TCP_H
#define TOKEN_RING_TCP_H

#include <stdio.h>

int tcp_set_recport(char* recport);
void* tcp_accept_connection(void* socket_ptr);
int tcp_set_sendport(char* send_ip, int sendport);

#endif //TOKEN_RING_TCP_H
