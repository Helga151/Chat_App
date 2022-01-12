#ifndef SERV_PAWEL
#define SERV_PAWEL

#include "struct.h"

void ShowQueue(int clients_all, User *arr_users);
void DeleteFromQueue(int *arr_queue, int clients_all, int id, User *arr_users);
int CountClientsInFile();
void DeleteLine(User Client);
void LogoutClient(int id, int* arr_queue, int clients_all, User *arr_users);
void SendHeartbeat(int* arr_time,int id, int* arr_queue,int clients_all, User *arr_users);
void InformAbautLogout(int id,char name[100], int* arr_queue, int clients_all, User *arr_users);


#endif