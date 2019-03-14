#ifndef TOKEN_RING_UTIL_H
#define TOKEN_RING_UTIL_H

#define MULTIPORT 8999

#define NONE 0
#define SEND 1
#define RECEIVED 2
#define INIT 3
#define INITREPLY 4


struct token{
    char from[32];
    char to[32];
    // 0 - empty, 1 - send, 2 received, 3 - init
    // in case of init message should contain port number
    int mode;
    char msg[256];
};

void report_token(char* id);

#endif //TOKEN_RING_UTIL_H
