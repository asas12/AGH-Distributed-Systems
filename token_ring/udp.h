#ifndef TOKEN_RING_UDP_H
#define TOKEN_RING_UDP_H

int udp_set_recport(char* recport);
int udp_set_sendport(char* send_ip, char* sendport);
int udp_send_init(int socket, char* name);
int udp_rec_init(int socket);

#endif //TOKEN_RING_UDP_H
