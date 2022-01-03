#ifndef SERV_OLGA
#define SERV_OLGA

#include "struct.h"

int CheckIfUnique(char *text);
void RegisterClient(int *arr_queue, int clients_all, int temp_q, User *arr_users, char (*arr_rooms) [100]);
void SendMessage(int current_queue, int* arr_queue, int clients_all, User *arr_users);
void WriteOldMessages(int current_queue);
void PrintRoomsList(int current_queue);
void AddUserToFile(User user);
void PrintUsernames(int current_queue, int *arr_queue, int clients_all, User *arr_users);

#endif
