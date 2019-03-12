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

    int send_port_number = atoi(sendport);
    printf("I listen on port number: %d\n", send_port_number);

    printf("I send to %s\n", argv[4]);
    printf("I have token: %s\n", argv[5]);
    printf("Mode: %s\n", argv[6]);

    int send_socket;
    int rec_socket;

    if(strcmp(argv[6], "tcp") == 0){
        int listen_socket = tcp_set_recport(recport);
        send_socket = tcp_set_sendport(send_ip, sendport);
        rec_socket = tcp_accept_connection(listen_socket);
        tcp_send_init(send_socket);
        tcp_rec_init(rec_socket);
        close(listen_socket);
    }
    else{
        if(strcmp(argv[6], "udp") == 0){
            rec_socket = udp_set_recport(recport);
            send_socket = udp_set_sendport(send_ip, sendport);


            int i = udp_send_init(send_socket);
            printf("Sent: %d\n", i);

            i = udp_rec_init(rec_socket);
            printf("Received: %d\n", i);
        }else{
            printf("Unknown mode.\n");
            return 1;
        }
    }


    close(send_socket);
    close(rec_socket);
    return 0;
}