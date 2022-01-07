#include "../lib/serv_olga.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int CheckIfUnique(char *text) { //for rooms file
    int rooms_file = open("txt/rooms_file", O_RDWR | O_CREAT | O_EXCL, 0644);
    int enter = 0;
    if(errno == EEXIST) { //file existed - write \n in the beginning
        printf("File existed\n");
        enter = 1;
        rooms_file = open("txt/rooms_file", O_RDWR | O_CREAT, 0644);
        if(rooms_file < 0) { //or error occured
            printf("Could not open the file\n");
            return -1;
        }
    }
    else { //file didn't exist 
        printf("File didn't exist\n");
    }
    int unique = 0; //0 - unique
    char letter;
    char arr[100];
    memset(arr, 0, 100);
    int j = 0;
    while(read(rooms_file, &letter, 1) > 0) {
        if(letter == '\n') {
            if(strcmp(arr, text) == 0) {
                printf("Room '%s' exists\n", text);
                unique = 1;
                break;
            }
            memset(arr, 0, 100);
            j = 0;
        }
        else {
            arr[j] = letter;
            j++;
        }   
    }
    if(strcmp(arr, text) == 0) { //check the last line
        printf("Room '%s' exists\n", text);
        unique = 1;
    }
    if(unique == 0) {
        if(enter == 1) {
            write(rooms_file, "\n", 1);
        }
        write(rooms_file, text, strlen(text));
        close(rooms_file); 
    }
    return unique;
}

void AddUserToFile(User user) {
    int names_file = open("txt/names_file", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(names_file < 0) {
        printf("Could not open the file\n");
        return;
    }
    char pid[20];
    sprintf(pid, "%ld", user.uid);
    write(names_file, user.uname, strlen(user.uname));
    write(names_file, "_", 1);
    write(names_file, pid, strlen(pid));
    write(names_file, "\n", 1);
    close(names_file);  
}

int AddRoomToArray(int tmp, User *current_user, char *text, char (*arr_rooms)[100]) {
    int j = 0, free_space = 0;
    if(tmp == 0) { //unique room name
        for(j = 1; j <= rooms_all; j++) { //find first empty slot for new room
            if(strlen(arr_rooms[j]) == 0) {
                strcpy(arr_rooms[j], text);
                current_user->urooms[j] = 1;
                printf("%d. %s\n", j, arr_rooms[j]);
                free_space = 1;
                break;
            }
        }
    }
    else if(tmp == 1) { //not unique and not error, assign 1 to user.uroom where room name is
        for(j = 1; j <= rooms_all; j++) { 
            if(strcmp(arr_rooms[j], text) == 0) {
                current_user->urooms[j] = 1;
                printf("%d. %s\n", j, arr_rooms[j]);
                free_space = 1;
                break;
            }
        }
    }
    if(free_space == 0) {
        printf("No empty room slots left\n");
        j = 0;
    }
    return j;
}

char *WriteCurrentTime(time_t sec) {
    struct tm* current_time;
    current_time = localtime(&sec);

    char *time = malloc(11);
    sprintf(time, "%02d:%02d:%02d", current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
    return time;
}

void RegisterClient(int *arr_queue, int clients_all, int temp_q, User *arr_users, char (*arr_rooms)[100]) {
    //user has logged in
    Message mes;
    int receive = msgrcv(temp_q, &mes, (sizeof(mes) - sizeof(long)), 20, IPC_NOWAIT);
    if(receive > 0) {
        int zm = 0;
        int j;
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] == 0) { //first empty slot
                arr_queue[i] = msgget(mes.mid, 0644 | IPC_CREAT);
                strcpy(arr_users[i].uname, mes.mfrom);
                arr_users[i].uid = mes.mid;
                arr_users[i].ulog = 1;
                printf("%s %ld %s %d\n", arr_users[i].uname, arr_users[i].uid, mes.mtext, arr_users[i].ulog);
                zm = 1;
                //check if room name is unique - yes(create new room)
                int tmp = CheckIfUnique(mes.mtext);
                j = AddRoomToArray(tmp, &arr_users[i], mes.mtext, arr_rooms);
                AddUserToFile(arr_users[i]);
                break;
            }
        }
        if(zm == 0) { //to many clients
            printf("No empty slots for clients left\n");
            strcpy(mes.mtext, "failed");
        }
        mes.mtype = server_type;
        mes.mid = (long)j; //j is an index where is the room written by user
        msgsnd(temp_q, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void SendPrivateMessage(int current_queue, int* arr_queue, int clients_all, User *arr_users, User *current_user) {
//private - send the message to one, particular user
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 1, IPC_NOWAIT);
    if(receive > 0) {
        //find this user 
        int exists = 0;
        int same_room = 0;
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] != 0 && arr_users[i].ulog == 1 && strcmp(arr_users[i].uname, mes.mto) == 0) {
                exists = 1;
                for(int j = 1; j <= rooms_all; j++) { //recipent and sender are in the same room, don't care which one 
                    if(arr_users[i].urooms[j] == 1 && current_user->urooms[j] == 1) {
                        mes.mtype = msg_from_server;
                        msgsnd(arr_queue[i], &mes, (sizeof(mes) - sizeof(long)), 0);
                        printf("sent\n");
                        same_room = 1;
                        break;
                    }
                }
                if(exists == 1) break;
            }
        }
        if(exists == 0) {
            //send feedback to client that written recipent does not exist
            strcpy(mes.mtext, "Written recipent does not exist\n");
        }
        else if(same_room == 0 && exists == 1) {
            strcpy(mes.mtext, "No common rooms with you and the recipent\n");
        }
        else strcpy(mes.mtext, "Message sent\n");
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void SendPublicMessage(int current_queue, int* arr_queue, int clients_all, User *arr_users, User *current_user) {
//public - send the message to every client on entered room
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 2, IPC_NOWAIT);
    if(receive > 0) {
        mes.mtype = msg_from_server;
        char text[100];
        //check if sender belongs to the room which he/she entered
        if(current_user->urooms[(int)mes.mid] == 0) {
            sprintf(text, "You can not sent public message to room where you do not belong\n");
        } 
        else {
            int count = 0;
            for(int i = 0; i < clients_all; i++) {
                if(arr_queue[i] != 0 && arr_users[i].urooms[(int)mes.mid] == 1 && strcmp(arr_users[i].uname, mes.mfrom) != 0) {
                    msgsnd(arr_queue[i], &mes, (sizeof(mes) - sizeof(long)), 0);
                    printf("sent\n");
                    count++;
                }
            }
            sprintf(text, "Message sent to %d users\n", count);
            char time[10];
            strcpy(time, WriteCurrentTime(mes.msec));
            printf("%s %s\n", time, mes.mtext);
            char file[10];
            sprintf(file, "txt/%ld", mes.mid);
            //write the message, time and sender to the chat file
            int chat = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            write(chat, time, 8);
            write(chat, " user: ", 7);
            write(chat, &mes.mfrom, strlen(mes.mfrom));
            write(chat, " message: ", 10);
            write(chat, &mes.mtext, strlen(mes.mtext));
            write(chat, "\n", 1);
            close(chat);
        }
        printf("%s", text);
        strcpy(mes.mtext, text);
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void WriteOldMessages(int current_queue, User *current_user) {
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 3, IPC_NOWAIT);
    if(receive > 0) {
        //check if sender belongs to the room which he/she entered
        char arr[1000];
        if(current_user->urooms[(int)mes.mid] == 0) {
            sprintf(arr, "You can not read messages from the room where you do not belong\n");
        }
        else {
            char letter;
            int j = 0;
            char file[10];
            sprintf(file, "txt/%ld", mes.mid);
            int chat = open(file, O_RDONLY | O_CREAT, 0644);
            if(chat < 0) { //or error occured
                printf("Could not open the file\n");
                return;
            }
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
                arr[i] = lines[j];
                i++;
            }
            if(nl == 10) { //no messages was sent to this room
                strcpy(arr, "No messages\n");
            }
        } 
        printf("%s\n", arr);
        strcpy(mes.mtext, arr);
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void PrintUsernames(int current_queue, int *arr_queue, int clients_all, User *arr_users) {
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 4, IPC_NOWAIT);
    if(receive > 0) {
        char arr[1000], tmp[1000];
        memset(arr, 0, 1000);
        memset(tmp, 0, 1000);
        for(int i = 0; i < clients_all; i++) {
            if(strlen(arr_users[i].uname) > 0) { 
                sprintf(tmp, "%d. %s\n", i + 1, arr_users[i].uname);
                strcat(arr, tmp);
            }
        }
        printf("%s\n", arr);  
        mes.mtype = server_type;
        strcpy(mes.mtext, arr);
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void PrintRoomsList(int current_queue) {
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 5, IPC_NOWAIT);
    if(receive > 0) {
        int rooms_file = open("txt/rooms_file", O_RDONLY, 0644);
        if(rooms_file < 0) {
            printf("Could not open the file\n");
            strcpy(mes.mtext, "failed");
        }
        else {
            int i = 1, j = 0;
            char arr[1000], letter, tmp[1000];
            memset(arr, 0, 1000);
            memset(tmp, 0, 1000);
            sprintf(arr, "%d. ", i);

            while(read(rooms_file, &letter, 1) > 0) {
                sprintf(tmp, "%c", letter);
                strcat(arr, tmp);
                memset(tmp, 0, 1000);

                if(letter == '\n') {
                    i++;
                    sprintf(tmp, "%d. ", i);
                    strcat(arr, tmp);
                    memset(tmp, 0, 1000);
                }
            }
            printf("%s\n", arr);        
            strcpy(mes.mtext, arr);
        }
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
        close(rooms_file);
    }
}

void WriteUsersRooms(int current_queue, User *current_user, char (*arr_rooms)[100]) {
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 6, IPC_NOWAIT);
    if(receive > 0) {
        char text[1000], tmp[100];
        memset(tmp, 0, 100);
        memset(text, 0, 1000);
        int count = 0;
        for(int i = 1; i <= rooms_all; i++) {
            //printf("%d. %d %s\n", i, current_user->urooms[i], arr_rooms[i]);
            if(current_user->urooms[i] == 1) { //user belongs to this room
                //printf("%d. %s\n", i, arr_rooms[i]); //finds room's name
                sprintf(tmp, "%i. %s\n", i, arr_rooms[i]);
                strcat(text, tmp);
                memset(tmp, 0, 100);
                count++;
            }
        }
        printf("%d rooms:\n%s\n", count, text);
        strcpy(mes.mtext, text);
        mes.mid = (long)count;
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void WriteAllUsersRooms(int current_queue, int *arr_queue, int clients_all, User *arr_users, char (*arr_rooms)[100]) {
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 7, IPC_NOWAIT);
    if(receive > 0) {
        char text[1000], tmp[200];
        memset(tmp, 0, 200);
        memset(text, 0, 1000);
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] != 0) {
                sprintf(tmp, "user %s room(s): ", arr_users[i].uname);
                strcat(text, tmp);
                memset(tmp, 0, 100);
                for(int j = 1; j <= rooms_all; j++) {
                    if(arr_users[i].urooms[j] == 1) {
                        sprintf(tmp, "%s ", arr_rooms[j]);
                        strcat(text, tmp);
                        memset(tmp, 0, 200);
                    }
                }
                strcat(text, "\n");
            }
        }
        printf("%s", text);
        strcpy(mes.mtext, text);
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void AddUserToRoom(int current_queue, char (*arr_rooms)[100], User *current_user) {
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 8, IPC_NOWAIT);
    if(receive > 0) {
        int tmp = CheckIfUnique(mes.mtext);
        int j = AddRoomToArray(tmp, current_user, mes.mtext, arr_rooms);
        printf("%d\n", j);
        mes.mtype = server_type;
        mes.mid = (long)j; //j is an index where is the room written by user
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}

void RemoveUserFromRoom(int current_queue, User *current_user) {
    Message mes;
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 9, IPC_NOWAIT);
    if(receive > 0) {
        if(current_user->urooms[(int)mes.mid] == 1) {
            current_user->urooms[(int)mes.mid] = 0;
            strcpy(mes.mtext, "Successfully removed\n");
        }
        else {
            strcpy(mes.mtext, "User does not belong to this room\n");
        }
        mes.mtype = server_type;
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}