#include "lib/serv_olga.h"
#include "lib/serv_pawel.h"
#include "lib/struct.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int temp_q = msgget(12345678, 0644 | IPC_CREAT);
    int clients_all = 5; //max 5 clients from the task instruction
    int arr_queue[clients_all+1];
    User arr_users[clients_all+1];
    for(int i = 0; i < clients_all+1; i++) {
        arr_queue[i] = 0;
        arr_users[i].uid = 0;
        arr_users[i].ulog = 0;
        memset(arr_users[i].uname,0,sizeof(arr_users[i].uname));
    }
    int receive;
    //ShowQueue(clients_all,arr_users);
    while(1) {
        RegisterClient(arr_queue, clients_all, temp_q, arr_users);
        //checking if there is any message
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] != 0) { //look for a not empty queue
                SendMessage(arr_queue[i], arr_queue, clients_all, arr_users);
                WriteOldMessages(arr_queue[i]);
                LogoutClient(arr_queue[i], i, arr_queue, clients_all, arr_users);
            }
        }
        sleep(1);
    }
    //delete all temp files after server stops*/
    return 0;
}