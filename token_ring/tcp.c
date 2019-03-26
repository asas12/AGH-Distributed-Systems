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

int tcp_send_init(int socket, char* from, char* recport){

    struct token init = {
            .mode = INIT,
            };
    strcpy(init.msg, recport);
    strcpy(init.from, from);
    printf("Sent info that I, %s listen on %s\n", from, recport);
    return send(socket, &init, sizeof(init), 0);
}

int tcp_rec_init(int socket){

    struct token init;

    int rec = recv(socket, &init, sizeof(init), 0);
    printf("%s said to send to port: %s\n", init.from, init.msg);
    return atoi(init.msg);
}

int tcp_send_msg(int socket, char* msg, char* from, char* to){
    struct token token;
    token.mode = SEND;
    strcpy(token.from, from);
    strcpy(token.to, to);
    strcpy(token.msg, msg);

    return send(socket, &token, sizeof(token), 0);
}

void tcp_loop(char* recport, int has_token, char* my_name, char* send_ip, int sendport) {

    int send_socket, rec_socket;
    struct token received;
    strcpy(received.from,my_name);
    received.mode = NONE;
    strcpy(received.msg, "Hello!");

    int listen_socket = tcp_set_recport(recport);

    pthread_t accepter;
    pthread_create(&accepter, NULL, tcp_accept_connection, (void*)&listen_socket);

    send_socket = tcp_set_sendport(send_ip, sendport);

    int* rec_sock; //= malloc(sizeof(rec_sock));


    int s = tcp_send_init(send_socket, my_name, recport);

    //blocking - waiting for response from inviter
    pthread_join(accepter, (void**) &rec_sock);
    printf("Thread gave: %d\n", *rec_sock);
    rec_socket = *rec_sock;

    printf("init sent with %d bytes\n", s);
    // check if talking to myself
    int rec = recv(rec_socket, &received, sizeof(received), 0);
    while(rec<=0){
        rec = recv(rec_socket, &received, sizeof(received), 0);
    }
    //int first = 0;
    if(strcmp(received.from, my_name)==0){
        printf("I must be first to run.\n");
      //  first = 1;
    }
    else{
        if(received.mode!=INITREPLY){
            printf("Wrong reply! Mode: %d, from %s\n", received.mode, received.from);
            return;
        }
        close(send_socket);

        send_socket = tcp_set_sendport(send_ip, atoi(received.msg));
    }


    pthread_create(&accepter, NULL, tcp_accept_connection, (void*)&listen_socket);

    int i = 0;
    while(i<30){
        int r = pthread_tryjoin_np(accepter, (void**) &rec_sock);
        if(r==0){
            printf("Someone is trying to get in\n");
            printf("Thread gave: %d\n", *rec_sock);
            int rec_tmp_socket = *rec_sock;

            rec = recv(rec_tmp_socket, &received, sizeof(received), 0);
            while(rec<=0){
                rec = recv(rec_tmp_socket, &received, sizeof(received), 0);
            }
            printf("RECEIVED: from %s, mode: %d message: %s\n", received.from,received.mode, received.msg);
                report_token(my_name);
                struct token cpy;
            while(received.mode!=INIT){

                if(strcmp(received.to,my_name)==0){
                    // message to me
                    printf("\"%s\"\nfrom: %s",received.msg,received.from);

                }
                if(strcmp(received.from,my_name)==0){
                    // circulating message
                    printf("Circulating... %s, %s\n", received.msg, received.from);
                }
                cpy = received;
                rec = -1;
                while(rec<=0){
                    rec = recv(rec_tmp_socket, &received, sizeof(received), 0);
                }

            }
        //        if(first!=1){
                printf("Sending reply to %s\n", received.from);
                int tmp = atoi(received.msg);
                send_socket = tcp_set_sendport(send_ip, tmp);
            sprintf(received.msg, "%d", sendport);

            sendport = tmp;

            received.mode = INITREPLY;
                strcpy(received.to, received.from);
                strcpy(received.from, my_name);


                send(send_socket, &received, sizeof(received), 0);

            //cpy.mode = NONE;
            //sprintf(cpy.msg, "%d lap, %s", i, my_name);
            //send(send_socket, &cpy, sizeof(cpy), 0);

            pthread_create(&accepter, NULL, tcp_accept_connection, (void*)&listen_socket);

            close(rec_tmp_socket);
        }


        rec = recv(rec_socket, &received, sizeof(received), 0);



        if(rec>0){
            printf("RECEIVED: from %s, mode: %d message: %s\n", received.from,received.mode, received.msg);

            report_token(my_name);
            printf("Received msg: %s from %s\n", received.msg, received.from);
            if(received.mode == INIT){
             //   if(first != 1){
                send_socket = tcp_set_sendport(send_ip, atoi(received.msg));
                received.mode = INITREPLY;
                strcpy(received.from, my_name);
                sprintf(received.msg, "%d", sendport);
                send(send_socket, &received, sizeof(received), 0);
                //}
                //else{
               //     first = 0;
                //}
            }
            else{
                   // send_init_reply();
                    if(strcmp(received.to,my_name)==0){
                        // message to me
                        printf("\"%s\"\nfrom: %s",received.msg,received.from);
                        received.mode = NONE;
                        sprintf(received.msg, "%d lap, %s", i, my_name);
                        send(send_socket, &received, sizeof(received), 0);
                    }
                    if(strcmp(received.from,my_name)==0){
                        // circulating message
                        received.mode = NONE;
                        sprintf(received.msg, "%d lap, %s", i, my_name);
                        send(send_socket, &received, sizeof(received), 0);
                    }
                }
            }

        if(has_token>0){
            sleep(3);
            has_token = 0;
            received.mode = SEND;
            strcpy(received.to, "B");
            sprintf(received.msg,"%s from %s ", "Hello, my message!", my_name);
            send(send_socket, &received, sizeof(received), 0);
        }


        sleep(1);
        i++;
        printf("loop: %d\n",i);
    }
}