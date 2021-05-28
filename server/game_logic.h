//
// Created by monstertau on 12/04/2021.
//



#ifndef SERVER_GAME_LOGIC_H
#define SERVER_GAME_LOGIC_H

#include <stdbool.h>
#include <pthread.h>

#define MAX_BOARD_SIZE 16
#define MAX_PLAYER 2
#define MAX_WATCHER 5
#endif //SERVER_GAME_LOGIC_H
struct Move_ {
    int x;
    int y;
};
typedef struct Move_ Move;

struct Player_ {
    int sockfd;
    char name[30];
    bool isTurned;
    char label;
};
typedef struct Player_ Player;


struct GameBoard_ {
    int roomID;
    pthread_cond_t gameCond;
    pthread_mutex_t gameMutex;
    int size;
    char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    Player *playerList[MAX_PLAYER];
    Player *watcherList[MAX_WATCHER];
    char winner;
};
typedef struct GameBoard_ GameBoard;

int generateRoomCode();

Move *newMove(int x, int y);

Player *newPlayer(int sockfd, char *name, bool isTurned, char label);

GameBoard *newGameBoard(int size);

void remakeGameBoard(GameBoard *gameBoard);

bool isWin(GameBoard *gameBoard, Move *move, char label);

void addPlayer(GameBoard *gameBoard, Player *player);

void makeMove(GameBoard *gameBoard, Move *move, Player *player);

bool isValidMove(GameBoard *gameBoard, Move *move);

bool isPlayable(GameBoard *gameBoard);

Player *getWinner(GameBoard *gameBoard);

char *serializeBoard(char label, char opLabel, int size, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE]);

int getNumPlayer(GameBoard *gameBoard);

void freeGameBoard(GameBoard *gameBoard);

void freePlayer(Player *player);

int getTurnedPlayer(GameBoard *gameBoard);

Player *getAvailablePlayer(GameBoard *gameBoard);

void printBoard(GameBoard *gameBoard);

int getNumWatcher(GameBoard *GameBoard);

void addWatcher(GameBoard *gameBoard, Player *player);

char *currentBoard(int size, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE]);