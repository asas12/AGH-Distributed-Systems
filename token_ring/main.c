#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tcp.h"
#include "udp.h"

int main(int argc, char * argv[]) {
    if(argc!=7){
        printf("Too few arguments!");
        return 0;
    }
    printf("My name: %s\n", argv[1]);
    char* recport = argv[2];
    char* send_ip = argv[3];
    char* sendport = argv[4];

    int port_number = atoi(sendport);
    printf("I listen on port number: %d\n", port_number);

    printf("I send to %s\n", argv[4]);
    printf("I have token: %s\n", argv[5]);
    printf("Mode: %s\n", argv[6]);

    if(strcmp(argv[6], "tcp") == 0){
        int listen_socket = set_recport(recport);
        int send_socket = set_sendport(send_ip, sendport);
        int rec_socket = accept_connection(listen_socket);
        send_init(send_socket);
        rec_init(rec_socket);

    }
    else{
        if(strcmp(argv[6], "udp") == 0){
            hello_udp();

        }else{
            printf("Unknown mode.\n");
            return 1;
        }
    }
    
    return 0;
}