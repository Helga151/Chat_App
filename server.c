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
    int arr_queue[clients_all+1],arr_time[clients_all];
    User arr_users[clients_all+1];
    for(int i = 0; i < clients_all+1; i++) {
        arr_queue[i] = 0;
        arr_users[i].uid = 0;
        arr_users[i].ulog = 0;
        for(int j = 1; j <= rooms_all; j++) {
            arr_users[i].urooms[j] = 0;
        }
        arr_time[i%clients_all]=10;
        memset(arr_users[i].uname,0,sizeof(arr_users[i].uname));
    }

    char arr_rooms[rooms_all + 1][100];
    for(int i = 1; i <= rooms_all; i++) {
        memset(arr_rooms[i], 0, 100);
    }   
    while(1) {
        //PrintRoomsList(temp_q);
        RegisterClient(arr_queue, clients_all, temp_q, arr_users, arr_rooms);
        //checking if there is any message
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] != 0) { //look for a not empty queue
                arr_time[i]--;
                SendMessage(arr_queue[i], arr_queue, clients_all, arr_users);
                WriteOldMessages(arr_queue[i]);
                PrintRoomsList(arr_queue[i]);
                PrintUsernames(arr_queue[i], arr_queue, clients_all, arr_users);
                //arr_users[i + 1] - index from 1
                AddUserToRoom(arr_queue[i], arr_rooms, arr_users[i + 1]);
                WriteUsersRooms(arr_queue[i], arr_users[i + 1], arr_rooms);
                LogoutClient(i, arr_queue, clients_all, arr_users);
                SendHeartbeat(arr_time, i, arr_queue, clients_all, arr_users);
            }
        }
        sleep(1);
    }
    return 0;
}