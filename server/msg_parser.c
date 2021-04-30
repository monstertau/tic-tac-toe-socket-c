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

        WatchCmd watchCmd;
        cmdValue.watchCmd = watchCmd;
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
