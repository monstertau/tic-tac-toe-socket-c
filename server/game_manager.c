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
#include <sys/time.h>
#include <asm-generic/errno.h>

#define BUFF_SIZE 1024

typedef struct arg_game {
    GameManager *manager;
    GameBoard *gameBoard;
} ArgsGame;
typedef struct arg_player {
    int client_sock;
    GameBoard *gameBoard;
} ArgsPlayer;

GameManager *newGameManager() {
    GameManager *manager = (GameManager *) malloc(sizeof(GameManager));
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    manager->managerMutex = mutex;
    return manager;
}

int getNumAvailableRoom(GameManager *gameManager) {
    pthread_mutex_lock(&gameManager->managerMutex);
    int r = 0;
    for (int i = 0; i < MAX_ROOM; i++) {
        if (gameManager->RoomGameList[i] == NULL) {
            r++;
        }
    }
    pthread_mutex_unlock(&gameManager->managerMutex);
    return r;
}

void freeGame(int code, GameManager *manager) {
    pthread_mutex_lock(&manager->managerMutex);
    for (int i = 0; i < MAX_ROOM; i++) {
        if (manager->RoomGameList[i] == NULL || manager->RoomGameList[i]->roomID != code) {
            continue;
        }
        printf("[GAME MANAGEMENT] [INFO] Start delete game room %d\n", manager->RoomGameList[i]->roomID);
        freeGameBoard(manager->RoomGameList[i]);
        manager->RoomGameList[i] = NULL;

        break;
    }
    pthread_mutex_unlock(&manager->managerMutex);
    printf("[GAME MANAGEMENT] [INFO] Free Room Available %d\n", getNumAvailableRoom(manager));
}


void waitState(GameBoard *gameBoard, GameManager *gameManager) {
    int waitTimeInMs = 60 * 1000; // wait for 3 minutes
    struct timeval tv;
    struct timespec ts;

    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + waitTimeInMs / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (waitTimeInMs % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);
    pthread_mutex_lock(&gameBoard->gameMutex);
    printf("[ROOM %d] [INFO] Board size %dx%d wait for player...!\n", gameBoard->roomID, gameBoard->size,
           gameBoard->size);
    while (getNumPlayer(gameBoard) < MAX_PLAYER) {
        if (getNumPlayer(gameBoard) == 0) {
            printf("[ROOM %d] [INFO] None waiting for player!\n", gameBoard->roomID);
            freeGame(gameBoard->roomID, gameManager);
            pthread_exit(NULL);
        }
        int n = pthread_cond_timedwait(&gameBoard->gameCond, &gameBoard->gameMutex, &ts);
        if (n == 0) {
            continue;
        } else if (n == ETIMEDOUT) {
            printf("[ROOM %d] [INFO] Timeout %d seconds waiting for player!\n", gameBoard->roomID, waitTimeInMs / 1000);
            freeGame(gameBoard->roomID, gameManager);
            pthread_exit(NULL);
        }

    }
    printf("[ROOM %d] [INFO] Player 2 join and ready to start game!\n", gameBoard->roomID);
    pthread_mutex_unlock(&gameBoard->gameMutex);
}

int checkDisconnectedStatus(GameBoard *gameBoard, GameManager *manager) {
    if (getNumPlayer(gameBoard) == 0) {
        freeGame(gameBoard->roomID, manager);
        pthread_exit(NULL);
    } else if (getNumPlayer(gameBoard) < MAX_PLAYER) {
        Player *playerAvail = getAvailablePlayer(gameBoard);
        int byte_sends = send(playerAvail->sockfd, "status~1~Opponent has left! Wait for another opponent...",
                              strlen("status~1~Opponent has left! Wait for another opponent..."), MSG_NOSIGNAL);
        if (byte_sends <= 0) {
            freeGame(gameBoard->roomID, manager);
            pthread_exit(NULL);
        }
        waitState(gameBoard, manager);
        return 1;
    }
    return 0;
}

void *handleRecvPlayer(void *arguments) {
    int status;
    if (pthread_detach(pthread_self())) {
        printf("[ERROR] pthread_detach error\n");
    }
    ArgsPlayer *args = (ArgsPlayer *) arguments;
    int client_fd = args->client_sock;
    GameBoard *gameBoard = args->gameBoard;
    free(args); // no need to use so free it

    int cur = 0;
    for (int i = 0; i <= MAX_PLAYER; i++) {
        if (client_fd == gameBoard->playerList[i]->sockfd) {
            cur = i;
            break;
        }
    }
    char buff[BUFF_SIZE];
    while ((status = recv(client_fd, buff, BUFF_SIZE, 0)) > 0) {
        char **cmdArr = parseCmd(buff);
        CmdValue cmdValue = getCommand(cmdArr);
        if (cmdValue.type == MOVING) {
            printf("[ROOM %d] [INFO] Player %d want to make move!\n", gameBoard->roomID, cur);
            Move *move = newMove(cmdValue.moveCmd.x, cmdValue.moveCmd.y);
            // TODO: Check is place yet
            if (isValidMove(gameBoard, move)) {
                makeMove(gameBoard, move, gameBoard->playerList[cur]);
                printf("[ROOM %d] [INFO] Player %d make move x: %d - y: %d\n", gameBoard->roomID, cur, move->x,
                       move->y);

            } else
                send(client_fd, "status~0~Invalid move", strlen("status~0~Invalid move"),
                     MSG_NOSIGNAL);
            gameBoard->hasUpdate = true;
            pthread_cond_broadcast(&gameBoard->gameCond);
            memset(buff, 0, BUFF_SIZE);
        } else if (cmdValue.type == CHAT) {
            ChatMessage *chatMsg = (ChatMessage *) malloc(sizeof(ChatMessage));
            char *msgClone = (char *) malloc(sizeof(char) * MAXCMDLENGTH);
            sprintf(msgClone, "%s: %s", gameBoard->playerList[cur]->name, cmdValue.chatCmd.chatMsg);
            printf("[ROOM %d] [INFO] New Message: %s\n", gameBoard->roomID, msgClone);
            chatMsg->message = msgClone;
            chatMsg->next = gameBoard->chatMessage;
            gameBoard->chatMessage = chatMsg;
            gameBoard->hasUpdate = true;
            pthread_cond_broadcast(&gameBoard->gameCond);
            memset(buff, 0, BUFF_SIZE);
        }

    }

    if (status <= 0) {
        printf("[ROOM %d] [INFO] Player %d has left!\n", gameBoard->roomID, cur);
        freePlayer(gameBoard->playerList[cur]);
        gameBoard->playerList[cur] = NULL;
    }
    gameBoard->hasUpdate = true;
    pthread_cond_broadcast(&gameBoard->gameCond);
    pthread_exit(NULL);
}

/* main handling the game board. first the player 1 will call pthread_cond_wait() to wait for the player 2 to join.
 * If player 2 has joined, player 2 will signal to the thread and the thread will check if the player
 * 2 has joined this room. If 2 player has joined, the thread will start to handle the game
 * */
void *handleGameBoard(void *arguments) {
    int status;
    if (pthread_detach(pthread_self())) {
        printf("[ERROR] pthread_detach error\n");
    }
    ArgsGame *args = (ArgsGame *) arguments;
    GameManager *manager = args->manager;
    GameBoard *gameBoard = args->gameBoard;
    free(args); // no need to use so free it

    waitState(gameBoard, manager); // into wait state

    // SEND START PLAYING TO CLIENT
    sleep(2);

    printf("[ROOM %d] [INFO] Start Playing\n", gameBoard->roomID);

    while (1) {
        if (checkDisconnectedStatus(gameBoard, manager)) {
            continue;
        }
        for (int i = 0; i < MAX_PLAYER; i++) {

            int byte_sends = send(gameBoard->playerList[i]->sockfd, "status~1~Next Turn!",
                                  strlen("status~1~Next Turn!"),
                                  MSG_NOSIGNAL);
            if (byte_sends <= 0) {
                printf("[ROOM %d] [INFO] Player %d has left!\n", gameBoard->roomID, i);
                freePlayer(gameBoard->playerList[i]);
                gameBoard->playerList[i] = NULL;
            }
        }
        if (checkDisconnectedStatus(gameBoard, manager)) {
            continue;
        }
        usleep(50000);
        printf("[ROOM %d] [INFO] Current board status\n", gameBoard->roomID);
        printBoard(gameBoard);

        // send update command
        for (int i = 0; i < MAX_PLAYER; i++) {
            char label = gameBoard->playerList[i % 2]->label;
            char opLabel = 'O';
            if (label == 'O') {
                opLabel = 'X';
            }
            char *sendBoard = serializeBoard(label, opLabel, gameBoard->size, gameBoard->board);
            status = send(gameBoard->playerList[i]->sockfd, sendBoard, strlen(sendBoard), MSG_NOSIGNAL);
            if (status <= 0) {
                printf("[ROOM %d] [INFO] Player %d has left!\n", gameBoard->roomID, i);
                freePlayer(gameBoard->playerList[i]);
                gameBoard->playerList[i] = NULL;
                continue;
            }
            usleep(100000);
            char *sendChat = serializeChat(gameBoard);
            status = send(gameBoard->playerList[i]->sockfd, sendChat, strlen(sendChat), MSG_NOSIGNAL);
            if (status <= 0) {
                printf("[ROOM %d] [INFO] Player %d has left!\n", gameBoard->roomID, i);
                freePlayer(gameBoard->playerList[i]);
                gameBoard->playerList[i] = NULL;
            }
        }
        usleep(100000);
        // broadcast update to watcher
        for (int i = 0; i < MAX_WATCHER; i++) {
            char *sendBoard = serializeBoard('-', '-', gameBoard->size, gameBoard->board);
            if (gameBoard->watcherList[i] != NULL) {
                status = send(gameBoard->watcherList[i]->sockfd, sendBoard, strlen(sendBoard), MSG_NOSIGNAL);
                if (status <= 0) {
                    freePlayer(gameBoard->watcherList[i]);
                    gameBoard->watcherList[i] = NULL;
                    continue;
                }
                usleep(100000);
                char *sendChat = serializeChat(gameBoard);
                status = send(gameBoard->watcherList[i]->sockfd, sendChat, strlen(sendChat), MSG_NOSIGNAL);
                if (status <= 0) {
                    freePlayer(gameBoard->watcherList[i]);
                    gameBoard->watcherList[i] = NULL;
                    continue;
                }
            }
        }

        if (checkDisconnectedStatus(gameBoard, manager)) {
            continue;
        }


        usleep(200000);
        if (!isPlayable(gameBoard)) {
            int concount = 0;
            int dropPlayer;
            for (int i = 0; i < MAX_PLAYER; i++) {
                bool is_winner = gameBoard->playerList[i]->label == gameBoard->winner;
                char msg[BUFF_SIZE] = {0};
                sprintf(msg, "done~%d~%s", is_winner,
                        (is_winner ? gameBoard->playerList[i]->name : gameBoard->playerList[(i + 1) % 2]->name));
                status = send(gameBoard->playerList[i]->sockfd, msg, BUFF_SIZE, MSG_NOSIGNAL);

            }
            // Ask to continue
            for (int i = 0; i < MAX_PLAYER; i++) {
                char buff[BUFF_SIZE] = {0};
                recv(gameBoard->playerList[i]->sockfd, buff, BUFF_SIZE, 0);
                printf("Return %s %d\n", buff, strcmp(buff, "1"));
                if (strcmp(buff, "1") == 0)
                    concount++; // check is continue
                else
                    dropPlayer = i;
            }
            printf("[ROOM %d] [INFO] Number of agree %d\n", gameBoard->roomID, concount);
            if (concount < MAX_PLAYER) {
                if (concount == 1) {
                    char msg[BUFF_SIZE];
                    sprintf(msg, "status~1~%s is abort", gameBoard->playerList[dropPlayer]->name);
                    status = send(gameBoard->playerList[(dropPlayer + 1) % 2]->sockfd, msg, strlen(msg), MSG_NOSIGNAL);
                }
                break;
            } else {
                printf("Remake\n");
                remakeGameBoard(gameBoard);
                continue;
            }
        }

        // send moving command
        int i = getTurnedPlayer(gameBoard);
        status = send(gameBoard->playerList[i]->sockfd, "moving", strlen("moving"), MSG_NOSIGNAL);
        if (status <= 0) {
            printf("[ROOM %d] [INFO] Player %d has left!\n", gameBoard->roomID, i);
            freePlayer(gameBoard->playerList[i]);
            gameBoard->playerList[i] = NULL;
        }
        if (checkDisconnectedStatus(gameBoard, manager)) {
            continue;
        }
        pthread_mutex_lock(&gameBoard->gameMutex);
        while (!gameBoard->hasUpdate) {
            pthread_cond_wait(&gameBoard->gameCond, &gameBoard->gameMutex);
        }
        pthread_mutex_unlock(&gameBoard->gameMutex);
        gameBoard->hasUpdate = false;
    }

    freeGame(gameBoard->roomID, manager);
    pthread_exit(NULL);
}

/* Get free room and assign new game to that index and create new thread to handling that gameboard, else return -1 */
int getFreeRoom(GameManager *manager, int size, char *name, int sockfd) {
    pthread_mutex_lock(&manager->managerMutex);
    int j = -1;
    for (int i = 0; i < MAX_ROOM; i++) {
        if (manager->RoomGameList[i] == NULL) {
            // create new room
            pthread_t tid;
            Player *player1 = newPlayer(sockfd, name, true, 'X');
            GameBoard *gameBoard = newGameBoard(size);
            addPlayer(gameBoard, player1);
            manager->RoomGameList[i] = gameBoard;
            j = manager->RoomGameList[i]->roomID;

            ArgsGame *args = (ArgsGame *) malloc(sizeof(ArgsGame));
            args->manager = manager;
            args->gameBoard = manager->RoomGameList[i];
            pthread_create(&tid, NULL, &handleGameBoard, (void *) args);

            ArgsPlayer *argsPlayer = (ArgsPlayer *) malloc(sizeof(ArgsPlayer));
            argsPlayer->client_sock = sockfd;
            argsPlayer->gameBoard = manager->RoomGameList[i];
            pthread_create(&tid, NULL, &handleRecvPlayer, (void *) argsPlayer);
            break;
        }
    }
    pthread_mutex_unlock(&manager->managerMutex);
    printf("[GAME MANAGEMENT] [INFO] Free Room Available %d\n", getNumAvailableRoom(manager));
    return j;
}

/* request join room if room code is incorrect return -1, if the room is full return -2,
 * if success assign player to that gameboard, and broadcast to thread that wait for player 2
 * that the player 2 has joined */
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

        char label = 'O';
        for (int i = 0; i < MAX_PLAYER; i++) {
            if (gameBoard->playerList[i] != NULL && gameBoard->playerList[i]->label == 'O') {
                label = 'X';
                break;
            }
        }
        Player *player2 = newPlayer(sockfd, name, false, label);
        addPlayer(gameBoard, player2);

        pthread_t tid;
        ArgsPlayer *argsPlayer = (ArgsPlayer *) malloc(sizeof(ArgsPlayer));
        argsPlayer->client_sock = sockfd;
        argsPlayer->gameBoard = manager->RoomGameList[i];
        pthread_create(&tid, NULL, &handleRecvPlayer, (void *) argsPlayer);

        pthread_cond_broadcast(&gameBoard->gameCond);
        pthread_mutex_unlock(&gameBoard->gameMutex);

        j = i;
        break;
    }
    pthread_mutex_unlock(&manager->managerMutex);
    printf("[GAME MANAGEMENT] [INFO] Free Room Available %d\n", getNumAvailableRoom(manager));
    return j;
}

int requestWatchRoom(int code, char *name, int sockfd, GameManager *manager) {
    pthread_mutex_lock(&manager->managerMutex);
    // pthread_mutex_lock(&manager->managerMutex);
    int j = -1;
    for (int i = 0; i < MAX_ROOM; i++) {
        if (manager->RoomGameList[i] == NULL || manager->RoomGameList[i]->roomID != code)
            continue;
        if (getNumWatcher(manager->RoomGameList[i]) >= MAX_WATCHER) {
            j = -2;
            break;
        }
        GameBoard *gameBoard = manager->RoomGameList[i];

        Player *watcher = newPlayer(sockfd, name, false, ' ');
        addWatcher(gameBoard, watcher);

        j = i;
        break;
    }
    pthread_mutex_unlock(&manager->managerMutex);
    return j;
}

char *getListRoom(GameManager *manager) {
    // char tmp
    char *lstMsg = (char *) malloc(sizeof(char) * BUFF_SIZE);
    strcat(lstMsg, "list~");
    for (int i = 0; i < MAX_ROOM; i++) {
        char tmp[100];
        GameBoard *gb = manager->RoomGameList[i];
        if (gb != NULL) {
            sprintf(tmp, "%d~%d~%d/%d~%d/%d", gb->roomID, gb->size, getNumPlayer(gb), MAX_PLAYER, getNumWatcher(gb),
                    MAX_WATCHER); //id size player/max wacher/max
            // printf(tmp);
            strcat(lstMsg, tmp);
            strcat(lstMsg, "~");
        }
    }
    printf(lstMsg);
    printf("\n");
    return lstMsg;
} 