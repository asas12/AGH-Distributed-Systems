#define _GNU_SOURCE
#include "tcp.h"
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "util.h"

int tcp_set_recport(char* recport){

    struct sockaddr_in rec_addr;
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_port = htons(atoi(recport));
    rec_addr.sin_addr.s_addr = INADDR_ANY;
    memset(rec_addr.sin_zero, '\0', sizeof rec_addr);

    int rec_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK , 0);
    if(rec_socket==-1){printf("Error creating socket\n");};
    if(bind(rec_socket,(struct sockaddr*) &rec_addr, sizeof rec_addr)!=0){
        printf("Error binding receiving port.\n");
        return -1;
    }

    if(listen(rec_socket, 10)!=0){
        printf("Error setting socket to listening state.\n");
        return -1;
    }

    return rec_socket;
}

void* tcp_accept_connection(void* socket_ptr){

    int* res_ptr = malloc(sizeof *res_ptr);
    int socket = *(int *) socket_ptr;

    struct sockaddr_in rec_addr;
    socklen_t size = sizeof rec_addr;
    int rec_socket = -1;
    while(rec_socket<0){
        //rec_socket = accept4(socket, (struct sockaddr*) &rec_addr, &size, SOCK_NONBLOCK);
        rec_socket = accept4(socket, (struct sockaddr*) &rec_addr, &size, 0);
    }
    printf("Accepted connection on recport.\n");
    *res_ptr =  rec_socket;
    pthread_exit(res_ptr);
}

int tcp_set_sendport(char* send_ip, int sendport){

    printf("Setting sendport: %d\n", sendport);

    struct sockaddr_in send_addr;
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(sendport);
    send_addr.sin_addr.s_addr = inet_addr(send_ip);
    memset(send_addr.sin_zero, '\0', sizeof send_addr);

    int send_socket = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 0;
    while(connect(send_socket, (struct sockaddr*) &send_addr, sizeof send_addr)!=0){
        if(flag==0){
            printf("Error connecting. Retrying...\n");
            flag = 1;
        }
    }
    printf("Connected to %s on port %d\n", send_ip, sendport);

    return send_socket;
}