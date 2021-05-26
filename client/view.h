//
// Created by monstertau on 24/05/2021.
//

#ifndef CLIENT_VIEW_H
#define CLIENT_VIEW_H

#endif //CLIENT_VIEW_H

#include "ncurses.h"

#define MAX_MSG_LEN 100
#define GAME_TITLE " Game Board "
#define SYSTEM_TITLE " System Message "
#define CHAT_TITLE " Chat "
#define LABEL_TITLE " Labels "
#define COMMAND_TITLE " Game Board "
#define MAX_BOARD_SIZE 16
#define MAX_MENU 4
#define MENU_TITLE " Tic Tac Toe Online Game "

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
    int gameCode;
    int gameSize;
    char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    char myLabel;
    char opLabel;
    int lastX;
    int lastY;
} GameInfo;
typedef struct ChatMessage_ {
    int whoami;
    char *message;
    struct ChatMessage_ *next;
} ChatMessage;

extern ChatInfo chatInfo;
extern ChatMessage *chatMessageHead;

void insertChatMessage(int whoami, char *msg);

GameBoardInfo newGameBoardInfo(int xMax, int yMax, int gameSize);

void drawNormalTitle(WINDOW *window, char *title);


void drawLightTitle(WINDOW *window, char *title);

WINDOW *newSubWindow(char *title, int widthX, int heightY, int startX, int startY, bool inTab);


WINDOW *drawBanner(int xMax);

WINDOW *newGameWindow(int xMax, int yMax, bool inTab);

WINDOW *newSystemWindow(int xMax, int yMax, bool inTab);

WINDOW *newChatWindow(int xMax, int yMax, bool inTab);

WINDOW *newLabelWindow(int xMax, int yMax, bool inTab);

WINDOW *newCommandWindow(int xMax, int yMax, bool inTab);

GameScreen *newGameScreen(int xMax, int yMax);

void setSystemMessage(GameScreen *screen, char *msg);


void setLabelMessage(GameScreen *screen, char label, char opLabel);

GameBoardInfo drawGameBoard(GameScreen *screen, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int gameSize);

void refreshView(GameScreen *screen);

void setNormalTitle(GameScreen *screen);


int movingGameWindow(GameBoardInfo *gameBoardInfo, GameScreen *gameScreen, GameInfo *gameInfo);

void drawChatDialog(GameScreen *gameScreen);

int movingChatWindow(GameScreen *gameScreen, char *msg, int *i);

WINDOW *newDialogWindow(char *title, int *xMax, int *yMax);

void newMenuWindow(int *choice, int *xMax, int *yMax);

WINDOW *newStatusWindow(char *title, char *label, bool isError);

char *inputNewBox(char *title, char *label);