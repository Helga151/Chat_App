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

void PrintOptions() {
    printf("---------------------------\n");
    printf("1. Send a message\n");
    printf("2. Show 10 last public messages from current room\n");
    printf("3. Wyloguj uzytkownika - zamknij program\n"); //usunięcie nazwy użytkownika z pliku i zamknęcie programu
    printf("4. Show all existing rooms\n");
    printf("5. Show all users registered to this server\n"); 
    printf("6. Add user to a room\n");
    printf("7. Write rooms where user belongs\n");
    printf("8. Remove user from a room\n");
    //dodanie do struktury argumentu mówiącego o grupach w których jestem
    /*printf("4. Wypisz uzytkownika z pokoju\n");
    printf("7. Wyswietl liste uzytkownikow zapisanych do pokoi\n");*/ //dodatkowe pytanie: z jakich pokoi
    printf("Type task number -> ");
}

void PrintPressKey() {
    printf("Press ENTER to continue\n");
    getchar(); //newline was left in the input stream, get it
    char enter = 0;
    while (enter != '\r' && enter != '\n') { 
        enter = getchar(); 
    }
}

void ReceiveMessage(int queue) {
    int rcv = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), msg_from_server, IPC_NOWAIT);
    if(rcv > 0) {
        if(strcmp(mes.mto, "server") == 0){ //public message
            if(strcmp(mes.mfrom, user.uname) != 0) {
                printf("New public message from %s: %s \n", mes.mfrom, mes.mtext);
                PrintPressKey();
            }
        }
        /*printf("Previous message was succesfully sent.\n");
        printf("If you do not see it on other client, please continue reading printed instructions\n");
        printf("Message will be shown after finishing current tasks\n");*/
        else { //private message
            printf("New private message from %s: %s \n", mes.mfrom, mes.mtext);
            PrintPressKey();
        }
    }
}

void ListingRequest(int queue) {
    msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
    if(receive > 0) {
        printf("%s\n", mes.mtext); 
    }
    else printf("An error has occurred!\n");
}

int Register() {
    int temp_q = msgget(12345678, 0644 | IPC_CREAT); //create temporary queue to send id to the server, typ logowania - 20
    ReadUsername();
    //nwm czm to nie działa, ale czasami daje Segmentation fault (core dumped)
    /*mes.mtype = 4;
    msgsnd(temp_q, &mes, (sizeof(mes) - sizeof(long)), 0);
    int receive = msgrcv(temp_q, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
    if(receive > 0) {
        if(strcmp(mes.mtext, "nofile") == 0) {
            printf("No created rooms yet\n");
            printf("Enter room name where you want to belong\n-> ");
        }
        else {
            printf("Available rooms:\n");
            printf("%s\n", mes.mtext); 
            printf("Choose a room from above or create a new one\n-> ");
        }
    }
    else {
        printf("An error has occurred!\n");
        return 0;
    }*/
    printf("Enter room name where you want to belong\n-> ");
    char room_name[100];
    scanf("%s", room_name);
    
    mes.mtype = 20;
    strcpy(mes.mfrom, user.uname);
    mes.mid = user.uid;
    strcpy(mes.mtext, room_name);
    msgsnd(temp_q, &mes, (sizeof(mes) - sizeof(long)), 0);
    int receive = msgrcv(temp_q, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
    if(receive > 0) {
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

int ReadNumber() {
    int choice;      
    while (scanf("%d", &choice) != 1) {
        scanf("%*s");
    }
    return choice;
}
void HeartBeat(int queue){
    while (1){
        if (msgrcv(queue,&mes,(sizeof(mes) - sizeof(long)),11,0)>0){
            mes.mtype=12;
            strcpy(mes.mtext,"Beat");
            msgsnd(queue,&mes,(sizeof(mes) - sizeof(long)), 0);
        }
    }
}

void ListYourRooms(int queue) {
    mes.mtype = 7;
    msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
    int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
    if(receive > 0) {
        printf("Your rooms:\n");
        printf("%s\n", mes.mtext);
    }
    else printf("An error has occurred!\n");
}

int main(int argc, char* argv[]) {
    printf("--CZAT by Olga Gerlich and Pawel Marczewski--\n");
    printf("Hi!\n");
    user.ulog = 0;
    int queue = Register();
    if(queue == 0) return 0;
    else if(fork()==0){
        HeartBeat(queue);
    }
    PrintPressKey();
    while(1) {
        ReceiveMessage(queue);
        PrintOptions();
        switch (ReadNumber()) {
            case 1: { //sending message to room
                printf("Send private (1) or public (2) message\n");
                int type = ReadNumber();                
                if(type == 1) {
                    printf("Whom to send?: ");
                    char recipient[100];
                    scanf("%s", recipient);
                    while(strcmp(recipient, user.uname) == 0) {
                        printf("You can not send a message to yourself\n");
                        printf("Whom to send?: ");
                        scanf("%s", recipient);
                    }
                    strcpy(mes.mto, recipient);
                }
                else if(type == 2) strcpy(mes.mto, "server");
                else {
                    printf("This option does not exits\n");
                    PrintPressKey();
                    break;
                }
                printf("Napisz wiadomosc: ");
                char text[1000];
                getchar();
                scanf("%[^\n]s", mes.mtext);
                mes.msec = time(NULL);
                mes.mtype = 1;
                mes.mid = user.uid;
                strcpy(mes.mfrom, user.uname);
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), msg_from_server, 0);
                if(strcmp(mes.mtext, "none") == 0) {
                    printf("Written recipent does not exist\n");
                }
                else {
                    printf("Message sent\n");
                }
                break;
            } 
            case 2: { //writing 10 last messages
                mes.mtype = 2;
                printf("Last messages:\n");
                ListingRequest(queue);
                PrintPressKey();
                break;
            } 
            case 3: { //wyloguj
                mes.mtype = 3;
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    printf("Wylogowano\n");
                    printf("%s\n", mes.mtext); 
                    return 0;
                }
                else printf("An error has occurred!\n");
                PrintPressKey();
                return 0;
                break;
            } 
            case 4: {
                mes.mtype = 4;
                printf("All rooms:\n");
                ListingRequest(queue);
                PrintPressKey();
                break;
            }
            case 5: {
                mes.mtype = 5;
                printf("All users on this server:\n");
                ListingRequest(queue);
                PrintPressKey();
                break;           
            }
            case 6: {
                //print all rooms and then ask where to add the user
                //if user enters room which doesn't exist, create new one
                mes.mtype = 4;
                printf("Available rooms:\n");
                ListingRequest(queue);
                printf("You can create a new room by writing unique room name\n");
                printf("Enter room name where you want to be added:\n-> ");
                char room[100];
                scanf("%s", room);
                mes.mtype = 6;
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
            case 7: {
                ListYourRooms(queue);
                PrintPressKey();
                break;
            }
            case 8: {
                ListYourRooms(queue);
                if(mes.mid == 1) {
                    printf("This user belongs to only one room\n");
                    printf("Removing from the room cannot be perfomed\n");
                }
                else {
                    printf("Enter number of room from which user will be erased\n");
                    int room_number;
                    scanf("%d", &room_number);
                    mes.mid = (long)room_number;
                    user.urooms[room_number] = 0;
                    mes.mtype = 8;
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
            default: {
                printf("Podana opcja nie istnieje!\n");
            }
        }
    }

    return 0;
}