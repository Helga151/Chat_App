#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

struct Message
{
    long mtype;
    char mtext[1000];
    char mfrom[100]; 
    long mid; //id of sender
    char mto[100];
    time_t msec;
}mes;
long server_type = 100;

//methods
void RegisterClient(int *arr_queue, int clients_all, int temp_q) {
    //user has logged in
    int receive = msgrcv(temp_q, &mes, (sizeof(mes) - sizeof(long)), 20, IPC_NOWAIT);
    if(receive > 0) {
        int zm = 0;
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] == 0) { //first empty slot
                arr_queue[i] = msgget(mes.mid, 0644);
                printf("%s %ld %s\n", mes.mfrom, mes.mid, mes.mtext);
                zm = 1;
                break;
            }
        }
        if(zm == 0) { //to many clients
            printf("Nie ma wolnych miejsc\n");
        }
    }
}

void SendMessage(int current_queue) {
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 1, IPC_NOWAIT);
    if(receive > 0) {
        printf("%s\n", mes.mtext);
        struct tm* current_time;
        current_time = localtime(&mes.msec);

        char time[10];
        sprintf(time, "%02d:%02d:%02d", current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
        printf("%s %s\n", time, mes.mtext);

        int chat = open("chat.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
        write(chat, time, 8);
        write(chat, " user: ", 7);
        write(chat, &mes.mfrom, strlen(mes.mfrom));
        write(chat, " message: ", 10);
        write(chat, &mes.mtext, strlen(mes.mtext));
        write(chat, "\n", 1);
        close(chat);
    }
    //send to everyone on this server if mes.mto is 1
    //elsewise to particular client
}

void WriteOldMessages(int current_queue) {
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 2, IPC_NOWAIT);
    if(receive > 0) {
        printf("10 wiadomosci\n");
        char letter;
        char arr[1000];
        int j = 0;
        int chat = open("chat.txt", O_RDONLY | O_CREAT | O_APPEND, S_IRWXU);
        while (read(chat, &letter, 1) > 0)
        {
            arr[j] = letter;
            j++;
        }
        close(chat);
        int nl = 10, k = 0; //write 10 last lines
        char lines[1000];
        while(j-- && nl > 0) {
            if(arr[j] == '\n') {
                nl--;
            }
            lines[k] = arr[j];
            k++;
        }
        memset(arr, 0, 1000);
        int i = 0;
        for(j = k - 1; j >= 0; j--) {
            //printf("%c", lines[j]);
            arr[i] = lines[j];
            i++;
        }
        //printf("\n");
        printf("%s\n", arr);
        strcpy(mes.mtext, arr);
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

int main(int argc, char* argv[]) {
    int temp_q = msgget(12345678, 0644 | IPC_CREAT);
    int clients_all = 5; //max 5 clients from the task instruction
    int arr_queue[clients_all];
    for(int i = 0; i < clients_all; i++) {
        arr_queue[i] = 0;
    }
    //int chat = open("chat.txt", O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
    int receive;
    while(1) {
        RegisterClient(arr_queue, clients_all, temp_q);
        //checking if there is any message
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] != 0) { //look for a not empty queue
                SendMessage(arr_queue[i]);
                WriteOldMessages(arr_queue[i]);
            }
            else {
                break; //no more clients to check
            }
        }
    }
    /*int chat = open("chat.txt", O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
    while (1)
    {
        int queue = msgget(1000, 0666 | IPC_CREAT);
        msgrcv(queue, &mes, sizeof(mes), 10, 0);

        //strcmp: 0 - same, < 0 - first smaller, > 0 -first greater
        if(strcmp(mes.mtext, "end") == 0) break; //"end" ends server from executing

    }
    //delete all temp files after server stops*/
    return 0;
}