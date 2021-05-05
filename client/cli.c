#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "msg_parser.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8081
#define BUFF_SIZE 1024
void handleContinue(int sock);

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

    printf("\n\n-----------------------------\n");
    printf("TIC TAC TOE CLI\n");
    printf("-----------------------------\n");

    //Step 4: Communicate with server

    int c = 0;

    printf("1. CREATE NEW GAME\n");
    printf("2. JOIN GAME\n");
    printf("3. EXIT\n");
    printf("Your choice (1-3):");
    scanf("%d", &c);
    getchar();
    char name[BUFF_SIZE];
    char roomCode[BUFF_SIZE];
    int roomCodeInt = 0;
    switch (c) {
        case 1:
            memset(buff, '\0', BUFF_SIZE);
            strcpy(buff, "create");
            strcat(buff, "~");
            printf("\nInsert name:");
            memset(name, '\0', BUFF_SIZE);
            fgets(name, BUFF_SIZE, stdin);
            name[strcspn(name, "\n")] = '\0';
            strcat(buff, name);
            strcat(buff, "~3");
            break;
        case 2:
            memset(buff, '\0', BUFF_SIZE);
            strcpy(buff, "join");
            strcat(buff, "~");
            printf("\nInsert name:");

            memset(name, '\0', BUFF_SIZE);
            fgets(name, BUFF_SIZE, stdin);
            name[strcspn(name, "\n")] = '\0';
            strcat(buff, name);
            strcat(buff, "~");

            memset(roomCode, '\0', BUFF_SIZE);
            printf("\nInsert room code:");
            scanf("%d", &roomCodeInt);
            sprintf(roomCode, "%d", roomCodeInt);
            strcat(buff, roomCode);
            break;
        default:
            break;
    }
    //Step 4: Communicate with server

    msg_len = strlen(buff);

    bytes_sent = send(client_sock, buff, msg_len, 0);
    if (bytes_sent < 0)
        perror("\nError: ");
    printf("[~] Wait for server to response...\n");
    memset(buff, '\0', BUFF_SIZE);
    while ((bytes_received = recv(client_sock, buff, BUFF_SIZE, 0)) > 0) {

        buff[bytes_received] = '\0';
        printf("Reply from server: %s\n", buff);
        // get cmd array
        char **cmdArr = parseCmd(buff);
        CmdValue cmdValue = getCommand(cmdArr);
        char msg[BUFF_SIZE];
        switch (cmdValue.type) {
            case STATUS:
                if (cmdValue.statusCmd.status == SUCCESS_STT) {
                    printf("[+] Status: Success - Message: %s\n", cmdValue.statusCmd.message);
                } else if (cmdValue.statusCmd.status == ERROR_STT) {
                    printf("[-] Status: Failed - Message: %s\n", cmdValue.statusCmd.message);
                } else {
                    printf("[-] Unrecognized status from server %s\n", cmdArr[1]);
                }
                break;
            case MOVING:
                sprintf(msg, "%d~%d", cmdValue.movingCmd.x, cmdValue.movingCmd.y);
                send(client_sock, msg, strlen(msg), 0);
                break;
            case UPDATE:
                printf("-- Your Label:\t\t%c --\n", cmdValue.updateCmd.label);
                printf("-- Opponent Label:\t%c --\n", cmdValue.updateCmd.opLabel);
                for (int i = 0; i < cmdValue.updateCmd.boardSize; i++) {
                    for (int j = 0; j < cmdValue.updateCmd.boardSize; j++) {
                        printf("%c  ", cmdValue.updateCmd.board[i][j]);
                    }
                    printf("\n");
                }
                send(client_sock, "1", strlen("1"), 0); // send success update table
                break;
            case DONE:
                if(cmdValue.doneCmd.is_winner)
                    printf("[+] Congratulation! You are the winner!!\n");
                else
                    printf("[+] You are the loser!! The winner is %s\n", cmdValue.doneCmd.winner);
                //TODO: send yes or no
                handleContinue(client_sock);
                break;
            default:
                break;
        }
        printf("[~] Wait for server to response...\n");
        destroyCmd(cmdArr);
        memset(buff, 0, BUFF_SIZE);
    }
    if (bytes_received < 0)
        perror("\nError: ");
    else if (bytes_received == 0)
        printf("[-] Connection closed.\n");
    close(client_sock);
    return 0;
}

void handleContinue(int sock){
    char cmd;
    printf("Do you want to continue [y/n]: ");
    scanf(" %c", &cmd);
    if(cmd != 'y' && cmd != 'Y'){
        send(sock, "0", strlen("0"), 0);
        printf("Thank you for playing\n");
    } else {
        send(sock, "1", strlen("1"), 0);   
    }
}