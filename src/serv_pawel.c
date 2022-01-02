#include "../lib/serv_pawel.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

Message mes;

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

int CountClientsInFile(){
    int fd = open("txt/names_file", O_RDWR | O_CREAT, 0644);
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
    char linia[150],id[20];
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

void LogoutClient(int current_queue,int id, int* arr_queue, int clients_all, User *arr_users){
    int receive = msgrcv(current_queue, &mes, (sizeof(mes) - sizeof(long)), 3, IPC_NOWAIT);
    if(receive > 0) {
        printf("wylogowuje %s\n",arr_users[id].uname);
        DeleteLine("txt/names_file",arr_users[id]);
        mes.mtype=100; //tu pomyśleć jak dać server_type
        strcpy(mes.mtext,"Godbye!!!\n");
        msgsnd(current_queue, &mes, (sizeof(mes) - sizeof(long)), 0);
        DeleteFromQueue(arr_queue,clients_all,id,arr_users);
        printf("done\n");
    }
    //else printf("An error has occurred!\n");
}