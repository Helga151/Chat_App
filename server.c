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
#include <math.h>
#include <fcntl.h>

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

typedef struct User User;
struct User {
    long uid; //unique id - pid
    char uname[100];
    int ulog; //is user logged in: 0 - no, 1 - yes
    char uroom[100]; //room to which user belongs
};


//methods
void ShowQueue(int clients_all, User *arr_users){
    for(int i = 0; i<clients_all; i++) {
        printf("%ld\t",arr_users[i].uid);
        printf("%s\t",arr_users[i].uname);
        printf("%d\n",arr_users[i].ulog);
    }
}
void DeleteFromQueue(int *arr_queue, int clients_all, int id, User *arr_users){
    int i;
    for(i = id; arr_queue[i+2]!=0 || i+1<clients_all; i++) {
        arr_users[i].uid=arr_users[i+1].uid;
        arr_users[i].ulog=arr_users[i+1].ulog;
        strcpy(arr_users[i].uname,arr_users[i+1].uname);
    }
    i++;
    arr_queue[i]=0;
    arr_users[i].uid=0;
    arr_users[i].ulog=0;
    arr_users[i].uname[0]='\0';
    ShowQueue(clients_all,arr_users);
}
void RegisterClient(int *arr_queue, int clients_all, int temp_q, User *arr_users) {
    //user has logged in
    int receive = msgrcv(temp_q, &mes, (sizeof(mes) - sizeof(long)), 20, IPC_NOWAIT);
    if(receive > 0) {
        int zm = 0;
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] == 0) { //first empty slot
                arr_queue[i] = msgget(mes.mid, 0644 | IPC_CREAT);
                strcpy(arr_users[i].uname, mes.mfrom);
                arr_users[i].uid = mes.mid;
                arr_users[i].ulog = 1;
                strcpy(arr_users[i].uroom, mes.mtext);
                printf("%s %ld %s %d\n", arr_users[i].uname, arr_users[i].uid, mes.mtext, arr_users[i].ulog);
                //printf("%s %ld %s\n", mes.mfrom, mes.mid, mes.mtext);
                zm = 1;
                //check if room name is unique - yes(create new room)
                int rooms_file = open("rooms_file.txt", O_RDWR | O_CREAT, 0644);
                if(rooms_file < 0) {
                    printf("Could not open the file\n");
                }
                else {
                    int unique = 0; //0 - unique
                    char letter;
                    char arr[100];
                    memset(arr, 0, 100);
                    int j = 0;
                    while(read(rooms_file, &letter, 1) > 0) {
                        if(letter == '\n') {
                            printf("%s\n", arr);
                            if(strcmp(arr, mes.mtext) == 0) {
                                printf("Room '%s' exists\n", mes.mtext);
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
                    if(unique == 0) {
                        write(rooms_file, mes.mtext, strlen(mes.mtext));
                        write(rooms_file, "\n", 1);
                        close(rooms_file); 
                    }
                }
                break;
            }
        }
        if(zm == 0) { //to many clients
            printf("Nie ma wolnych miejsc\n");
            strcpy(mes.mtext, "failed");
        }
        mes.mtype = server_type;
        msgsnd(temp_q, &mes, (sizeof(mes) - sizeof(long)), 0);
    }
}
int CountClientsInFile(){
     int fd= open("names_file.txt", O_RDWR | O_CREAT, 0644);
    char c;
    int i=0;
    while((read(fd, &c, 1))>0){
        if (c=='\n')
            i++;
     }
	lseek(fd, 0, SEEK_SET);
    c='0'+i;
    write(fd,&c,1);
    c='\n';
    write(fd,&c,1);
    close(fd);
    printf("liczba klientow: %d\n",i);
    return i;
}
void DeleteLine(char plik[20],User Client){
    char linia[20],id[20];
    memset(linia,0,20);
    sprintf(linia, "%s_%ld",Client.uname, Client.uid);
    printf("%s\n",linia);

    int fd=open(plik,O_RDWR,0644);
    struct stat sb;
    char c,d;
    int i=0,poczatek,koniec,teraz,rozm;
    int same=1;
    while((read(fd, &c, 1))>0){
        if (c=='\n'){
            if (same==1){//jezeli wczesniejsza linijka byla taka sama
                printf("same!!!\n");
                koniec=lseek(fd,0,SEEK_CUR);
                rozm=koniec-poczatek;
                //read(fd,&d,1);
                while(read(fd,&d,1)>0){   
                    lseek(fd,poczatek++,SEEK_SET);
                    write(fd,&d,1);
                    lseek(fd,++koniec,SEEK_SET);
                }
                lstat(plik, &sb);
                ftruncate(fd,sb.st_size-rozm);
            }
            i=0;
            same=1;
            poczatek=lseek(fd,0,SEEK_CUR);
        }
        else{
            if (linia[i++]!=c)
                same=0;
        }
     }
     if (same==1){
         printf("same!!!\n");
        koniec=lseek(fd,0,SEEK_CUR);
        rozm=koniec-poczatek;
        //read(fd,&d,1);
        while(read(fd,&d,1)>0){   
            lseek(fd,poczatek++,SEEK_SET);
            write(fd,&d,1);
            lseek(fd,++koniec,SEEK_SET);
        }
        lstat(plik, &sb);
        ftruncate(fd,sb.st_size-rozm);
    }
    close(fd);
}

void DeleteClientFromFile(User Client){
    int fd=open("names_file.txt",O_RDWR,0644);
    char pid[20],id[20];
    strcat(pid,Client.uname);
    strcat(pid,"_");
    sprintf(id, "%ld", Client.uid);
    strcat(pid,id);
    printf("%s\n",pid);
    struct stat sb;
    char c,d;
    int i=0,poczatek,koniec,teraz,rozm;
    int same=1;
    while((read(fd, &c, 1))>0){
        if (c=='\n'){
            if (same==1){//jezeli wczesniejsza linijka byla taka sama
                koniec=lseek(fd,0,SEEK_CUR);
                rozm=koniec-poczatek;
                while(read(fd,&d,1)>0){   
                    lseek(fd,poczatek++,SEEK_SET);
                    write(fd,&d,1);
                    lseek(fd,++koniec,SEEK_SET);
                }
                lstat("names_file.txt", &sb);
                ftruncate(fd,sb.st_size-rozm);
            }
            i=0;
            same=1;
            poczatek=lseek(fd,0,SEEK_CUR);
        }
        else{
            if (pid[i++]!=c)
                same=0;
        }
     }
     if (same==1){
        koniec=lseek(fd,0,SEEK_CUR);
        rozm=koniec-poczatek;
        while(read(fd,&d,1)>0){   
            lseek(fd,poczatek++,SEEK_SET);
            write(fd,&d,1);
            lseek(fd,++koniec,SEEK_SET);
        }
        lstat("names_file.txt", &sb);
        ftruncate(fd,sb.st_size-rozm);
    }
    close(fd);
}

void LogoutClient(int current_queue,int id, int* arr_queue, int clients_all, User *arr_users){
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 3, IPC_NOWAIT);
    if(receive > 0) {
        printf("wylogowuje %s\n",arr_users[id].uname);
        DeleteLine("names_file.txt",arr_users[id]);
        mes.mtype=server_type;
        strcpy(mes.mtext,"Godbye!!!\n");
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
        DeleteFromQueue(arr_queue,clients_all,id,arr_users);
        printf("done\n");
    }
    //else printf("An error has occurred!\n");

}

void SendMessage(int current_queue, int* arr_queue, int clients_all, User *arr_users) {
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 1, IPC_NOWAIT);
    if(receive > 0) {
        //printf("%s\n", mes.mtext);
        struct tm* current_time;
        current_time = localtime(&mes.msec);

        char time[10];
        sprintf(time, "%02d:%02d:%02d", current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
        printf("%s %s\n", time, mes.mtext);

        if(strcmp(mes.mto, "server") == 0) {
        //public - send the message to every client
            for(int i = 0; i < clients_all; i++) {
                if(arr_queue[i] != 0) {
                    mes.mtype = server_type;
                    msgsnd(arr_queue[i], &mes, (sizeof(mes) - sizeof(long)), 0);
                    printf("poszlo\n");
                }
            }
            //write the message to the chat file
            int chat = open("chat.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
            write(chat, time, 8);
            write(chat, " user: ", 7);
            write(chat, &mes.mfrom, strlen(mes.mfrom));
            write(chat, " message: ", 10);
            write(chat, &mes.mtext, strlen(mes.mtext));
            write(chat, "\n", 1);
            close(chat);
        }
        else {
            //private - send the message to one, particular user
            //find this user 
            int zm = 0;
            for(int i = 0; i < clients_all; i++) {
                if(arr_queue[i] != 0 && arr_users[i].ulog == 1 && strcmp(arr_users[i].uname, mes.mto) == 0) {
                    mes.mtype = server_type;
                    msgsnd(arr_queue[i], &mes, (sizeof(mes) - sizeof(long)), 0);
                    printf("poszlo\n");
                    zm = 1;
                    break;
                }
            }
            if(zm == 0) {
                //send feedback to client that written recipent does not exist
                mes.mtype = server_type;
                strcpy(mes.mtext, "none");
                printf("nie ma takiego uzytkownika\n");
            }
            msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
        }
    }
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
    User arr_users[clients_all];
    for(int i = 0; i < clients_all; i++) {
        arr_queue[i] = 0;
        arr_users[i].uid = 0;
        arr_users[i].ulog = 0;
        arr_users[i].uname[0]='\0';
    }
    //int chat = open("chat.txt", O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
    int receive;
    //ShowQueue(clients_all,arr_users);
    while(1) {
        RegisterClient(arr_queue, clients_all, temp_q, arr_users);
        //checking if there is any message
        for(int i = 0; i < clients_all; i++) {
            if(arr_queue[i] != 0) { //look for a not empty queue
                SendMessage(arr_queue[i], arr_queue, clients_all, arr_users);
                WriteOldMessages(arr_queue[i]);
                LogoutClient(arr_queue[i],i,arr_queue,clients_all,arr_users);
            }
            else {
                break; //no more clients to check
            }
        }
        sleep(1);
    }
    //delete all temp files after server stops*/
    return 0;
}