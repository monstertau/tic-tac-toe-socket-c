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
    } else if (strcmp(cmdArr[0], "list") == 0) {
        cmdValue.type = LIST;
        cmdValue.infoCmd = newListRoomCommand(cmdArr);
    } else if (strcmp(cmdArr[0], "chat") == 0) {
        cmdValue.type = CHAT;
        cmdValue.chatCmd = newChatCommand(cmdArr);
    } else {
        cmdValue.type = UNRECOGNIZED;
    }
    return cmdValue;
}

InfoCmd *newListRoomCommand(char **cmdArr) {
    InfoCmd *infoCmdHead = NULL;
    int i = 1;
    while (strlen(cmdArr[i]) != 0) {
        InfoCmd *newInfoCmd = (InfoCmd *) malloc(sizeof(InfoCmd));
        newInfoCmd->roomID = cmdArr[i++];
        newInfoCmd->size = cmdArr[i++];
        newInfoCmd->playerInfo = cmdArr[i++];
        newInfoCmd->watcherInfo = cmdArr[i++];
        newInfoCmd->next = infoCmdHead;
        infoCmdHead = newInfoCmd;
    }
    return infoCmdHead;
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
    if (strlen(cmdArr[3]) != 0) {
        statusCmd.gameCode = strtol(cmdArr[3], NULL, 10);
    }
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
    return movingCmd;
}

DoneCmd newDoneCommand(char **cmdArr) {
    //TODO
    DoneCmd doneCmd;
    doneCmd.is_winner = (strcmp(cmdArr[1], "1") == 0);
    strcpy(doneCmd.winner, cmdArr[2]);
    return doneCmd;
}

ChatCmd newChatCommand(char **cmdArr) {
    ChatCmd chatCmd;
    chatCmd.chatRecv = NULL;
    int i = 1;
    while (strlen(cmdArr[i]) > 0) {
        ChatRecv *chatRecv = (ChatRecv *) malloc(sizeof(ChatRecv));
        chatRecv->message = cmdArr[i];
        chatRecv->next = chatCmd.chatRecv;
        chatCmd.chatRecv = chatRecv;
        i++;
    }
    return chatCmd;
}