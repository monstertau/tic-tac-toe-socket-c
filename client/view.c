//
// Created by monstertau on 24/05/2021.
//

#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include "view.h"

ChatInfo chatInfo;
ChatMessage *chatMessageHead = NULL;

void insertChatMessage(int whoami, char *msg) {
    ChatMessage *chatMsg = (ChatMessage *) malloc(sizeof(ChatMessage));
    chatMsg->whoami = whoami;
    char *msgClone = (char *) malloc(sizeof(char) * strlen(msg));
    memset(msgClone, 0, strlen(msg));
    if (whoami == 0) {
        sprintf(msgClone, "You: %s", msg);
    } else {
        sprintf(msgClone, "Opponent: %s", msg);
    }
    chatMsg->message = msgClone;
    chatMsg->next = chatMessageHead;
    chatMessageHead = chatMsg;
}

GameBoardInfo newGameBoardInfo(int xMax, int yMax, int gameSize) {
    GameBoardInfo info;
    info.yBoard = gameSize * 2 + gameSize -
                  1;
    info.xBoard = gameSize * 4 + gameSize - 1;
    info.yCur = 1 + (yMax - info.yBoard) / 2;
    info.xCur = 2 + (xMax - info.xBoard) / 2;
    info.xMax = xMax;
    info.yMax = yMax;
    return info;
}

void drawNormalTitle(WINDOW *window, char *title) {
    mvwprintw(window, 0, 2, title);
}

void drawLightTitle(WINDOW *window, char *title) {
    wattron(window, A_REVERSE);
    mvwprintw(window, 0, 2, title);
    wattroff(window, A_REVERSE);
}

WINDOW *newSubWindow(char *title, int widthX, int heightY, int startX, int startY, bool inTab) {
    WINDOW *window = newwin(heightY, widthX, startY, startX);
    box(window, 0, 0);
    if (inTab) {
        drawLightTitle(window, title);
    } else {
        drawNormalTitle(window, title);
    }

    return window;
}


WINDOW *drawBanner(int xMax) {
    WINDOW *window = newwin(5, xMax, 0, 0);
    mvwprintw(window, 0, xMax / 4, "   __  _         __                __           ");
    mvwprintw(window, 1, xMax / 4, "  / /_(_)____   / /_____ ______   / /_____  ___ ");
    mvwprintw(window, 2, xMax / 4, " / __/ / ___/  / __/ __ `/ ___/  / __/ __ \\/ _ \\");
    mvwprintw(window, 3, xMax / 4, "/ /_/ / /__   / /_/ /_/ / /__   / /_/ /_/ /  __/");
    mvwprintw(window, 4, xMax / 4, "\\__/_/\\___/   \\__/\\__,_/\\___/   \\__/\\____/\\___/ ");
    return window;
}

WINDOW *newGameWindow(int xMax, int yMax, bool inTab) {
    WINDOW *window = newSubWindow(GAME_TITLE,
                                  xMax * 3 / 4, yMax - 10, 0, 5, inTab);
    return window;
}

WINDOW *newSystemWindow(int xMax, int yMax, bool inTab) {
    WINDOW *window = newSubWindow(SYSTEM_TITLE,
                                  xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0, inTab);
    return window;
}

WINDOW *newChatWindow(int xMax, int yMax, bool inTab) {
    WINDOW *window = newSubWindow(CHAT_TITLE, xMax * 1 / 4, yMax - 10, xMax * 3 / 4 + 1, 5, inTab);

    mvwhline(window, yMax - 20, 2, ACS_HLINE, xMax * 1 / 4 - 4);
    mvwprintw(window, yMax - 20 + 1, 2, "Message: ");
    int x, y;
    getyx(window, y, x);
    chatInfo.xCur = x;
    chatInfo.startX = x;
    chatInfo.yCur = y;
    chatInfo.startY = y;
    keypad(window, true);
    return window;
}

WINDOW *newLabelWindow(int xMax, int yMax, bool inTab) {
    WINDOW *window = newSubWindow(LABEL_TITLE,
                                  xMax * 1 / 4, 5, xMax * 3 / 4 + 1, yMax - 5, inTab);
    int heightY = getmaxy(window);
    mvwprintw(window, heightY / 2, 2, "Your Label:       Opponent's Label: ");
    return window;
}

WINDOW *newCommandWindow(int xMax, int yMax, bool inTab) {
    WINDOW *window = newSubWindow(COMMAND_TITLE,
                                  xMax * 3 / 4, 5, 0, yMax - 5, inTab);
    int heightY = getmaxy(window);
    mvwprintw(window, heightY / 2, 2,
              "=== Quit Game: ESC === Switch Tabs: TAB === Moving: ARROW KEY === Make Move: SPACE ===");
    return window;
}

GameScreen *newGameScreen(int xMax, int yMax) {
    GameScreen *screen = (GameScreen *) malloc(sizeof(GameScreen));
    screen->xMax = xMax;
    screen->yMax = yMax;
    screen->banner = drawBanner(xMax);
    screen->gameWin = newGameWindow(xMax, yMax, false);
    screen->chatWin = newChatWindow(xMax, yMax, false);
    screen->commandWin = newCommandWindow(xMax, yMax, false);
    screen->labelWin = newLabelWindow(xMax, yMax, false);
    screen->systemWin = newSystemWindow(xMax, yMax, false);
    return screen;
}

void setSystemMessage(GameScreen *screen, char *msg) {
    setNormalTitle(screen);
    delwin(screen->systemWin);
    screen->systemWin = newSystemWindow(screen->xMax, screen->yMax, true);
    int yMax, xMax;
    getmaxyx(screen->systemWin, yMax, xMax);
    mvwprintw(screen->systemWin, yMax / 2, 2, msg);
    wrefresh(screen->systemWin);
}


void setLabelMessage(GameScreen *screen, char label, char opLabel) {
    delwin(screen->labelWin);
    screen->labelWin = newLabelWindow(screen->xMax, screen->yMax, false);
    int yMax, xMax;
    getmaxyx(screen->labelWin, yMax, xMax);
    mvwprintw(screen->labelWin, yMax / 2, 2, "Your Label: %c      Opponent's Label: %c", label, opLabel);
    wrefresh(screen->labelWin);
}

GameBoardInfo drawGameBoard(GameScreen *screen, char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int gameSize) {
    setNormalTitle(screen);
    delwin(screen->gameWin);
    screen->gameWin = newGameWindow(screen->xMax, screen->yMax, true);
    int xMax, yMax;
    keypad(screen->gameWin, true);
    getmaxyx(screen->gameWin, yMax, xMax);
    GameBoardInfo info = newGameBoardInfo(xMax, yMax, gameSize);
    for (int i = 0; i < info.yBoard - 2; i += 3) {
        mvwhline(screen->gameWin, i + (yMax - info.yBoard) / 2 + 2, (xMax - info.xBoard) / 2, ACS_HLINE,
                 info.xBoard);
    }
    for (int i = 0; i < info.xBoard - 4; i += 5) {
        mvwvline(screen->gameWin, (yMax - info.yBoard) / 2, i + (xMax - info.xBoard) / 2 + 4, ACS_VLINE,
                 info.yBoard);
    }
    for (int i = 0; i < gameSize; i++) {
        for (int j = 0; j < gameSize; j++) {
            if (board[i][j] != '-') {
                mvwprintw(screen->gameWin, info.yCur, info.xCur, "%c", board[i][j]);
            }
            info.xCur += 5;
        }
        info.xCur = 2 + (xMax - info.xBoard) / 2;
        info.yCur += 3;
    }
    info.yCur = 1 + (yMax - info.yBoard) / 2; // di chuyen doc
    info.xCur = 2 + (xMax - info.xBoard) / 2; // di chuyen ngang
    wmove(screen->gameWin, info.yCur, info.xCur);

    wrefresh(screen->gameWin);
    return info;
}

void refreshView(GameScreen *screen) {
    wrefresh(screen->banner);
    wrefresh(screen->gameWin);
    wrefresh(screen->chatWin);
    wrefresh(screen->commandWin);
    wrefresh(screen->labelWin);
    wrefresh(screen->systemWin);
}

void setNormalTitle(GameScreen *screen) {
    drawNormalTitle(screen->gameWin, GAME_TITLE);
    drawNormalTitle(screen->chatWin, CHAT_TITLE);
    drawNormalTitle(screen->commandWin, COMMAND_TITLE);
    drawNormalTitle(screen->labelWin, LABEL_TITLE);
    drawNormalTitle(screen->systemWin, SYSTEM_TITLE);
    refreshView(screen);
}


int movingGameWindow(GameBoardInfo *gameBoardInfo, GameScreen *gameScreen, GameInfo *gameInfo) {
    setNormalTitle(gameScreen);
    drawLightTitle(gameScreen->gameWin, GAME_TITLE);
    wmove(gameScreen->gameWin, gameBoardInfo->yCur, gameBoardInfo->xCur);
    int move = wgetch(gameScreen->gameWin);
    switch (move) {
        case KEY_UP:
            if (gameBoardInfo->yCur > 1 + (gameBoardInfo->yMax - gameBoardInfo->yBoard) / 2) {
                gameBoardInfo->yCur -= 3;
            }

            wmove(gameScreen->gameWin, gameBoardInfo->yCur, gameBoardInfo->xCur);
            break;
        case KEY_DOWN:
            if (gameBoardInfo->yCur < (gameBoardInfo->yMax - gameBoardInfo->yBoard) / 2 + gameBoardInfo->yBoard - 1) {
                gameBoardInfo->yCur += 3;
            }

            wmove(gameScreen->gameWin, gameBoardInfo->yCur, gameBoardInfo->xCur);
            break;
        case KEY_LEFT:
            if (gameBoardInfo->xCur > 2 + (gameBoardInfo->xMax - gameBoardInfo->xBoard) / 2) {
                gameBoardInfo->xCur -= 5;
            }

            wmove(gameScreen->gameWin, gameBoardInfo->yCur, gameBoardInfo->xCur);
            break;
        case KEY_RIGHT:
            if (gameBoardInfo->xCur < (gameBoardInfo->xMax - gameBoardInfo->xBoard) / 2 + gameBoardInfo->xBoard - 2) {
                gameBoardInfo->xCur += 5;
            }

            wmove(gameScreen->gameWin, gameBoardInfo->yCur, gameBoardInfo->xCur);
            break;
        case ' ': // send msg
            gameInfo->lastY = (gameBoardInfo->xCur - 2 - (gameBoardInfo->xMax - gameBoardInfo->xBoard) / 2) / 5;
            gameInfo->lastX = (gameBoardInfo->yCur - 1 - (gameBoardInfo->yMax - gameBoardInfo->yBoard) / 2) / 3;
            break;
        default:
            break;
    }
    wrefresh(gameScreen->gameWin);
    return move;
}

void drawChatDialog(GameScreen *gameScreen) {
    ChatMessage *chatMsg = chatMessageHead;
    int x, y;
    getmaxyx(gameScreen->chatWin, y, x);
    int i = 0;
    while (1) {
        if (chatMsg == NULL) {
            return;
        }
        mvwprintw(gameScreen->chatWin, y - 12 + i, 2, chatMsg->message);
        i -= 1;
        if (y - 12 + i < 2) {
            return;
        }
        chatMsg = chatMsg->next;
    }
}

int movingChatWindow(GameScreen *gameScreen, char *msg, int *i) {
    setNormalTitle(gameScreen);
    delwin(gameScreen->chatWin);
    gameScreen->chatWin = newChatWindow(gameScreen->xMax, gameScreen->yMax, true);
    drawChatDialog(gameScreen);
    mvwprintw(gameScreen->chatWin, chatInfo.yCur, chatInfo.xCur, msg);
    int y, x;
    int c = wgetch(gameScreen->chatWin);
    switch (c) {
        case KEY_BACKSPACE:
            getyx(gameScreen->chatWin, y, x);
            if (x > chatInfo.startX || y > chatInfo.startY) {
                msg[--*i] = '\0';
            }
            break;
        case 9:
            break;
        case 10:
            insertChatMessage(0, msg);
            memset(msg, 0, MAX_MSG_LEN);
            *i = 0;
            break;
        default:
            if (*i < MAX_MSG_LEN) // TODO: fix this to take c as printed value
            {
                msg[*i] = c;
                *i = *i + 1;
            }
            break;
    }
    getyx(gameScreen->chatWin, y, x);
    wrefresh(gameScreen->chatWin);
    return c;
}

WINDOW *newDialogWindow(char *title, int *xMax, int *yMax) {
    erase();
    getmaxyx(stdscr, *yMax, *xMax);
    WINDOW *window = newwin(*yMax / 2, *xMax / 2, *yMax / 4, *xMax / 4);
    box(window, 0, 0);
    refresh();
    wrefresh(window);
    keypad(window, true);
    getmaxyx(window, *yMax, *xMax);
    mvwprintw(window, 0, *xMax / 2 - strlen(title) / 2, title);
    return window;
}
