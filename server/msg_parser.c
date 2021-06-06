//
// Created by monstertau on 25/04/2021.
//
#include <malloc.h>
#include <string.h>
#include "msg_parser.h"
#include <stdlib.h>

char **parseCmd(char *getStr) {
    char **arr = (char **) malloc(MAXCMDLENGTH * sizeof(char *));

    for (int i = 0; i < MAXCMDLENGTH; i++) {
        arr[i] = (char *) malloc(MAXCMDLENGTH * sizeof(char));
        memset(arr[i], 0, MAXCMDLENGTH);
    }
    char tmp[MAXCMDLENGTH];
    memset(tmp, 0, MAXCMDLENGTH);
    int j = 0, r = 0;
    for (int i = 0; i < strlen(getStr); i++) {
        if (getStr[i] == '~') {
            strcpy(arr[r++], tmp);
            j = 0;
            memset(tmp, 0, MAXCMDLENGTH);
            tmp[j++] = getStr[++i];
            continue;
        }
        tmp[j++] = getStr[i];
    }
    strcpy(arr[r], tmp);
    return arr;
}

void destroyCmd(char **cmd) {
    free(*cmd);
    free(cmd);
}

CmdValue getCommand(char **cmdArr) {
    CmdValue cmdValue;
    if (strcmp(cmdArr[0], "create") == 0) {
        cmdValue.type = CREATE;
        cmdValue.createCmd = newCreateCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "join") == 0) {
        cmdValue.type = JOIN;
        cmdValue.joinCmd = newJoinCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "watch") == 0) {
        cmdValue.type = WATCH;
        cmdValue.watchCmd = newWatchCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "info") == 0) {
        cmdValue.type = INFO;
    } else if (strcmp(cmdArr[0], "moving") == 0) {
        cmdValue.type = MOVING;
        cmdValue.moveCmd = newMoveCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "chat") == 0) {
        cmdValue.type = CHAT;
        cmdValue.chatCmd = newChatCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "continue") == 0) {
        cmdValue.type = CONTINUE;
    } else {
        cmdValue.type = UNRECOGNIZED;
    }
    return cmdValue;
}

CreateCmd newCreateCommand(char **cmdArr) {
    char name[50];
    strcpy(name, cmdArr[1]); // name in cmd index 1
    int size = strtol(cmdArr[2], NULL, 10);
    CreateCmd createCmd;
    strcpy(createCmd.name, name);
    createCmd.boardSize = size;
    return createCmd;
}

JoinCmd newJoinCommand(char **cmdArr) {
    char name[50];
    strcpy(name, cmdArr[1]); // name in cmd index 1
    int code = strtol(cmdArr[2], NULL, 10); // roomcode in cmd index 2
    JoinCmd joinCmd;
    strcpy(joinCmd.name, name);
    joinCmd.roomCode = code;
    return joinCmd;
}

WatchCmd newWatchCommand(char **cmdArr) {
    char name[50];
    strcpy(name, cmdArr[1]);
    int code = strtol(cmdArr[2], NULL, 10);
    WatchCmd watchCmd;
    strcpy(watchCmd.name, name);
    watchCmd.roomCode = code;
    return watchCmd;
}

MoveCmd newMoveCommand(char **cmdArr) {
    MoveCmd moveCmd;
    moveCmd.x = strtol(cmdArr[1], NULL, 10);
    moveCmd.y = strtol(cmdArr[2], NULL, 10);
    return moveCmd;
}

ChatCmd newChatCommand(char **cmdArr) {
    ChatCmd chatCmd;
    memset(chatCmd.chatMsg, 0, MAXCMDLENGTH);
    strcpy(chatCmd.chatMsg, cmdArr[1]);
    return chatCmd;
}