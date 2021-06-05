//
// Created by monstertau on 25/04/2021.
//
#include <stdbool.h>

#ifndef SERVER_MSG_PARSER_H
#define SERVER_MSG_PARSER_H
#define MAXCMDLENGTH 512
#define MAX_BOARD_SIZE 16
#define MAXMSG 1024
#endif //SERVER_MSG_PARSER_H
typedef enum {
    STATUS,
    MOVING,
    UPDATE,
    DONE,
    LIST,
    CHAT,
    UNRECOGNIZED
} Command;
typedef enum {
    SUCCESS_STT,
    ERROR_STT,
    UNRECOGNIZED_STT
} Status;
typedef struct StatusCmd_ {
    char message[512];
    int gameCode;
    Status status;
} StatusCmd;

typedef struct MovingCmd_ {
    int x;
    int y;
} MovingCmd;

typedef struct UpdateCmd_ {
    char label;
    char opLabel;
    int boardSize;
    char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
} UpdateCmd;

typedef struct DoneCmd_ {
    char winner[MAXMSG];
    bool is_winner;
} DoneCmd;

typedef struct InfoCmd_ {
    char *roomID;
    char *size;
    char *playerInfo;
    char *watcherInfo;
    struct InfoCmd_ *next;
} InfoCmd;
typedef struct ChatRecv_ {
    char *message;
    struct ChatRecv_ *next;
} ChatRecv;
typedef struct ChatCmd_ {
    ChatRecv *chatRecv;
} ChatCmd;

typedef struct CmdValue_ {
    Command type;
    union {
        StatusCmd statusCmd;
        MovingCmd movingCmd;
        UpdateCmd updateCmd;
        DoneCmd doneCmd;
        InfoCmd *infoCmd;
        ChatCmd chatCmd;
    };
} CmdValue;

char **parseCmd(char *getStr);

void destroyCmd(char **cmd);

CmdValue getCommand(char **cmdArr);

StatusCmd newStatusCommand(char **cmdArr);

UpdateCmd newUpdateCommand(char **cmdArr);

MovingCmd newMovingCommand(char **cmdArr);

DoneCmd newDoneCommand(char **cmdArr);

InfoCmd *newListRoomCommand(char **cmdArr);

ChatCmd newChatCommand(char **cmdArr);