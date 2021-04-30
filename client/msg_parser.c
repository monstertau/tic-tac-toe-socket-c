#include <string.h>
#include "msg_parser.h"
#include <stdlib.h>
#include <stdio.h>

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
    if (strcmp(cmdArr[0], "status") == 0) {
        cmdValue.type = STATUS;
        cmdValue.statusCmd = newStatusCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "moving") == 0) {
        cmdValue.type = MOVING;
        cmdValue.movingCmd = newMovingCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "update") == 0) {
        cmdValue.type = UPDATE;
        cmdValue.updateCmd = newUpdateCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "done") == 0) {
        cmdValue.type = DONE;
        cmdValue.doneCmd = newDoneCommand(cmdArr);
    } else {
        cmdValue.type = UNRECOGNIZED;
    }
    return cmdValue;
}

StatusCmd newStatusCommand(char **cmdArr) {
    StatusCmd statusCmd;
    if (strcmp(cmdArr[1], "1") == 0) {
        statusCmd.status = SUCCESS_STT;
    } else if (strcmp(cmdArr[1], "0") == 0) {
        statusCmd.status = ERROR_STT;
    } else {
        statusCmd.status = UNRECOGNIZED_STT;
        return statusCmd;
    }
    strcpy(statusCmd.message, cmdArr[2]);
    return statusCmd;
}

UpdateCmd newUpdateCommand(char **cmdArr) {
    //TODO
    UpdateCmd updateCmd;
    updateCmd.label = *cmdArr[1];
    updateCmd.opLabel = *cmdArr[2];
    updateCmd.boardSize = strtol(cmdArr[3], NULL, 10);
    int r = 0;
    for (int i = 0; i < updateCmd.boardSize; i++) {
        for (int j = 0; j < updateCmd.boardSize; j++) {
            updateCmd.board[i][j] = cmdArr[4][r++];
        }
    }
    return updateCmd;
}

MovingCmd newMovingCommand(char **cmdArr) {
    //TODO
    MovingCmd movingCmd;
    printf("Enter x location:");
    scanf("%d", &movingCmd.x);
    printf("Enter y location:");
    scanf("%d", &movingCmd.y);
    return movingCmd;
}

DoneCmd newDoneCommand(char **cmdArr) {
    //TODO
    DoneCmd doneCmd;
    return doneCmd;
}