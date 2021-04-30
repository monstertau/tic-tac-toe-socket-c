//
// Created by monstertau on 24/04/2021.
//

#ifndef SERVER_GAME_MANAGER_H
#define SERVER_GAME_MANAGER_H
#define MAX_ROOM 2

#include <stdbool.h>
#include "game_logic.h"

#endif //SERVER_GAME_MANAGER_H


typedef struct GameManager_ {
    GameBoard *RoomGameList[MAX_ROOM];
    pthread_mutex_t managerMutex;
} GameManager;

GameManager *newGameManager();

int getFreeRoom(GameManager *manager, int size, char *name, int sockfd);

int requestJoinRoom(int code, char *name, int sockfd, GameManager *manager);

void joinRoom(int code, char *name, int sockfd, GameManager *manager);