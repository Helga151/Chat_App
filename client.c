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
    int names_file = open("txt/names_file", O_RDWR | O_CREAT, 0644);
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
    char pid[20];
    sprintf(pid, "%ld", user.uid);

    write(names_file, user.uname, strlen(user.uname));
    write(names_file, "_", 1);
    write(names_file, pid, strlen(pid));
    write(names_file, "\n", 1);
    close(names_file);  
}

void ReadRoomName() {
    printf("Enter room name where you want to belong\n-> ");
    char room_name[100];
    scanf("%s", room_name);
    strcpy(user.uroom, room_name);
}

void PrintOptions() {
    printf("---------------------------\n");
    printf("1. Send a message\n");
    printf("2. Show 10 last public messages from current room\n");
    printf("3. Wyloguj uzytkownika - zamknij program\n"); //usunięcie nazwy użytkownika z pliku i zamknęcie programu
    //dodanie do struktury argumentu mówiącego o grupach w których jestem
    /*printf("3. Zapisz uzytkownika do pokoju\n"); //dopisanie nazwy użytkownika i pidu do pliku danego serwera
    printf("4. Wypisz uzytkownika z pokoju\n");
    printf("5. Wyswietl liste zalogowanych uzytkownikow\n"); //zapytać czy można usunąć
    printf("6. Wyswietl liste zarejestrowanych kanalow\n"); //jakie serwery
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
    int rcv = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, IPC_NOWAIT);
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

int Register() {
    int temp_q = msgget(12345678, 0644 | IPC_CREAT); //create temporary queue to send id to the server, typ logowania - 20
    ReadUsername();
    ReadRoomName();
    mes.mtype = 20;
    strcpy(mes.mfrom, user.uname);
    mes.mid = user.uid;
    strcpy(mes.mtext, user.uroom);
    msgsnd(temp_q, &mes, (sizeof(mes) - sizeof(long)), 0);
    msgrcv(temp_q, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
    if(strcmp(mes.mtext, "failed") == 0) {
        printf("No empty slots left!\n");
        printf("App is closing\n");
        PrintPressKey();
        return 0;
    }
    printf("Twoj login to %s a identyfikator %ld\n", user.uname, user.uid);
    user.ulog = 1;
    int queue = msgget(user.uid, 0644 | IPC_CREAT); //create queue for this client
    return queue;
    //printf("%d\n", queue);
}

int ReadNumber() {
    int choice;      
    while (scanf("%d", &choice) != 1) {
        scanf("%*s");
    }
    return choice;
} 

int main(int argc, char* argv[]) {
    printf("--CZAT by Olga Gerlich and Pawel Marczewski--\n");
    printf("Hi!\n");
    user.ulog = 0;
    int queue;
    if(user.ulog == 0) {
        queue = Register();
        if(queue == 0) return 0;
        printf("%s %ld %ld\n", mes.mtext, mes.mid, user.uid);
    }
    else {
        printf("Użytkownik już jest zalogowany\n");
    }
    PrintPressKey();
    while(1) {
        ReceiveMessage(queue);
        PrintOptions();
        printf("\n");
        ReceiveMessage(queue);
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
                msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
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
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
                int receive = msgrcv(queue, &mes, (sizeof(mes) - sizeof(long)), server_type, 0);
                if(receive > 0) {
                    printf("10 wiadomosci\n");
                    printf("%s\n", mes.mtext); 
                }
                else printf("An error has occurred!\n");
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
            default: {
                printf("Podana opcja nie istnieje!\n");
                //break;
            }
        }
    }

    return 0;
}