//
// Created by monstertau on 24/04/2021.
//

#include <malloc.h>
#include "game_manager.h"

void testGetFreeRoom(int listTakenRoom[], int n) {
    GameManager *manager = (GameManager *) malloc(sizeof(GameManager));
    for (int i = 0; i < n; i++) {
        int j = listTakenRoom[i];
        manager->RoomGameList[j] = (GameBoard *) malloc(sizeof(GameBoard));
    }
    int res = getFreeRoom(manager);
    printf("%d\n", res);
}

int main() {
    int arr1[] = {0};
    testGetFreeRoom(arr1, 1); // free room slot = 1
    int arr2[] = {0, 1, 2, 3};
    testGetFreeRoom(arr2, 4); // free room slot = 4
    int arr3[] = {0, 1, 2, 4, 5};
    testGetFreeRoom(arr3, 5); // free room slot = 3
    return 0;
}