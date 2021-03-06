//
// Created by monstertau on 12/04/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include "game_manager.h"
#include "msg_parser.h"

#define PORT 8080
#define BACKLOG 36
#define BUFF_SIZE 1024
typedef struct arg_struct {
    GameManager *manager;
    int client_sockfd;
} Args;

void sendToClient(int sockfd, char *msg) {
    send(sockfd, msg, strlen(msg), 0);
}


/* Main handling thread when a new socket is connected to the server. Til now
 * only build the CREATE and JOIN command, for any other command will be UNRECOGNIZED */
void *roomManagement(void *arguments) {
    if (pthread_detach(pthread_self())) {
        printf("[ROOM_MANAGEMENT] [ERROR] pthread_detach error\n");
    }
    Args *args = (Args *) arguments;
    GameManager *manager = args->manager;
    int client_sock = args->client_sockfd;
    free(args); // no need to use

    int byte_recv;
    char buff[BUFF_SIZE];

    printf("[ROOM_MANAGEMENT] [INFO] Created thread for client sock id = %d\n", client_sock);
    while (1) {
        memset(buff, 0, BUFF_SIZE);
        byte_recv = recv(client_sock, buff, BUFF_SIZE, 0);
        if (byte_recv < 0) {
            perror("[ROOM_MANAGEMENT] [ERROR] Read error");
            close(client_sock);
            pthread_exit(NULL);
        } else if (byte_recv == 0) {
            printf("[ROOM_MANAGEMENT] [INFO] Connection closed.\n");
            pthread_exit(NULL);
        }


        // parse buffer to command array with character ~ between each
        char **cmdArr = parseCmd(buff);
        // get command from each command array
        CmdValue cmdValue = getCommand(cmdArr);
        int code;
        switch (cmdValue.type) {
            case CREATE:
                code = getFreeRoom(manager, cmdValue.createCmd.boardSize, cmdValue.createCmd.name, client_sock);
                if (code == -1) {
                    printf("[ROOM_MANAGEMENT] [WARN] Full Room to create! sockfd = %d\n", client_sock);
                    sendToClient(client_sock, "status~0~Full Room to create");
                    destroyCmd(cmdArr);
                    close(client_sock);
                    pthread_exit(NULL);
                }
                char s[BUFF_SIZE];
                sprintf(s, "status~1~Create Room Successfully~%d", code);
                sendToClient(client_sock, s);
                break;
            case JOIN:
                code = requestJoinRoom(cmdValue.joinCmd.roomCode, cmdValue.joinCmd.name, client_sock, manager);
                if (code == -1) {
                    printf("[ROOM_MANAGEMENT] [WARN] Wrong Room Code! sockfd = %d\n", client_sock);
                    sendToClient(client_sock, "status~0~Wrong Room Code");
                    destroyCmd(cmdArr);
                    close(client_sock);
                    pthread_exit(NULL);
                } else if (code == -2) {
                    printf("[ROOM_MANAGEMENT] [WARN] Room is full! sockfd = %d\n", client_sock);
                    sendToClient(client_sock, "status~0~Room is full");
                    destroyCmd(cmdArr);
                    close(client_sock);
                    pthread_exit(NULL);
                }
                sendToClient(client_sock, "status~1~Join room successfully");
                break;
            case WATCH:
                code = requestWatchRoom(cmdValue.watchCmd.roomCode, cmdValue.watchCmd.name, client_sock, manager);
                if (code == -1) {
                    printf("[ROOM_MANAGEMENT] [WARN] Wrong Room Code! sockfd = %d\n", client_sock);
                    sendToClient(client_sock, "status~0~Wrong Room Code");
                    destroyCmd(cmdArr);
                    close(client_sock);
                    pthread_exit(NULL);
                } else if (code == -2) {
                    printf("[ROOM_MANAGEMENT] [WARN] Room is full! sockfd = %d\n", client_sock);
                    sendToClient(client_sock, "status~0~Room is full");
                    destroyCmd(cmdArr);
                    close(client_sock);
                    pthread_exit(NULL);
                }
                sendToClient(client_sock, "status~1~Watch room successfully");
                sleep(1);
                GameBoard *gameBoard = manager->RoomGameList[code];
                char *sendBoard = serializeBoard('-', '-', gameBoard->size, gameBoard->board);
                int status = send(client_sock, sendBoard, strlen(sendBoard), MSG_NOSIGNAL);
                break;
            case INFO: {
                char *sendListBoard = getListRoom(manager);
                send(client_sock, sendListBoard, strlen(sendListBoard), MSG_NOSIGNAL);
                continue;
                break;
            }
            default:
                printf("[ROOM_MANAGEMENT] [WARN] Unrecognized command %s\n", cmdArr[0]);
                sendToClient(client_sock, "status~0~Unrecognized command");
                destroyCmd(cmdArr);
                close(client_sock);
                pthread_exit(NULL);
        }
        break;
    }
    printf("[ROOM_MANAGEMENT] [INFO] Close thread\n");
    pthread_exit(NULL);
}


// START GAME SERVER HERE
int main(int argc, char **argv) {
    /*  set rand num for room code generation - need to modify the room generation code */
    srand(time(NULL));

    /* INIT SOCKET SERVER */

    int server_sock, port; /* file descriptors */
    struct sockaddr_in server;    /* server's address information */
    struct sockaddr_in client;    /* client's address information */


    if (argc != 2) {
        printf("[MAIN] [ERROR] run application with a port argument");
        exit(1);
    }
    port = strtol(argv[1], NULL, 10);
    if (port <= 0) {
        printf("[MAIN] [ERROR] port must be int > 0");
        exit(1);
    }

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /* calls socket() */
        printf("[MAIN] [ERROR] socket() error\n");
        return 0;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

    if (bind(server_sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("\n[MAIN] [ERROR]Error: ");
        return 0;
    }
    printf("[INFO] Server is running at port %d\n", port);
    if (listen(server_sock, BACKLOG) == -1) {
        perror("\n[MAIN] [ERROR]Error: ");
        return 0;
    }
    pthread_t tid;

    /* INIT GAME MANAGER */

    GameManager *manager = newGameManager();


    while (1) {
        printf("[MAIN] [INFO] Listening...\n");
        int client_sock = accept(server_sock, NULL, NULL);
        printf("[MAIN] [INFO] Connection accepted\n");
        if (client_sock < 0) {
            printf("[MAIN] [INFO] Server acccept failed...\n");
            exit(0);
        } else
            printf("[MAIN] [INFO] Server acccept the client\n");
        Args *args = (Args *) malloc(sizeof(Args));
        args->manager = manager;
        args->client_sockfd = client_sock;
        pthread_create(&tid, NULL, &roomManagement, (void *) args);
    }
    close(server_sock);
    return 0;
}