#include "lib/serv_olga.h"
#include "lib/serv_pawel.h"
#include "lib/struct.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

void CloseProgram() {
    unlink("txt/rooms_file");
    unlink("txt/chat");
    unlink("txt/names_file");
    printf("\nBye\n");
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, CloseProgram);
    int temp_q = msgget(12345678, 0644 | IPC_CREAT);
    int clients_all = 5; //max 5 clients from the task instruction
    int arr_queue[clients_all];
    User arr_users[clients_all];
    for(int i = 0; i < clients_all; i++) {
        arr_queue[i] = 0;
        arr_users[i].uid = 0;
        arr_users[i].ulog = 0;
        arr_users[i].uname[0]='\0';
    }

    char arr_rooms[rooms_all + 1][100];
    for(int i = 1; i <= rooms_all; i++) {
        memset(arr_rooms[i], 0, 100);
    }   
    while(1) {
        RegisterClient(arr_queue, clients_all, temp_q, arr_users, arr_rooms);
        //checking if there is any message
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] != 0) { //look for a not empty queue
                SendMessage(arr_queue[i], arr_queue, clients_all, arr_users);
                WriteOldMessages(arr_queue[i]);
                PrintRoomsList(arr_queue[i]);
                LogoutClient(arr_queue[i], i, arr_queue, clients_all, arr_users);
                PrintUsernames(arr_queue[i], arr_queue, clients_all, arr_users);
            }
        }
        sleep(1);
    }
    return 0;
}