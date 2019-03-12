#ifndef TOKEN_RING_TCP_H
#define TOKEN_RING_TCP_H

int tcp_set_recport(char* recport);
int tcp_accept_connection(int socket);
int tcp_set_sendport(char* send_ip, char* sendport);
int tcp_send_init(int socket);
int tcp_rec_init(int socket);


#endif //TOKEN_RING_TCP_H
