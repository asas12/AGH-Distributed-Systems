#ifndef TOKEN_RING_TCP_H
#define TOKEN_RING_TCP_H

int set_recport(char* recport);
int accept_connection(int socket);
int set_sendport(char* send_ip, char* sendport);
int send_init(int socket);
int rec_init(int socket);


#endif //TOKEN_RING_TCP_H
