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
#include "view.h"


#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080

#define BUFF_SIZE 1024

void handleContinue(GameScreen *gameScreen, int sock) {
    char *cmd = inputNewBox(" Reminder ", "Do you want to continue? [Y/n]: ");

    if (strcmp(cmd, "Y") != 0 && strcmp(cmd, "y") != 0) {
        send(sock, "0", strlen("0"), 0);
        while (1) {
            WINDOW *sttWin = newStatusWindow(" Message ", "Thank you for playing!", true);
            int c = wgetch(sttWin);
            if (c == 10) {
                break;
            }
        }
        close(sock);
        endwin();
        exit(1);
    } else {
        send(sock, "1", strlen("1"), 0);
        int yMax, xMax;
        getmaxyx(stdscr, yMax, xMax);
        gameScreen = newGameScreen(xMax, yMax);
        refreshView(gameScreen);
    }
}

void gameHandler(char **cmdArr, int client_sock, GameScreen *gameScreen,
                 GameInfo *gameInfo, GameBoardInfo *gameBoardInfo) {
    // get cmd array
    char systemRecvMsg[BUFF_SIZE] = {0};
    char sendMsg[BUFF_SIZE];
    CmdValue cmdValue = getCommand(cmdArr);
    char *msg = malloc(sizeof(char) * MAX_MSG_LEN);
    memset(msg, 0, MAX_MSG_LEN);
    switch (cmdValue.type) {
        case STATUS:
            if (cmdValue.statusCmd.status == SUCCESS_STT) {
                sprintf(systemRecvMsg, "[+] %s", cmdValue.statusCmd.message);

            } else if (cmdValue.statusCmd.status == ERROR_STT) {
                sprintf(systemRecvMsg, "[-] %s", cmdValue.statusCmd.message);
            } else {
                sprintf(systemRecvMsg, "[-] Unrecognized status %s", cmdArr[1]);
            }
            setSystemMessage(gameScreen, systemRecvMsg);
            sleep(2);
            break;
        case MOVING:
            setSystemMessage(gameScreen, "[~] It's Your Turn");
            int r = 0;
            int tab = 0;
            while (1) {
                if (tab == 0) {
                    int key = movingGameWindow(gameBoardInfo, gameScreen, gameInfo);
                    if (key == 27) {
                        break;
                    }
                    if (key == 9) {
                        tab = 1;
                    }
                    if (key == ' ') {
                        sprintf(sendMsg, "%d~%d", gameInfo->lastX, gameInfo->lastY);
                        send(client_sock, sendMsg, strlen(sendMsg), 0);
                        break;
                    }
                } else {
                    int key = movingChatWindow(gameScreen, msg, &r);
                    if (key == 9) {
                        tab = 0;
                    }
                }

            }
            break;
        case UPDATE:
            gameInfo->myLabel = cmdValue.updateCmd.label;
            gameInfo->opLabel = cmdValue.updateCmd.opLabel;
            gameInfo->gameSize = cmdValue.updateCmd.boardSize;
            for (int i = 0; i < cmdValue.updateCmd.boardSize; i++) {
                for (int j = 0; j < cmdValue.updateCmd.boardSize; j++) {
                    gameInfo->board[i][j] = cmdValue.updateCmd.board[i][j];
                }
            }
            setLabelMessage(gameScreen, gameInfo->myLabel, gameInfo->opLabel);
            setSystemMessage(gameScreen, "[~] Update Board...");

            *gameBoardInfo = drawGameBoard(gameScreen, gameInfo->board, gameInfo->gameSize);
            send(client_sock, "1", strlen("1"), 0); // send success update table
            break;
        case DONE:
            if (cmdValue.doneCmd.is_winner) {
                setSystemMessage(gameScreen, "[+] Congratulation! You are the winner!!");
            } else {
                setSystemMessage(gameScreen, "[+] You are the loser!!");
            }
            sleep(2);
            //TODO: send yes or no
            handleContinue(gameScreen, client_sock);
            break;
        default:
            break;
    }
}


void createGameHandler(int client_sock, GameInfo *gameInfo) {
    char *name = NULL;
    char *size = NULL;
    char buff[BUFF_SIZE + 1];
    int bytes_received;
    int gameSize = 0;
    name = inputNewBox(" Create New Game ", "Enter Your Name:");
    size = inputNewBox(" Create New Game ", "Enter N Board Size (NxN):");
    gameSize = strtol(size, NULL, 10);
    while (gameSize < 3 || gameSize > 16) {
        size = inputNewBox(" Create New Game ", "3<=Size<=16.Enter Again:");
        gameSize = strtol(size, NULL, 10);
    }
    sprintf(buff, "create~%s~%d", name, gameSize);
    send(client_sock, buff, strlen(buff), 0);
    memset(buff, 0, BUFF_SIZE);
    bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
    if (bytes_received <= 0) {
        while (1) {
            WINDOW *sttWin = newStatusWindow(" Game Loading... ", "Connection closed or error", true);
            int c = wgetch(sttWin);
            if (c == 10) {
                break;
            }

        }
        close(client_sock);
        endwin();
        exit(1);
    }
    char **cmdArr = parseCmd(buff);
    CmdValue cmdValue = getCommand(cmdArr);
    if (cmdValue.statusCmd.status == ERROR_STT) {
        while (1) {
            WINDOW *sttWin = newStatusWindow(" Game Loading... ", cmdValue.statusCmd.message, true);
            int c = wgetch(sttWin);
            if (c == 10) {
                break;
            }

        }
        close(client_sock);
        endwin();
        exit(1);
    }
    char successMsg[MAXCMDLENGTH] = {0};
    sprintf(successMsg, "%s.Room Code: %d", cmdValue.statusCmd.message, cmdValue.statusCmd.gameCode);
    WINDOW *sttWin = newStatusWindow(" Game Loading... ", successMsg, false);
    gameInfo->gameCode = cmdValue.statusCmd.gameCode;

    bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
    if (bytes_received <= 0) {
        while (1) {
            sttWin = newStatusWindow(" Game Loading... ", "Connection closed or error", true);
            int c = wgetch(sttWin);
            if (c == 10) {
                break;
            }
        }
    }
    sttWin = newStatusWindow(" Game Loading... ", cmdValue.statusCmd.message, true);
    destroyCmd(cmdArr);
}

void joinGameHandler(int client_sock, GameInfo *gameInfo) {
    char *name = NULL;
    char *code = NULL;
    int gameCode = 0;
    char buff[BUFF_SIZE + 1];
    int bytes_received;

    code = inputNewBox(" Join Existing Game ", "Enter Exist Room Code:");
    gameCode = strtol(code, NULL, 10);
    while (gameCode <= 0) {
        code = inputNewBox(" Join Existing Game ", "GameCode must be number > 0. Try Again:");
        gameCode = strtol(code, NULL, 10);
    }
    name = inputNewBox(" Join Existing Game ", "Enter Your Name:");
    sprintf(buff, "join~%s~%d", name, gameCode);
    send(client_sock, buff, strlen(buff), 0);
    memset(buff, 0, BUFF_SIZE);
    bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
    if (bytes_received <= 0) {
        while (1) {
            WINDOW *sttWin = newStatusWindow(" Game Loading... ", "Connection closed or error", true);
            int c = wgetch(sttWin);
            if (c == 10) {
                break;
            }

        }
        close(client_sock);
        endwin();
        exit(1);
    }

    char **cmdArr = parseCmd(buff);
    CmdValue cmdValue = getCommand(cmdArr);
    if (cmdValue.statusCmd.status == ERROR_STT) {
        while (1) {
            WINDOW *sttWin = newStatusWindow(" Game Loading... ", cmdValue.statusCmd.message, true);
            int c = wgetch(sttWin);
            if (c == 10) {
                break;
            }

        }
        close(client_sock);
        endwin();
        exit(1);
    }
    WINDOW *sttWin = newStatusWindow(" Game Loading... ", cmdValue.statusCmd.message, false);
    gameInfo->gameCode = gameCode;

    bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
    if (bytes_received <= 0) {
        while (1) {
            sttWin = newStatusWindow(" Game Loading... ", "Connection closed or error", true);
            int c = wgetch(sttWin);
            if (c == 10) {
                break;
            }
        }
    }
    sttWin = newStatusWindow(" Game Loading... ", cmdValue.statusCmd.message, true);
    destroyCmd(cmdArr);
}

int main() {
    int client_sock;
    char buff[BUFF_SIZE + 1];
    struct sockaddr_in server_addr; /* server's address information */
    int bytes_received;
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
    // Init Game Information
    GameInfo *gameInfo = (GameInfo *) malloc(sizeof(GameInfo));
    // init game screen
    initscr();
    cbreak();
    curs_set(0);
    noecho();
    int yMax = 0, xMax = 0;
    getmaxyx(stdscr, yMax, xMax);
    int choice = 0;
    newMenuWindow(&choice, &yMax, &xMax);


    switch (choice) {
        case 0:
            createGameHandler(client_sock, gameInfo);
            break;
        case 1:
            joinGameHandler(client_sock, gameInfo);
            break;
        case 2:
            break;
        default:
            close(client_sock);
            endwin();
            return 0;
    }
    erase();
    getmaxyx(stdscr, yMax, xMax);
    GameScreen *gameScreen = newGameScreen(xMax, yMax);
    refresh();
    refreshView(gameScreen);

    GameBoardInfo *gameBoardInfo = (GameBoardInfo *) malloc(sizeof(GameBoardInfo));
    while ((bytes_received = recv(client_sock, buff, BUFF_SIZE, 0)) > 0) {
        buff[bytes_received] = '\0';
        char **cmdArr = parseCmd(buff);
        gameHandler(cmdArr, client_sock, gameScreen, gameInfo, gameBoardInfo);
        destroyCmd(cmdArr);
        memset(buff, 0, BUFF_SIZE);
        setSystemMessage(gameScreen, "[~] Waiting for Opponent...");
    }

    close(client_sock);
    endwin();
    return 0;
}