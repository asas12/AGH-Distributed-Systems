#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "udp.h"
#include "util.h"

#include <errno.h>

int udp_set_recport(char* recport){
    printf("Setting recport: %s\n", recport);
    struct sockaddr_in rec_addr;
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_port = htons(atoi(recport));
    rec_addr.sin_addr.s_addr = INADDR_ANY;
    memset(rec_addr.sin_zero, '\0', sizeof rec_addr);

    //int rec_socket = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    int rec_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(rec_socket==-1){printf("Error creating socket\n");};
    if(bind(rec_socket,(struct sockaddr*) &rec_addr, sizeof rec_addr)!=0){
        printf("Error binding receiving port.\n");
        return -1;
    }

    return rec_socket;
}

int udp_set_sendport(char* send_ip, char* sendport){

    printf("Setting sendport: %s\n", sendport);

    struct sockaddr_in send_addr;
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(atoi(sendport));
    send_addr.sin_addr.s_addr = inet_addr(send_ip);
    memset(send_addr.sin_zero, '\0', sizeof send_addr);

    int send_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if(connect(send_socket, (struct sockaddr*) &send_addr, sizeof send_addr)!=0){
        printf("Error connecting.\n");
    }

    return send_socket;
}

int udp_send_init(int socket, char* from){
    char* msg = "Hello!\n";
    //struct token token = {"Ala", 1, "hello!"};
    //return send(socket, &token, sizeof(token), 0);


    struct token init = {
            .mode = INIT,
    };
    strcpy(init.from, from);
    sprintf(init.msg, "Hello!");

    return send(socket, &init, sizeof(init), 0);



    return send(socket, msg, sizeof msg, 0);
}

int udp_rec_init(int socket){
   // char msg[100];
    //ssize_t  rec = recv(socket, msg, sizeof msg, 0);
    //printf("%s\n", msg);


    struct token init;

    int rec = recv(socket, &init, sizeof(init), 0);
    printf("%s said: %s\n", init.from, init.msg);
    return rec;


    //struct token token;
    //ssize_t rec = recv(socket, &token, sizeof token, 0);
    //printf("%s from %s\n",token.msg, token.id);
    return rec;
}