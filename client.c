#include "lib/struct.h"
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
#include <ctype.h>

Message mes;
User user;
int childpid;

void PrintPressKey() {
    printf("Press ENTER to continue\n");
    getchar(); //newline was left in the input stream, get it
    char enter = 0;
    while (enter != '\r' && enter != '\n') { 
        enter = getchar(); 
    }
}

int ReadNumber() {
    int choice;      
    while (scanf("%d", &choice) != 1) {
        scanf("%*s");
    }
    return choice;
}

void ReadUsername() { //check if written name is unique
    int unique = 1; //0 - unique
    int names_file = open("txt/names_file", O_RDONLY | O_CREAT, 0644);
    if(names_file < 0) {
        printf("Could not open the file\n");
        return;
    }
    char name[100];
    while(unique == 1) { //read client name untill it is unique
        unique = 0;
        printf("Enter your username (without spaces): ");
        scanf("%s", name);
        char letter, arr[100];
        int i = 0;
        while(read(names_file, &letter, 1) > 0) {
            if(letter == '_') {
                if(strcmp(arr, name) == 0) {
                    printf("Client with this username '%s' exists\n", name);
                    unique = 1;
                    break;
                }
            }
            else if(letter == '\n') {
                memset(arr, 0, 100);
                i = 0;
            }
            else {
                arr[i++] = letter;
            }
        }
    }
    //if unique - add name to the text file
    strcpy(user.uname, name);
    user.uid = getpid();
    close(names_file); 
}

void ReceiveMessage(int queue) {
    while(msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), msg_from_server, IPC_NOWAIT) > 0) {
        if(mes.mid != 0){ //public message
            if(strcmp(mes.mfrom, user.uname) != 0) {
                printf("New public message from %s: %s \n", mes.mfrom, mes.mtext);
            }
        }
        else { //private message
            printf("New private message from %s: %s \n", mes.mfrom, mes.mtext);
        }
    }
}

void ListingRequest(int queue) {//list all depending on mtype 4 - clients; 5- rooms
    msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    if(msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0) > 0) {
        printf("%s\n", mes.mtext); 
    }
    else printf("An error has occurred!\n");
}

void ListYourRooms(int queue) {
    mes.mtype = 6;
    msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    if(msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0) > 0) {
        printf("Your rooms:\n");
        printf("%s\n", mes.mtext);
    }
    else printf("An error has occurred!\n");
}

int Register() {
    int temp_q = msgget(12345678, 0644 | IPC_CREAT); //create temporary queue to send id to the server, typ logowania - 20
    ReadUsername();
    printf("Enter room name where you want to belong\n-> ");
    scanf("%s", mes.mtext);
    
    mes.mtype = 20;
    strcpy(mes.mfrom, user.uname);
    mes.mid = user.uid;
    msgsnd(temp_q, &mes, (sizeof(mes) - sizeof(long)), 0);
    if(msgrcv(temp_q, &mes, (sizeof(mes) - sizeof(long)), server_type, 0) > 0) {
        if(strcmp(mes.mtext, "failed") == 0) {
            printf("No empty slots left!\n");
            printf("App is closing\n");
            PrintPressKey();
            return 0;
        }
        printf("Your login: '%s' and id: '%ld'\n", user.uname, user.uid);
        user.ulog = 1;
        user.urooms[(int)mes.mid] = 1;
        int queue = msgget(user.uid, 0644 | IPC_CREAT); //create queue for this client
        return queue;
    }
    else {
        printf("An error has occurred!\n");
        return 0;
    }
    //printf("%d\n", queue);
}

void HeartBeat(int queue){
    int t=3;
    while (t){
        int received=0;
        while (msgrcv(queue,&mes,(sizeof(mes) - sizeof(long)),11,IPC_NOWAIT)>0){
            received=1;
        }
        if(received){
            mes.mtype=12;
            //printf("%s\n",mes.mtext);
            //strcpy(mes.mtext,"Beat");
            msgsnd(queue,&mes,(sizeof(mes) - sizeof(long)), 0);
            t=3;
        }
        else 
            t--;
        sleep(1);
    }
    printf("\n");
    ReceiveMessage(queue);
    printf("Server is down\n");
    kill(getppid(),SIGTERM);
}

void PrintOptions() {
    printf("---------------------------\n");
    printf("1. Send private message\n");
    printf("2. Send public message\n");
    printf("3. Show 10 last public messages from current room\n");
    printf("4. Show all users registered to this server\n"); 
    printf("5. Show all existing rooms\n");
    printf("6. Show all rooms where this user belongs\n");
    printf("7. Show all users and rooms where they belong\n");
    printf("8. Join a room\n");
    printf("9. Leave a room\n");
    printf("10. Exit program\n"); 
    printf("Type task number -> ");
}

int main(int argc, char* argv[]) {
    printf("--CZAT by Olga Gerlich and Pawel Marczewski--\n");
    printf("Hi!\n");
    user.ulog = 0;
    int queue = Register();
    childpid=fork();
    if(queue == 0) return 0;
    else if(childpid==0){
        HeartBeat(queue);
        return 0;
    }
    PrintPressKey();
    while(1) {
        ReceiveMessage(queue);
        PrintOptions();
        switch (ReadNumber()) {
            case 1: { //sending message to room
                printf("Whom to send?: ");
                char recipient[100];
                scanf("%s", recipient);
                while(strcmp(recipient, user.uname) == 0) {
                    printf("You can not send a message to yourself\n");
                    printf("Whom to send?: ");
                    scanf("%s", recipient);
                }
                strcpy(mes.mto, recipient);

                printf("Write the message: ");
                char text[1000];
                getchar();
                scanf("%[^\n]s", mes.mtext);
                mes.msec = time(NULL);
                mes.mtype = 1;
                mes.mid = 0; //to differentiate private from public message
                strcpy(mes.mfrom, user.uname);
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    printf("%s", mes.mtext); 
                }
                else printf("An error has occurred!\n");
                PrintPressKey();
                break;
            }
            case 2: {
                printf("Rooms where you belong:\n");
                ListYourRooms(queue);
                printf("Enter number of a room where to sent a message: ");
                mes.mid = (long)ReadNumber();
                printf("Write the message: ");
                char text[1000];
                getchar();
                scanf("%[^\n]s", mes.mtext);
                mes.msec = time(NULL);
                mes.mtype = 2;
                strcpy(mes.mfrom, user.uname);
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    printf("%s", mes.mtext); 
                }
                else printf("An error has occurred!\n");
                PrintPressKey();
                break;
            } 
            case 3: { //writing 10 last messages
                printf("Rooms where you belong:\n");
                ListYourRooms(queue);
                printf("Enter number of a room from which you would like to read messages: ");
                mes.mid = (long)ReadNumber();
                mes.mtype = 3;
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    printf("%s", mes.mtext); 
                }
                else printf("An error has occurred!\n");
                PrintPressKey();
                break;
            } 
            case 4: {
                mes.mtype = 4;
                printf("All users on this server:\n");
                ListingRequest(queue);
                PrintPressKey();
                break;           
            }
            case 5: {
                mes.mtype = 5;
                printf("All rooms:\n");
                ListingRequest(queue);
                PrintPressKey();
                break;
            }
            case 6: {
                ListYourRooms(queue);
                PrintPressKey();
                break;
            }
            case 7: {
                mes.mtype = 7;
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    printf("%s\n", mes.mtext); 
                }
                else printf("An error has occurred!\n");
                PrintPressKey();
                break;
            }
            case 8: {
                //print all rooms and then ask where to add the user
                //if user enters room which doesn't exist, create new one
                mes.mtype = 5;
                printf("Available rooms:\n");
                ListingRequest(queue);
                printf("You can create a new room by writing unique room name\n");
                printf("Enter room name where you want to be added:\n-> ");
                char room[100];
                scanf("%s", room);
                mes.mtype = 8;
                strcpy(mes.mtext, room);
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    if((int)mes.mid == 0) {
                        printf("No empty room slots left\n");
                    }
                    else {
                        if(user.urooms[(int)mes.mid] == 0) {
                            user.urooms[(int)mes.mid] = 1;
                            printf("User correctly added to room '%s'\n", room);
                        }
                        else {
                            printf("User has already belonged to this room\n");
                        }
                    }
                }
                else printf("An error has occurred!\n");
                PrintPressKey();
                break;
            }
            case 9: {
                ListYourRooms(queue);
                if(mes.mid == 1) {
                    printf("This user belongs to only one room\n");
                    printf("Leaving the room cannot be perfomed\n");
                }
                else {
                    ReceiveMessage(queue);
                    printf("Enter a number of a room which you want to leave\n");
                    int room_number = ReadNumber();
                    mes.mid = (long)room_number;
                    user.urooms[room_number] = 0;
                    mes.mtype = 9;
                    msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                    int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                    if(receive > 0) {
                        printf("%s", mes.mtext);
                    }
                    else printf("An error has occurred!\n");
                }
                PrintPressKey();
                break;
            }
            case 10: { 
                ReceiveMessage(queue);
                mes.mtype = 10;
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    printf("Log out successful\n");
                    printf("%s\n", mes.mtext); 
                    kill(childpid,SIGTERM);
                    return 0;
                }
                else printf("An error has occurred!\n");
                PrintPressKey();
                return 0;
                break;
            } 
            default: {
                printf("Entered option does not exist!\n");
            }
        }
    }

    return 0;
}