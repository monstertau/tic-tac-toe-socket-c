//
// Created by monstertau on 24/04/2021.
//

#include <malloc.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "game_manager.h"
#include "msg_parser.h"

#define BUFF_SIZE 1024

typedef struct arg_game {
    GameManager *manager;
    GameBoard *gameBoard;
} ArgsGame;

GameManager *newGameManager() {
    GameManager *manager = (GameManager *) malloc(sizeof(GameManager));
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    manager->managerMutex = mutex;
    return manager;
}

void freeGame(int code, GameManager *manager) {
    pthread_mutex_lock(&manager->managerMutex);
    for (int i = 0; i < MAX_ROOM; i++) {
        if (manager->RoomGameList[i] == NULL || manager->RoomGameList[i]->roomID != code) {
            continue;
        }
        printf("[~] Start delete game room %d\n", manager->RoomGameList[i]->roomID);
        freeGameBoard(manager->RoomGameList[i]);
        manager->RoomGameList[i] = NULL;
        break;
    }

    pthread_mutex_unlock(&manager->managerMutex);
}

void *handleGameBoard(void *arguments) {
    if (pthread_detach(pthread_self())) {
        printf("[-] pthread_detach error\n");
    }
    ArgsGame *args = (ArgsGame *) arguments;
    GameManager *manager = args->manager;
    GameBoard *gameBoard = args->gameBoard;
    free(args); // no need to use so free it

    pthread_mutex_lock(&gameBoard->gameMutex);
    printf("[~] Room %d wait for player...!\n", gameBoard->roomID);
    while (getNumPlayer(gameBoard) < MAX_PLAYER) {
        pthread_cond_wait(&gameBoard->gameCond, &gameBoard->gameMutex);
    }
    printf("[+] Room %d Player 2 join and ready to start game!\n", gameBoard->roomID);
    pthread_mutex_unlock(&gameBoard->gameMutex);

    // SEND START PLAYING TO CLIENT
    sleep(1);
    for (int i = 0; i < MAX_PLAYER; i++) {
        send(gameBoard->playerList[i]->sockfd, "status~1~start playing", strlen("status~1~start playing"),
             0); // TODO: generalize this
    }

    while (isPlayable(gameBoard)) {
        for (int i = 0; i < MAX_PLAYER; i++) {
            char label = gameBoard->playerList[i % 2]->label;
            char opLabel = gameBoard->playerList[(i + 1) % 2]->label;
            char *sendBoard = serializeBoard(label, opLabel, gameBoard->size, gameBoard->board);
            send(gameBoard->playerList[i]->sockfd, sendBoard, strlen(sendBoard), 0);
            char buff[BUFF_SIZE];
            recv(gameBoard->playerList[i]->sockfd, buff, BUFF_SIZE, 0);
        }
        for (int i = 0; i < MAX_PLAYER; i++) {
            if (gameBoard->playerList[i]->isTurned) {
                send(gameBoard->playerList[i]->sockfd, "moving", strlen("moving"), 0);
                char buff[BUFF_SIZE];
                //TODO: handle if a client out of game
                int bytes_received = recv(gameBoard->playerList[i]->sockfd, buff, BUFF_SIZE, 0);
                char **cmdArr = parseCmd(buff);
                int x = strtol(cmdArr[0], NULL, 10);
                int y = strtol(cmdArr[1], NULL, 10);
                Move *move = newMove(x, y);
                makeMove(gameBoard, move, gameBoard->playerList[i]);

                gameBoard->playerList[i]->isTurned = false;
                gameBoard->playerList[(i + 1) % 2]->isTurned = true;
                break;
            }
        }
    }


    freeGame(gameBoard->roomID, manager);
    pthread_exit(NULL);
}

int getFreeRoom(GameManager *manager, int size, char *name, int sockfd) {
    pthread_mutex_lock(&manager->managerMutex);
    int j = -1;
    for (int i = 0; i < MAX_ROOM; i++) {
        if (manager->RoomGameList[i] == NULL) {
            // create new room
            pthread_t tid;
            Player *player1 = newPlayer(sockfd, name, true, 'X');
            manager->RoomGameList[i] = newGameBoard(player1, size);
            j = manager->RoomGameList[i]->roomID;

            ArgsGame *args = (ArgsGame *) malloc(sizeof(ArgsGame));
            args->manager = manager;
            args->gameBoard = manager->RoomGameList[i];
            pthread_create(&tid, NULL, &handleGameBoard, (void *) args);

            break;
        }
    }
    pthread_mutex_unlock(&manager->managerMutex);
    return j;
}

int requestJoinRoom(int code, char *name, int sockfd, GameManager *manager) {
    pthread_mutex_lock(&manager->managerMutex);
    int j = -1;
    for (int i = 0; i < MAX_ROOM; i++) {
        if (manager->RoomGameList[i] == NULL || manager->RoomGameList[i]->roomID != code) {
            continue;
        }
        if (getNumPlayer(manager->RoomGameList[i]) >= MAX_PLAYER) {
            j = -2;
            break;
        }
        GameBoard *gameBoard = manager->RoomGameList[i];
        pthread_mutex_lock(&gameBoard->gameMutex);

        Player *player2 = newPlayer(sockfd, name, false, 'O');
        addPlayer2(gameBoard, player2);
        pthread_cond_broadcast(&gameBoard->gameCond);

        pthread_mutex_unlock(&gameBoard->gameMutex);

        j = i;
        break;

    }
    pthread_mutex_unlock(&manager->managerMutex);
    return j;
}

