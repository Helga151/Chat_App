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

typedef struct User User;
struct User {
    long uid; //unique id - pid
    char uname[100];
    int ulog; //is user logged in: 0 - no, 1 - yes
};


void DeleteClientFromFile(User Client){
    int fd=open("names_file.txt",O_RDWR);
    char pid[20],id[20];
    strcat(pid,Client.uname);
    pid[strlen(pid)]='_';
    pid[strlen(pid)+1]='\0';
    sprintf(id, "%ld", Client.uid);
    strcat(pid,id);
    printf("%s\n",pid);
    struct stat sb;
    char c,d;
    int i=0,poczatek,koniec,teraz,rozm;
    int same=0;
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
int main(){
    User klient;
    klient.uid=9999;
    strcpy(klient.uname,"eminem");
    DeleteClientFromFile(klient);
}