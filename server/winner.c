#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "game_logic.h"

bool isWinTest(GameBoard *gameBoard, Move *move, char label)
{
    int n = (gameBoard->size < 5 ? gameBoard->size : 5);
    // printf("Number of win %d\n", n);

    int count = 0;
    for (int i = 0; i < gameBoard->size; i++)
    {
        if (gameBoard->board[move->x][i] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    count = 0;
    for (int i = 0; i < gameBoard->size; i++)
    {
        if (gameBoard->board[i][move->y] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    // check left to right downward diagonal
    count = 0;
    int startx = move->x - (move->x < move->y ? move->x : move->y);
    int starty = move->y - (move->x < move->y ? move->x : move->y);
    for (int i = 0; i < gameBoard->size; i++)
    {
        int tmpx = startx + i;
        int tmpy = starty + i;
        if (tmpx >= gameBoard->size || tmpy >= gameBoard->size)
            break;
        if (gameBoard->board[tmpx][tmpy] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    // check left to right upward diagonal
    count = 0;
    startx = move->x + (gameBoard->size - 1 - move->x);
    starty = move->y - (gameBoard->size - 1 - move->x);
    for (int i = 0; i < gameBoard->size; i++)
    {
        int tmpx = startx - i;
        int tmpy = starty + i;
        if (tmpx < 0 || tmpy >= gameBoard->size)
            break;
        if (gameBoard->board[tmpx][tmpy] == label)
            count++;
        else
            count = 0;
        if (count == n)
            return true;
    }
    return false;
}

int main(int argc, char const *argv[])
{
    // Player *newPlayer =
    GameBoard *gameBoard = newGameBoard(NULL, 3);
    gameBoard->board[0][0] = 'X';
    gameBoard->board[1][1] = 'X';
    gameBoard->board[2][2] = 'X';
    for (int x = 0; x < gameBoard->size; x++)
    {
        for (int y = 0; y < gameBoard->size; y++)
        {
            printf("%c ", gameBoard->board[x][y]);
        }
        printf("\n");
    }
    for (int i = 0; i < gameBoard->size; i++)
    {
        for (int j = 0; j < gameBoard->size; j++)
        {
            Move *move = newMove(i, j);
            if (isWinTest(gameBoard, move, 'X'))
            {
                printf("move %d %d\n", move->x, move->y);
                printf("Is Winner %d\n", isWinTest(gameBoard, move, 'X'));
            }
        }
    }

    return 0;
}
