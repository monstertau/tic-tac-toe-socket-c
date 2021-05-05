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

typedef struct arg_game
{
    GameManager *manager;
    GameBoard *gameBoard;
} ArgsGame;

GameManager *newGameManager()
{
    GameManager *manager = (GameManager *)malloc(sizeof(GameManager));
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    manager->managerMutex = mutex;
    return manager;
}

void freeGame(int code, GameManager *manager)
{
    pthread_mutex_lock(&manager->managerMutex);
    for (int i = 0; i < MAX_ROOM; i++)
    {
        if (manager->RoomGameList[i] == NULL || manager->RoomGameList[i]->roomID != code)
        {
            continue;
        }
        printf("[~] Start delete game room %d\n", manager->RoomGameList[i]->roomID);
        freeGameBoard(manager->RoomGameList[i]);
        manager->RoomGameList[i] = NULL;
        break;
    }

    pthread_mutex_unlock(&manager->managerMutex);
}
void handleGameOut(int cond, GameBoard *gameBoard, int player)
{
    int opponent = (player + 1) % 2;
    char msg[BUFF_SIZE];
    if (cond == 0)
    {
        sprintf(msg, "status~0~%s has left the room!!", gameBoard->playerList[player]->name);
        send(gameBoard->playerList[opponent]->sockfd, msg, strlen(msg), 0);
    }
}

/* main handling the game board. first the player 1 will call pthread_cond_wait() to wait for the player 2 to join.
 * If player 2 has joined, player 2 will signal to the thread and the thread will check if the player
 * 2 has joined this room. If 2 player has joined, the thread will start to handle the game*/
void *handleGameBoard(void *arguments)
{
    int status;
    if (pthread_detach(pthread_self()))
    {
        printf("[-] pthread_detach error\n");
    }
    ArgsGame *args = (ArgsGame *)arguments;
    GameManager *manager = args->manager;
    GameBoard *gameBoard = args->gameBoard;
    free(args); // no need to use so free it

    pthread_mutex_lock(&gameBoard->gameMutex);
    printf("[~] Room %d wait for player...!\n", gameBoard->roomID);
    while (getNumPlayer(gameBoard) < MAX_PLAYER)
    {
        pthread_cond_wait(&gameBoard->gameCond, &gameBoard->gameMutex);
    }
    printf("[+] Room %d Player 2 join and ready to start game!\n", gameBoard->roomID);
    pthread_mutex_unlock(&gameBoard->gameMutex);

    // SEND START PLAYING TO CLIENT
    sleep(1);
    for (int i = 0; i < MAX_PLAYER; i++)
    {
        send(gameBoard->playerList[i]->sockfd, "status~1~start playing", strlen("status~1~start playing"),
             0); // TODO: generalize this
    }
    // check if is playable, then send the update command and moving command to corresponding client
    sleep(0.5);
    while (1)
    {
        if (isPlayable(gameBoard))
        {
            printf("herrrrrrrreee\n");
            int concount = 0;
            int dropPlayer;
            for (int i = 0; i < MAX_PLAYER; i++)
            {
                bool is_winner = gameBoard->playerList[i]->label == gameBoard->winner;
                char msg[BUFF_SIZE];
                sprintf(msg, "done~%d~%s", is_winner,
                        (is_winner ? gameBoard->playerList[i]->name : gameBoard->playerList[(i + 1) % 2]->name));
                if (is_winner)
                    status = send(gameBoard->playerList[i]->sockfd, msg, BUFF_SIZE, 0);
                else
                    status = send(gameBoard->playerList[i]->sockfd, msg, BUFF_SIZE, 0);
                memset(msg, 0, BUFF_SIZE);
            }
            // Ask to continue
            for (int i = 0; i < MAX_PLAYER; i++)
            {
                char buff[BUFF_SIZE];
                recv(gameBoard->playerList[i]->sockfd, buff, BUFF_SIZE, 0);
                printf("Return %s %d\n", buff, strcmp(buff, "1"));
                if (strcmp(buff, "1") == 0)
                    concount++; // check is continue
                else
                    dropPlayer = i;
            }
            printf("Number of agree %d\n", concount);
            if (concount < MAX_PLAYER)
            {
                if (concount == 1)
                {
                    printf("send to %s\n", gameBoard->playerList[(dropPlayer + 1) % 2]->name);
                    char msg[BUFF_SIZE];
                    sprintf(msg, "status~1~%s is abort", gameBoard->playerList[dropPlayer]->name);
                    status = send(gameBoard->playerList[(dropPlayer + 1) % 2]->sockfd, msg, strlen(msg), 0);
                }
                break;
            }
            else
            {
                printf("Remake\n");
                remakeGameBroad(gameBoard);
                continue;
            }
        }
        for (int i = 0; i < MAX_PLAYER; i++)
        {
            char label = gameBoard->playerList[i % 2]->label;
            char opLabel = gameBoard->playerList[(i + 1) % 2]->label;
            char *sendBoard = serializeBoard(label, opLabel, gameBoard->size, gameBoard->board);
            status = send(gameBoard->playerList[i]->sockfd, sendBoard, strlen(sendBoard), 0);
            handleGameOut(status, gameBoard, i);
            char buff[BUFF_SIZE];
            status = recv(gameBoard->playerList[i]->sockfd, buff, BUFF_SIZE, 0);
            //TODO: handle if a client out of game
            handleGameOut(status, gameBoard, i);
        }
        for (int i = 0; i < MAX_PLAYER; i++)
        {
            // if (getWinner(gameBoard)){

            // }
            if (gameBoard->playerList[i]->isTurned)
            {
                status = send(gameBoard->playerList[i]->sockfd, "moving", strlen("moving"), 0);
                char buff[BUFF_SIZE];
                int bytes_received = recv(gameBoard->playerList[i]->sockfd, buff, BUFF_SIZE, 0);
                char **cmdArr = parseCmd(buff);
                int x = strtol(cmdArr[0], NULL, 10);
                int y = strtol(cmdArr[1], NULL, 10);
                Move *move = newMove(x, y);
                // TODO: Check is place yet
                if (isValidMove(gameBoard, move))
                {
                    makeMove(gameBoard, move, gameBoard->playerList[i]);
                    gameBoard->playerList[i]->isTurned = false;
                    gameBoard->playerList[(i + 1) % 2]->isTurned = true;
                }
                // else
                //     recv(gameBoard->playerList[i]->sockfd, "status~0~Inalid move", strlen("status~0~Inalid move"), 0);
                memset(buff, 0, BUFF_SIZE);
                break;
            }
        }
    }

    freeGame(gameBoard->roomID, manager);
    pthread_exit(NULL);
}

/* Get free room and assign new game to that index and create new thread to handling that gameboard, else return -1 */
int getFreeRoom(GameManager *manager, int size, char *name, int sockfd)
{
    pthread_mutex_lock(&manager->managerMutex);
    int j = -1;
    for (int i = 0; i < MAX_ROOM; i++)
    {
        if (manager->RoomGameList[i] == NULL)
        {
            // create new room
            pthread_t tid;
            Player *player1 = newPlayer(sockfd, name, true, 'X');
            manager->RoomGameList[i] = newGameBoard(player1, size);
            j = manager->RoomGameList[i]->roomID;

            ArgsGame *args = (ArgsGame *)malloc(sizeof(ArgsGame));
            args->manager = manager;
            args->gameBoard = manager->RoomGameList[i];
            pthread_create(&tid, NULL, &handleGameBoard, (void *)args);

            break;
        }
    }
    pthread_mutex_unlock(&manager->managerMutex);
    return j;
}

/* request join room if room code is incorrect return -1, if the room is full return -2,
 * if success assign player to that gameboard, and broadcast to thread that wait for player 2
 * that the player 2 has joined */
int requestJoinRoom(int code, char *name, int sockfd, GameManager *manager)
{
    pthread_mutex_lock(&manager->managerMutex);
    int j = -1;
    for (int i = 0; i < MAX_ROOM; i++)
    {
        if (manager->RoomGameList[i] == NULL || manager->RoomGameList[i]->roomID != code)
        {
            continue;
        }
        if (getNumPlayer(manager->RoomGameList[i]) >= MAX_PLAYER)
        {
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
