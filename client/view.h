//
// Created by monstertau on 24/05/2021.
//

#ifndef CLIENT_VIEW_H
#define CLIENT_VIEW_H

#endif //CLIENT_VIEW_H

#include "ncurses.h"

#define MAX_BOARD_SIZE 16
typedef struct GameScreen_ {
    int xMax, yMax;
    WINDOW *banner;
    WINDOW *gameWin;
    WINDOW *systemWin;
    WINDOW *chatWin;
    WINDOW *labelWin;
    WINDOW *commandWin;
} GameScreen;

typedef struct GameBoardInfo_ {
    int yBoard, xBoard, xCur, yCur, xMax, yMax;
} GameBoardInfo;

typedef struct ChatInfo_ {
    int startX, startY;
    int xCur, yCur;
} ChatInfo;

typedef struct GameInfo_ {
    int gameSize;
    char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    char myLabel;
    char opLabel;
} GameInfo;
typedef struct ChatMessage_ {
    int whoami;
    char *message;
    struct ChatMessage_ *next;
} ChatMessage;

GameScreen *newGameScreen(int xMax, int yMax);

void setNormalTitle(GameScreen *screen);