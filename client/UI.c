#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include "msg_parser.h"
#include "ncurses.h"

#define MAX_MENU 4

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8081

#define BUFF_SIZE 1024
int n;

void handleContinue(int sock) {
    char cmd;
    printf("Do you want to continue [y/n]: ");
    scanf(" %c", &cmd);
    if (cmd != 'y' && cmd != 'Y') {
        send(sock, "0", strlen("0"), 0);
        printf("Thank you for playing\n");
    } else {
        send(sock, "1", strlen("1"), 0);
    }
}

WINDOW *drawSubWindow(char *title, char *defaultContent, int widthX, int heightY, int startX, int startY) {
    WINDOW *window = newwin(heightY, widthX, startY, startX);
    box(window, 0, 0);
    mvwprintw(window, 0, 2, title);
    mvwprintw(window, heightY / 2, 2, defaultContent);
    wrefresh(window);
    return window;
}

//
void drawBanner(int xMax, int yMax) {
    WINDOW *window = newwin(5, xMax, 0, 0);
    mvwprintw(window, 0, xMax / 4, "   __  _         __                __           ");
    mvwprintw(window, 1, xMax / 4, "  / /_(_)____   / /_____ ______   / /_____  ___ ");
    mvwprintw(window, 2, xMax / 4, " / __/ / ___/  / __/ __ `/ ___/  / __/ __ \\/ _ \\");
    mvwprintw(window, 3, xMax / 4, "/ /_/ / /__   / /_/ /_/ / /__   / /_/ /_/ /  __/");
    mvwprintw(window, 4, xMax / 4, "\\__/_/\\___/   \\__/\\__,_/\\___/   \\__/\\____/\\___/ ");
    wrefresh(window);
}

void drawGame(char *buff, int client_sock, WINDOW *gameWin, WINDOW *systemMsgWin, WINDOW *turnWin, int xMax, int yMax) {
    // get cmd array
    char systemRecvMsg[BUFF_SIZE] = {0};
    char sendMsg[BUFF_SIZE];
    char myLabel = 0, opLabel = 0;
    char labelRecvMsg[BUFF_SIZE] = "Your Label:       Opponent's Label: ";
    int gameX, gameY;

    char **cmdArr = parseCmd(buff);
    CmdValue cmdValue = getCommand(cmdArr);
    switch (cmdValue.type) {
        case STATUS:
            if (cmdValue.statusCmd.status == SUCCESS_STT) {
                sprintf(systemRecvMsg, "[+] %s", cmdValue.statusCmd.message);

            } else if (cmdValue.statusCmd.status == ERROR_STT) {
                sprintf(systemRecvMsg, "[-] %s", cmdValue.statusCmd.message);
            } else {
                sprintf(systemRecvMsg, "[-] Unrecognized status %s", cmdArr[1]);
            }
            systemMsgWin = drawSubWindow(" System Message ",
                                         systemRecvMsg, xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0);
            break;
        case MOVING:
            sprintf(systemRecvMsg, "[~] It's Your Turn!");
            systemMsgWin = drawSubWindow(" System Message ",
                                         systemRecvMsg, xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0);
            int move;
            keypad(gameWin, true);

            int yBoard = n * 2 + n -
                         1;
            int xBoard = n * 4 + n - 1;
            getmaxyx(gameWin, yMax, xMax);
            int yCur = 1 + (yMax - yBoard) / 2; // di chuyen doc
            int xCur = 2 + (xMax - xBoard) / 2; // di chuyen ngang

            wmove(gameWin, yCur, xCur);
            wrefresh(gameWin);

            while (1) {
                move = wgetch(gameWin);
                switch (move) {
                    case KEY_UP:
                        if (yCur > 1 + (yMax - yBoard) / 2) {
                            yCur -= 3;
                        }

                        wmove(gameWin, yCur, xCur);
                        break;
                    case KEY_DOWN:
                        if (yCur < (yMax - yBoard) / 2 + yBoard - 1) {
                            yCur += 3;
                        }

                        wmove(gameWin, yCur, xCur);
                        break;
                    case KEY_LEFT:
                        if (xCur > 2 + (xMax - xBoard) / 2) {
                            xCur -= 5;
                        }

                        wmove(gameWin, yCur, xCur);
                        break;
                    case KEY_RIGHT:
                        if (xCur < (xMax - xBoard) / 2 + xBoard - 2) {
                            xCur += 5;
                        }

                        wmove(gameWin, yCur, xCur);
                        break;
                    case ' ': // send msg
                        cmdValue.movingCmd.y = (xCur - 2 - (xMax - xBoard) / 2) / 5;
                        cmdValue.movingCmd.x = (yCur - 1 - (yMax - yBoard) / 2) / 3;
                        break;
                    default:
                        break;
                }
                if (move == ' ') {
                    break;
                }
                wrefresh(gameWin);
            }
            sprintf(sendMsg, "%d~%d", cmdValue.movingCmd.x, cmdValue.movingCmd.y);
//            wmove(gameWin, 1, 0);
//            wprintw(gameWin, sendMsg);
//            wrefresh(gameWin);
            send(client_sock, sendMsg, strlen(sendMsg), 0);
            break;
        case UPDATE:
            sprintf(systemRecvMsg, "[~] Update Board...");
            systemMsgWin = drawSubWindow(" System Message ",
                                         systemRecvMsg, xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0);
            sprintf(labelRecvMsg, "Your Label: %c      Opponent's Label: %c", cmdValue.updateCmd.label,
                    cmdValue.updateCmd.opLabel);
            turnWin = drawSubWindow(" Labels ",
                                    labelRecvMsg, xMax * 1 / 4, 5,
                                    xMax * 3 / 4 + 1, yMax - 5);
            getmaxyx(gameWin, yMax, xMax);
            n = cmdValue.updateCmd.boardSize;
            int yBoardUpdate = cmdValue.updateCmd.boardSize * 2 + cmdValue.updateCmd.boardSize -
                               1; // TODO: fix board size as max screen
            int xBoardUpdate = cmdValue.updateCmd.boardSize * 4 + cmdValue.updateCmd.boardSize - 1;
            int yCurUpdate = 1 + (yMax - yBoardUpdate) / 2; // di chuyen doc
            int xCurUpdate = 2 + (xMax - xBoardUpdate) / 2; // di chuyen ngang
//            wprintw(gameWin,"%d - %d",yMax,xMax);
            for (int i = 0; i < yBoardUpdate - 2; i += 3) {
                mvwhline(gameWin, i + (yMax - yBoardUpdate) / 2 + 2, (xMax - xBoardUpdate) / 2, ACS_HLINE,
                         xBoardUpdate);
            }
            for (int i = 0; i < xBoardUpdate - 4; i += 5) {
                mvwvline(gameWin, (yMax - yBoardUpdate) / 2, i + (xMax - xBoardUpdate) / 2 + 4, ACS_VLINE,
                         yBoardUpdate);
            }
            for (int i = 0; i < cmdValue.updateCmd.boardSize; i++) {
                for (int j = 0; j < cmdValue.updateCmd.boardSize; j++) {
                    if (cmdValue.updateCmd.board[i][j] != '-') {
                        mvwprintw(gameWin, yCurUpdate, xCurUpdate, "%c", cmdValue.updateCmd.board[i][j]);
                    }
                    xCurUpdate += 5;
                }
                xCurUpdate = 2 + (xMax - xBoardUpdate) / 2;
                yCurUpdate += 3;
            }
            wrefresh(gameWin);
            send(client_sock, "1", strlen("1"), 0); // send success update table
            break;
        case DONE:

            if (cmdValue.doneCmd.is_winner) {
                sprintf(systemRecvMsg, "[+] Congratulation! You are the winner!!");
                systemMsgWin = drawSubWindow(" System Message ",
                                             systemRecvMsg, xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0);
            } else {
                sprintf(systemRecvMsg, "[+] You are the loser!! The winner is %s", cmdValue.doneCmd.winner);
                systemMsgWin = drawSubWindow(" System Message ",
                                             systemRecvMsg, xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0);
            }
            //TODO: send yes or no
            handleContinue(client_sock);
            break;
        default:
            break;
    }
    destroyCmd(cmdArr);
    memset(buff, 0, BUFF_SIZE);
}

WINDOW *drawDialogWindow(char *title, int *xMax, int *yMax) {
    erase();
    getmaxyx(stdscr, *yMax, *xMax);
    WINDOW *window = newwin(*yMax / 2, *xMax / 2, *yMax / 4, *xMax / 4);
    box(window, 0, 0);
    refresh();
    wrefresh(window);
    keypad(window, true);
    getmaxyx(window, *yMax, *xMax);
    wattron(window, A_REVERSE);
    mvwprintw(window, 0, *xMax / 2 - strlen(title) / 2, title);
    wattroff(window, A_REVERSE);
    return window;
}

char *inputNewBox(char *title, char *label) {
    char *name = malloc(sizeof(char) * 40);
    int yMax = 0, xMax = 0;


    erase();
    noecho();
    curs_set(1);

    int yTitle, xTitle;
    WINDOW *gameWin = drawDialogWindow(title, &xMax, &yMax);
    mvwprintw(gameWin, 5, xMax / 2 - strlen(label), label);

    getyx(gameWin, yTitle, xTitle);
    wprintw(gameWin, "%s", name);
    int c;
    int i = 0;
    while ((c = wgetch(gameWin)) != 10) {
        switch (c) {
            case KEY_BACKSPACE:
                getyx(gameWin, yMax, xMax);
                if (xMax > xTitle || yMax > yTitle) {
                    name[--i] = '\0';
                }
                break;
            default:
                if (i < 39) // TODO: fix this to take c as printed value
                {
                    name[i++] = c;
                }
                break;
        }
        werase(gameWin);
        gameWin = drawDialogWindow(title, &xMax, &yMax);
        mvwprintw(gameWin, 5, xMax / 2 - strlen(label), label);
        wprintw(gameWin, "%s", name);
        wrefresh(gameWin);
    }
    erase();
    name[i] = '\0';
    return name;
}


int main() {
    int client_sock;
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received, total_bytes;
    total_bytes = 0;
    //Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    //Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    //Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
        printf("\nError!Can not connect to sever! Client exit immediately! ");
        return 0;
    }

    initscr();
    cbreak();
    curs_set(0);
    noecho();
    int yMax = 0, xMax = 0;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW *menuWin = drawDialogWindow(" Tic Tac Toe Online Game ", &xMax, &yMax);

    char choices[MAX_MENU][40] = {"Start A New Game", "Join An Existing Game", "Game's Rule", "Exit"};
    int key;
    int choice = 0;
    while (1) {
        for (int i = 0; i < MAX_MENU; i++) {
            if (choice == i) {
                wattron(menuWin, A_REVERSE);
            }
            mvwprintw(menuWin, i + 5, xMax / 2 - strlen(choices[i]) / 2, choices[i]);
            wattroff(menuWin, A_REVERSE);
        }
        key = wgetch(menuWin);
        switch (key) {
            case KEY_UP:
                choice--;
                if (choice == -1)
                    choice = 0;
                break;
            case KEY_DOWN:
                choice++;
                if (choice > MAX_MENU - 1)
                    choice = MAX_MENU - 1;
                break;
            default:
                break;
        }
        if (key == 10) // Enter key stroke
            break;
    }
    char *name = NULL;
    char *code = NULL;
    char *size = NULL;
    int gameCode = 0;
    int gameSize = 0;
    switch (choice) {
        case 0:
            name = inputNewBox(" Create New Game ", "Enter Your Name:");
            size = inputNewBox(" Create New Game ", "Enter N Board Size (NxN):");
            gameSize = strtol(size, NULL, 10);
            while (gameSize < 3 || gameSize > 16) {
                size = inputNewBox(" Create New Game ", "3<=Size<=16.Enter Again:");
                gameSize = strtol(size, NULL, 10);
            }
            sprintf(buff, "create~%s~%d", name, gameSize);
//            drawGame(gameSize);
            break;
        case 1:
            code = inputNewBox(" Join Existing Game ", "Enter Exist Room Code:");
            gameCode = strtol(code, NULL, 10);
            while (gameCode <= 0) {
                code = inputNewBox(" Join Existing Game ", "GameCode must be number > 0. Try Again:");
                gameCode = strtol(code, NULL, 10);
            }
            name = inputNewBox(" Join Existing Game ", "Enter Your Name:");
            sprintf(buff, "join~%s~%d", name, gameCode);
            break;
        case 2:
            break;
        default:
            return 0;
    }
    msg_len = strlen(buff);
    bytes_sent = send(client_sock, buff, msg_len, 0);
    if (bytes_sent < 0)
        perror("\nError: ");
    erase();
    refresh();
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *win = drawSubWindow(" Game Loading... ", "Waiting for server to response", xMax / 2, yMax / 2, xMax / 4,
                                yMax / 4);
    memset(buff, '\0', BUFF_SIZE);

    erase();
    curs_set(1);
    noecho();
    getmaxyx(stdscr, yMax, xMax);
    refresh();
    drawBanner(xMax, yMax);
    WINDOW *gameWin = drawSubWindow(" Game Board ",
                                    "", xMax * 3 / 4, yMax - 10, 0, 5);
    WINDOW *chatWin = drawSubWindow(" Chat ",
                                    "", xMax * 1 / 4, yMax - 10, xMax * 3 / 4 + 1, 5);
    WINDOW *turnWin = drawSubWindow(" Labels ",
                                    "Your Label:       Opponent's Label: ", xMax * 1 / 4, 5,
                                    xMax * 3 / 4 + 1, yMax - 5);
    WINDOW *commandWin = drawSubWindow(" List Commands ",
                                       "=== Quit Game: ESC === Switch Tabs: TAB === Moving: ARROW KEY === Make Move: SPACE ===",
                                       xMax * 3 / 4, 5, 0, yMax - 5);
    WINDOW *systemMsgWin = drawSubWindow(" System Message ",
                                         "", xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0);
    while ((bytes_received = recv(client_sock, buff, BUFF_SIZE, 0)) > 0) {

        buff[bytes_received] = '\0';
        drawGame(buff, client_sock, gameWin, systemMsgWin, turnWin, xMax, yMax);
        systemMsgWin = drawSubWindow(" System Message ",
                                     "[+] Waiting For Opponent...", xMax * 1 / 4, 5, xMax * 3 / 4 + 1, 0);
    }


    close(client_sock);
    endwin();
    return 0;
}