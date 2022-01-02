#ifndef SERV_PAWEL
#define SERV_PAWEL

#include "struct.h"

void ShowQueue(int clients_all, User *arr_users);
void DeleteFromQueue(int *arr_queue, int clients_all, int id, User *arr_users);
int CountClientsInFile();
void DeleteLine(char plik[20],User Client);
void LogoutClient(int current_queue,int id, int* arr_queue, int clients_all, User *arr_users);

#endif