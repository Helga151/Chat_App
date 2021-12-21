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

typedef struct Message Message;
struct Message {
    long mtype;
    char mtext[1000];
    char mfrom[100]; //who sends the message
    long mid; //id of sender
    char mto[100]; //who should receive the message
    time_t msec; //when the message was sent, in seconds
}mes;

struct User {
    long uid; //unique id - pid
    char uname[100];
    int ulog; //is user logged in: 0 - no, 1 - yes
}user;
long server_type = 100;

Message SendAndReceive(int queue, Message mes_to, int wanted_type) {
    msgsnd(queue, &mes_to, (sizeof(mes_to) - sizeof(long)), 0);
    Message mes_from;
    msgrcv(queue, &mes_from, (sizeof(mes_from) - sizeof(long)), wanted_type, 0);
    return mes_from;
}

void CheckIfUnique() { //is written name unique
    int unique = 1; //0 - unique
    int names_file = open("names_file.txt", O_RDWR | O_CREAT, 0644);
    if(names_file < 0) {
        printf("Błąd otwarcia pliku\n");
        return;
    }
    char name[100];
    while(unique == 1) { //read client name untill it is unique
        unique = 0;
        printf("Podaj nazwe uzytkownika (bez spacji): ");
        scanf("%s", name);
        char letter;
        char arr[100];
        int i = 0;
        while(read(names_file, &letter, 1) > 0) {
            if(letter == ' ') {
                if(strcmp(arr, name) == 0) {
                    printf("Uzytkownik o nazwie '%s' juz istnieje\n", name);
                    unique = 1;
                    break;
                }
            }
            else if(letter == '\n') {
                memset(arr, 0, 100);
                i = 0;
            }
            else {
                arr[i] = letter;
                i++;
            }
        }
    }
    //if unique - add name to the text file
    strcpy(user.uname, name);
    user.uid = getpid();
    char pid[20];
    sprintf(pid, "%ld", user.uid);

    write(names_file, user.uname, strlen(user.uname));
    write(names_file, " ", 1);
    write(names_file, pid, strlen(pid));
    write(names_file, "\n", 1);
    close(names_file);  
}

void LogIn() { //type 1

}

void LogOut() {

}

void PrintOptions() {
    printf("---------------------------\n");
    printf("2. Wyloguj uzytkownika - zamknij program\n"); //usunięcie nazwy użytkownika z pliku i zamknęcie programu
    //dodanie do struktury argumentu mówiącego o grupach w których jestem
    printf("3. Zapisz uzytkownika do pokoju\n"); //dopisanie nazwy użytkownika i pidu do pliku danego serwera
    printf("4. Wypisz uzytkownika z pokoju\n");
    printf("5. Wyswietl liste zalogowanych uzytkownikow\n"); //zapytać czy można usunąć
    printf("6. Wyswietl liste zarejestrowanych kanalow\n"); //jakie serwery
    printf("7. Wyswietl liste uzytkownikow zapisanych do pokoi\n"); //dodatkowe pytanie: z jakich pokoi
    printf("2. Wyswietl max 10 ostatnich wiadomosci wyslanych na wskazanym kanale\n");
    printf("1. Wyslij wiadomosc na wskazany kanal\n");
    printf("10. Wyslij wiadomosc prywatna do danego uzytkownika\n");
    printf("Podaj numer czynnosci -> ");
}

void PrintPressKey() {
    printf("Press ENTER to continue\n");
    getchar(); //newline was left in the input stream, get it
    char enter = 0;
    while (enter != '\r' && enter != '\n') { 
        enter = getchar(); 
    }
}

int main(int argc, char* argv[]) {
    printf("--CZAT by Olga Gerlich--\n");
    printf("Czesc!\n");
    user.ulog = 0;
    int queue;
    if(user.ulog == 0) {
        CheckIfUnique();
        printf("Twoj login to %s a identyfikator %ld\n", user.uname, user.uid);
        getchar();
        user.ulog = 1;
        queue = msgget(user.uid, 0644 | IPC_CREAT); //create queue for this client

        int temp_q = msgget(12345678, 0644 | IPC_CREAT); //create temporary queue to send id to the server, typ logowania - 20
        mes.mtype = 20;
        strcpy(mes.mfrom, user.uname);
        mes.mid = user.uid;
        strcpy(mes.mtext, "udane");
        msgsnd(temp_q, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
    else {
        printf("Użytkownik już jest zalogowany\n");
    }
    PrintPressKey();
    while(1) {
        PrintOptions();
        int choice;
        scanf("%d", &choice);
        printf("\n");

        switch (choice) {
            case 1: { //sending message to room
                printf("Napisz wiadomosc: ");
                char text[1000];
                getchar();
                scanf("%[^\n]s", text);
                strcpy(mes.mtext, text);
                mes.msec = time(NULL);
                mes.mtype = 1;
                mes.mid = user.uid;
                strcpy(mes.mfrom, user.uname);
                //mes.mto = 1; //1 - server key
                msgsnd(queue, &mes, (sizeof(mes) - sizeof(long)), 0);
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
            case 3: {

            } 
            case 4: {

            } 
            case 5: {

            } 
            case 6: {

            } 
            case 7: {

            }     
            case 8: {

            } 
            case 9: {

            } 
            case 10: {

            } 
            case 11: {

            } 
            case 12: {

            } 
            default: {
                printf("Podana opcja nie istnieje!\n");
                break;
            }
        }


        //---READ A MESSAGE---
        /*if(choice == 1) {
            printf("Napisz wiadomosc: ");
            char text[100];
            scanf("%s", text);
            
            /*ZAPYTAc CZEMU NIE POBIERA WIADOMOsCI
            fgets(text, sizeof(text), stdin); //one line with spaces
            scanf("%[^\n]%*c", text); 

            strcpy(mes.mtext, text);
            msgsnd(queue, &mes, sizeof(mes), 0);
        }
        else if(choice == 4) {
            strcpy(mes.mtext, "write");
            msgsnd(queue, &mes, sizeof(mes), 0);
        }
        else if(choice == 5) {
            strcpy(mes.mtext, "end");
            msgsnd(queue, &mes, sizeof(mes), 0);
            break;
        }*/
    }

    return 0;
}