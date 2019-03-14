#ifndef TOKEN_RING_TCP_H
#define TOKEN_RING_TCP_H

#include <stdio.h>

int tcp_set_recport(char* recport);
void* tcp_accept_connection(void* socket_ptr);
int tcp_set_sendport(char* send_ip, int sendport);
int tcp_send_init(int socket, char* from, char* recport);
int tcp_rec_init(int socket);
void tcp_loop(char* recport, int has_token, char* my_name, char* send_ip, int sendport);
int tcp_send_msg(int socket, char* msg, char* from, char* to);


#endif //TOKEN_RING_TCP_H
