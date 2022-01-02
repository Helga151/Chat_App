#ifndef STRUCT
#define STRUCT

#include <time.h>
#include <stdio.h>
#define server_type 100

typedef struct Message Message;
struct Message {
    long mtype;
    char mtext[1000];
    char mfrom[100]; //who sends the message
    long mid; //id of sender
    char mto[100]; //who should receive the message
    time_t msec; //when the message was sent, in seconds
};

typedef struct User User;
struct User {
    long uid; //unique id - pid
    char uname[100];
    int ulog; //is user logged in: 0 - no, 1 - yes
    char uroom[100]; //room to which user belongs
};

#endif