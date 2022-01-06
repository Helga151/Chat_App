#ifndef SERV_OLGA
#define SERV_OLGA

#include "struct.h"

int CheckIfUnique(char *text);
void AddUserToFile(User user);
int AddRoomToArray(int tmp, User *current_user, char *text, char (*arr_rooms)[100]);
char *WriteCurrentTime(time_t sec);
void RegisterClient(int *arr_queue, int clients_all, int temp_q, User *arr_users, char (*arr_rooms) [100]);
void SendPrivateMessage(int current_queue, int* arr_queue, int clients_all, User *arr_users);
void SendPublicMessage(int current_queue, int* arr_queue, int clients_all, User *arr_users, User *current_user);
void WriteOldMessages(int current_queue, User *current_user);
void PrintUsernames(int current_queue, int *arr_queue, int clients_all, User *arr_users);
void PrintRoomsList(int current_queue);
void WriteUsersRooms(int current_queue, User *current_user, char (*arr_rooms)[100]);
void WriteAllUsersRooms(int current_queue, int *arr_queue, int clients_all, User *arr_users, char (*arr_rooms)[100]);
void AddUserToRoom(int current_queue, char (*arr_rooms)[100], User *current_user); 
void RemoveUserFromRoom(int current_queue, User *current_user);

#endif
