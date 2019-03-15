#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include "util.h"


void report_token(char* id){

    int sockfd;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MULTIPORT);
    addr.sin_addr.s_addr = inet_addr("224.1.1.1");
    memset(addr.sin_zero, '\0', sizeof addr.sin_zero);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    int broadcast = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    ssize_t res = sendto(sockfd, id, strlen(id), 0, (struct sockaddr*) &addr, sizeof addr );
    //printf("Sent info about token: %ld.\n", res);


}