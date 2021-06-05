//
// Created by monstertau on 25/04/2021.
//

#ifndef SERVER_MSG_PARSER_H
#define SERVER_MSG_PARSER_H
#define MAXCMDLENGTH 512
#endif //SERVER_MSG_PARSER_H

typedef enum {
    CREATE,
    JOIN,
    WATCH,
    INFO,
    MOVING,
    CHAT,
    UNRECOGNIZED
} Command;

typedef struct CreateCmd_ {
    char name[50];
    int boardSize;

} CreateCmd;

typedef struct JoinCmd_ {
    char name[50];
    int roomCode;
} JoinCmd;

typedef struct WatchCmd_ {
    char name[50];
    int roomCode;
} WatchCmd;

typedef struct InfoCmd_ {
} InfoCmd;

typedef struct MoveCmd_ {
    int x;
    int y;
} MoveCmd;

typedef struct ChatCmd_ {
    char chatMsg[MAXCMDLENGTH];
} ChatCmd;

typedef struct CmdValue_ {
    Command type;
    union {
        CreateCmd createCmd;
        JoinCmd joinCmd;
        WatchCmd watchCmd;
        MoveCmd moveCmd;
        InfoCmd infoCmd;
        ChatCmd chatCmd;
    };
} CmdValue;

char **parseCmd(char *getStr);

void destroyCmd(char **cmd);

CmdValue getCommand(char **cmdArr);

CreateCmd newCreateCommand(char **cmdArr);

JoinCmd newJoinCommand(char **cmdArr);

WatchCmd newWatchCommand(char **cmdArr);

MoveCmd newMoveCommand(char **cmdArr);

ChatCmd newChatCommand(char **cmdArr);