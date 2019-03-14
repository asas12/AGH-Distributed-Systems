#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "tcp.h"
#include "udp.h"
#include "util.h"

int main(int argc, char * argv[]) {
    if(argc!=8){
        printf("Too few arguments!");
        return 0;
    }

    char* my_name = argv[1];
    printf("My name: %s\n", my_name);

    char* recport = argv[2];
    char* send_ip = argv[3];
    char* sendport = argv[4];

    char* neighbour = argv[7];

    int send_port_number = atoi(sendport);
    printf("I listen on port number: %s\n", recport);

    printf("I send to %s\n", argv[4]);
    printf("I have token: %s\n", argv[5]);
    printf("Mode: %s\n", argv[6]);

    int listen_socket, send_socket;
    int *rec_socket_pointer;

    if(strcmp(argv[6], "tcp") == 0) {
        // TCP setup

        //rec_init_socket = tcp_accept_connection(listen_socket);

        //int new_sendport = tcp_rec_init(rec_init_socket);

        //int real_send_socket = tcp_set_sendport(send_ip, new_sendport);


        //binds, listens and returns socket descriptor
        listen_socket = tcp_set_recport(recport);

        //thread that accepts all requests on listen_socket and returns new socket descriptors
        pthread_t accepter;
        pthread_create(&accepter, NULL, tcp_accept_connection, (void *) &listen_socket);


        // tries to connect to sendport, blocking - because of accepting first, someone has to accept
        send_socket = tcp_set_sendport(send_ip, send_port_number);

        // blocking - waiting for receiving connection to be established
        pthread_join(accepter, (void **) &rec_socket_pointer);

    }else{
        // UDP setup
        rec_socket_pointer = malloc(sizeof(rec_socket_pointer));
        *rec_socket_pointer = udp_set_recport(recport);
        send_socket = udp_set_sendport(send_ip, sendport);

    }

        // send first token
        if(strcmp(argv[5], "y")==0) {
            // for udp, giving it a chance to get up in time...
            sleep(5);
            printf("Sending first token.\n");
            struct token token;
            token.mode = NONE;
            sprintf(token.from,"%s",  my_name);
            sprintf(token.msg, "First from %s", my_name);

            ssize_t res = send(send_socket, &token, sizeof(token), 0);
            if(res<1){
                printf("Error sending token.\n");
            }
        }else{
            if(strcmp(argv[6], "udp") == 0){
                // not the first host
                struct token init;
                init.mode = INIT;
                strcpy(init.msg, recport);
                strcpy(init.from, my_name);

                // send where to send data
                send(send_socket, &init, sizeof init, 0);
                

            }
        }

        //tcp_loop(recport, 1, my_name, send_ip, send_port_number);
        struct token token;
        int k = 0;
        ssize_t res;
        // main loop
        while (k < 1000) {
            // blocking
            res =  recv(*rec_socket_pointer, &token, sizeof(token), 0);
            sleep(1);
            printf("RECEIVED DATA. MODE: %d, FROM: %s, TO: %s, %s\n", token.mode, token.from, token. to, token.msg);
            report_token(my_name);

            if(token.mode == NONE){
                // lets assume always send
                printf("Sent message to: %s\n", neighbour);
                token.mode = SEND;
                sprintf(token.from, "%s", my_name);
                sprintf(token.to, "%s", neighbour);
                sprintf(token.msg, "Hello from %s", my_name);
                send(send_socket, &token, sizeof token, 0);
            }else{
                if(token.mode == SEND){
                    if(strcmp(token.to, my_name)==0){
                        // message to me
                        printf("%s says: \"%s\"\nSending conformation.\n", token.from, token.msg);
                        token.mode = RECEIVED;
                        strcpy(token.to, token.from);
                        strcpy(token.from, my_name);

                        send(send_socket, &token, sizeof token, 0);
                    }else{
                        // not to me
                        if(strcmp(token.from, my_name)==0){
                            //circulating frame
                            token.mode = NONE;
                        }
                    }

                    send(send_socket, &token, sizeof token, 0);
                }else{
                    if(token.mode == RECEIVED){
                        if(strcmp(token.to, my_name)==0){
                            printf("Received conformation from %s\n", token.from);
                        }
                        strcpy(token.from, my_name);
                        token.mode = NONE;

                    }else{
                        if(strcmp(argv[5], "tcp")==0) {
                            printf("Weird mode: %d\n", token.mode);
                        }else{
                            //UDP - adding new clients



                        }
                    }

                    send(send_socket, &token, sizeof(token), 0);
                }
            }




            /*
            if(strcmp(token.to, my_name)==0) {
                printf("received %ld bytes from %s: %s\n", res, token.from, token.msg);
                token.mode = RECEIVED;
                sprintf(token.to, "%s", neighbour);
                sprintf(token.from, "%s", my_name);

            }else{
                if(token.mode == NONE) {
                    token.mode = SEND;
                    sprintf(token.from, "%s", my_name);
                    sprintf(token.to, "%s", neighbour);
                    sprintf(token.msg, "Hello from %s", my_name);

                    res = send(send_socket, &token, sizeof(token), 0);
                }
                else{
                        // send through
                        send(send_socket, &token, sizeof(token), 0);
                    }
            }
                report_token(my_name);
            */
                k++;


        }

        //close(real_send_socket);
    //}
    /*else{
        if(strcmp(argv[6], "udp") == 0) {
            int rec_socket;
            rec_socket = udp_set_recport(recport);
            int send_socket = udp_set_sendport(send_ip, sendport);

            int k = 0;
            while (k < 1000) {
            int i = udp_send_init(send_socket, my_name);
            printf("Sent: %d\n", i);

            i = udp_rec_init(rec_socket);
            if (i > 0) {
                report_token(my_name);
                printf("Received: %d\n", i);
            }

            k++;
            sleep(1);
        }


            close(rec_socket);
            close(send_socket);

        }else{
            printf("Unknown mode.\n");
            return 1;
        }
    }*/
    free(rec_socket_pointer);

    return 0;
}