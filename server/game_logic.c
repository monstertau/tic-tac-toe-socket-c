//
// Created by monstertau on 12/04/2021.
//

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "game_logic.h"

#define BUFF_SIZE 1024

int generateRoomCode() {
    int code = rand();
    return code;
}

Move *newMove(int x, int y) {
    Move *move = (Move *) malloc(sizeof(Move));
    move->x = x;
    move->y = y;
    return move;
}

Player *newPlayer(int sockfd, char *name, bool isTurned, char label) {
    Player *player = (Player *) malloc(sizeof(Player));
    player->sockfd = sockfd;
    player->isTurned = isTurned;
    player->label = label;
    strcpy(player->name, name);
    return player;
}

GameBoard *newGameBoard(int size) {
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    GameBoard *game = (GameBoard *) malloc(sizeof(GameBoard));
    game->gameCond = cond;
    game->gameMutex = mutex;
    game->size = size;
    for (int i = 0; i < 2; i++) {
        game->playerList[i] = NULL;
    }
    game->roomID = generateRoomCode();
    game->winner = 0;
    game->hasUpdate = false;
    game->chatMessage = NULL;
    for (int i = 0; i < MAX_BOARD_SIZE; i++) {
        for (int j = 0; j < MAX_BOARD_SIZE; j++) {
            game->board[i][j] = '\0';
        }
    }
    return game;
}

void remakeGameBoard(GameBoard *gameBoard) {
    memset(gameBoard->board, 0, sizeof gameBoard->board); //Clear broad
    gameBoard->chatMessage = NULL;
    gameBoard->hasUpdate = false;
    gameBoard->winner = '\0';                             //clear old winner
}

void addPlayer(GameBoard *gameBoard, Player *player) {
    for (int i = 0; i < 2; i++) {
        if (gameBoard->playerList[i] == NULL) {
            gameBoard->playerList[i] = player;
            break;
        }
    }
}

void addWatcher(GameBoard *gameBoard, Player *player) {
    for (int i = 0; i < MAX_WATCHER; i++) {
        if (gameBoard->watcherList[i] == NULL) {
            gameBoard->watcherList[i] = player;
            break;
        }
    }
}

void makeMove(GameBoard *gameBoard, Move *move, Player *player) {
    int x = move->x;
    int y = move->y;
    gameBoard->board[x][y] = player->label;
    if (isWin(gameBoard, move, player->label)) {
        gameBoard->winner = player->label;
        printf("%c is the winner!!\n", player->label);
    }
}

bool isWin(GameBoard *gameBoard, Move *move, char label) {
    int n = (gameBoard->size < 5 ? gameBoard->size : 5);
    // printf("Number of win %d\n", n);

    int count = 0;
    for (int i = 0; i < gameBoard->size; i++) {
        if (gameBoard->board[move->x][i] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    count = 0;
    for (int i = 0; i < gameBoard->size; i++) {
        if (gameBoard->board[i][move->y] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    // check left to right downward diagonal
    count = 0;
    int startx = move->x - (move->x < move->y ? move->x : move->y);
    int starty = move->y - (move->x < move->y ? move->x : move->y);
    for (int i = 0; i < gameBoard->size; i++) {
        int tmpx = startx + i;
        int tmpy = starty + i;
        if (tmpx >= gameBoard->size || tmpy >= gameBoard->size)
            break;
        if (gameBoard->board[tmpx][tmpy] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    // check left to right upward diagonal
    count = 0;
    startx = move->x + (gameBoard->size - 1 - move->x);
    starty = move->y - (gameBoard->size - 1 - move->x);
    for (int i = 0; i < gameBoard->size; i++) {
        int tmpx = startx - i;
        int tmpy = starty + i;
        if (tmpx < 0 || tmpy >= gameBoard->size)
            break;
        if (gameBoard->board[tmpx][tmpy] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    return false;
}

bool isValidMove(GameBoard *gameBoard, Move *move) {
    if (move->x >= gameBoard->size || move->y >= gameBoard->size)
        return false;
    if (gameBoard->board[move->x][move->y])
        return false;
    return true;
}

bool isPlayable(GameBoard *gameBoard) {
    return (gameBoard->winner == 0);
}


char *serializeBoard(char label, char opLabel, int size, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE]) {
    char *buff = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(buff, 0, BUFF_SIZE);
    strcpy(buff, "update");
    strcat(buff, "~");
    char tmp[10];
    sprintf(tmp, "%c~%c~%d", label, opLabel, size);
    strcat(buff, tmp);
    strcat(buff, "~");

    char tmp2[MAX_BOARD_SIZE * MAX_BOARD_SIZE] = {0};
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            char boardLabel[2];
            if (board[i][j] != '\0') {
                sprintf(boardLabel, "%c", board[i][j]);
            } else {
                sprintf(boardLabel, "-");
            }
            strcat(tmp2, boardLabel);
        }
    }
    strcat(buff, tmp2);
    return buff;
}

void printBoard(GameBoard *gameBoard) {
    for (int i = 0; i < gameBoard->size; i++) {
        for (int j = 0; j < gameBoard->size; j++) {
            char boardLabel[2];
            if (gameBoard->board[i][j] != '\0') {
                printf("%c", gameBoard->board[i][j]);
            } else {
                printf("-");
            }
        }
        printf("\n");
    }
}

int getNumPlayer(GameBoard *gameBoard) {
    int j = 0;
    for (int i = 0; i < MAX_PLAYER; i++) {
        if (gameBoard->playerList[i] != NULL) {
            j++;
        }
    }
    return j;
}

int getNumWatcher(GameBoard *GameBoard) {
    int j = 0;
    for (int i = 0; i < MAX_WATCHER; i++) {
        if (GameBoard->watcherList[i] != NULL)
            j++;
    }
    return j;
}

void freePlayer(Player *player) {
    close(player->sockfd);
    free(player);
}

void freeGameBoard(GameBoard *gameBoard) {
    for (int i = 0; i < MAX_PLAYER; i++) {
        if (gameBoard->playerList[i] != NULL) {
            freePlayer(gameBoard->playerList[i]);
            gameBoard->playerList[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_WATCHER; i++) {
        if (gameBoard->watcherList[i] != NULL) {
            freePlayer(gameBoard->watcherList[i]);
            gameBoard->watcherList[i] = NULL;
        }
    }
    free(gameBoard);
}

int getTurnedPlayer(GameBoard *gameBoard) {
    int x = 0, o = 0;
    for (int i = 0; i < gameBoard->size; i++) {
        for (int j = 0; j < gameBoard->size; j++) {
            if (gameBoard->board[i][j] == 'X') {
                x++;
            } else if (gameBoard->board[i][j] == 'O') {
                o++;
            }
        }
    }
    int r = 0;
    if (x <= o) {
        for (int i = 0; i < MAX_PLAYER; i++) {
            if (gameBoard->playerList[i]->label == 'X') {
                r = i;
            }
        }
    } else {
        for (int i = 0; i < MAX_PLAYER; i++) {
            if (gameBoard->playerList[i]->label == 'O') {
                r = i;
            }
        }
    }
    return r;
}

Player *getAvailablePlayer(GameBoard *gameBoard) {
    Player *player;
    for (int i = 0; i < MAX_PLAYER; i++) {
        if (gameBoard->playerList[i] != NULL) {
            player = gameBoard->playerList[i];
        }
    }
    return player;
}

char *serializeChat(GameBoard *gameBoard) {
    char *buff = (char *) malloc(sizeof(char) * MAX_BUFF_MSG);
    memset(buff, 0, MAX_BUFF_MSG);
    strcpy(buff, "chat~");
    ChatMessage *tmp = gameBoard->chatMessage;
    while (1) {
        if (tmp == NULL) {
            break;
        }
        strcat(buff, tmp->message);
        strcat(buff, "~");
        tmp = tmp->next;
    }
    printf("send chat%s\n", buff);
    return buff;
}