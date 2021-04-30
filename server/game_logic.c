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

GameBoard *newGameBoard(Player *player1, int size) {
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    GameBoard *game = (GameBoard *) malloc(sizeof(GameBoard));
    game->gameCond = cond;
    game->gameMutex = mutex;
    game->size = size;
    game->playerList[0] = player1;
    game->roomID = generateRoomCode();
    game->winner = 0;
    return game;
}


void addPlayer2(GameBoard *gameBoard, Player *player2) {
    gameBoard->playerList[1] = player2;
}

void makeMove(GameBoard *gameBoard, Move *move, Player *player) {
    int x = move->x;
    int y = move->y;
    gameBoard->board[x][y] = player->label;
}

bool isValidMove(GameBoard *gameBoard, Move *move) {
    int x = move->x;
    int y = move->y;
    return gameBoard->board[x][y] == 0;
}

bool isPlayable(GameBoard *gameBoard) {
    // TODO: policy for 3x3 only, need to modify
    for (int i = 0; i < 3; i++) {
        // diagonal
        if (gameBoard->board[i][0] == gameBoard->board[i][1] && gameBoard->board[i][1] == gameBoard->board[i][2]) {
            gameBoard->winner = gameBoard->board[i][0];
        }
        // horizontal
        if (gameBoard->board[0][i] == gameBoard->board[1][i] && gameBoard->board[1][i] == gameBoard->board[2][i]) {
            gameBoard->winner = gameBoard->board[0][i];
        }
    }
    // cross
    if (gameBoard->board[0][0] == gameBoard->board[1][1] && gameBoard->board[1][1] == gameBoard->board[2][2]) {
        gameBoard->winner = gameBoard->board[0][0];
    }

    if (gameBoard->board[2][0] == gameBoard->board[1][1] && gameBoard->board[1][1] == gameBoard->board[0][2]) {
        gameBoard->winner = gameBoard->board[2][0];
    }

    return gameBoard->winner == 0;
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

    char tmp2[MAX_BOARD_SIZE * MAX_BOARD_SIZE];
    memset(tmp2, 0, MAX_BOARD_SIZE * MAX_BOARD_SIZE);
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

int getNumPlayer(GameBoard *gameBoard) {
    int j = 0;
    for (int i = 0; i < MAX_PLAYER; i++) {
        if (gameBoard->playerList[i] != NULL) {
            j++;
        }
    }
    return j;
}

void freePlayer(Player *player) {
    close(player->sockfd);
    free(player);
}

void freeGameBoard(GameBoard *gameBoard) {
    for (int i = 0; i < MAX_PLAYER; i++) {
        freePlayer(gameBoard->playerList[i]);
        gameBoard->playerList[i] = NULL;
    }
    free(gameBoard);
}