#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MENU 4

void drawGame(int n) {
    erase();
    curs_set(1);
    noecho();
    int yBoard = n * 2 + n - 1; // TODO: fix board size as max screen
    int xBoard = n * 4 + n - 1;
    int yCur = 1; // di chuyen doc
    int xCur = 2; // di chuyen ngang
    int xMax, yMax;
    getmaxyx(stdscr, yMax, xMax);
    int move;
    WINDOW *gameWin = newwin(yBoard, xBoard, (yMax - yBoard) / 2, (xMax - xBoard) / 2);
    keypad(gameWin, true);
    // box(gameWin, 0, 0);
    for (int i = 2; i < yBoard; i += 3) {
        wmove(gameWin, i, 0);
        whline(gameWin, ACS_HLINE, xBoard);
    }
    for (int i = 4; i < xBoard; i += 5) {
        wmove(gameWin, 0, i);
        wvline(gameWin, ACS_VLINE, yBoard);
    }
    wmove(gameWin, yCur, xCur);
    refresh();
    wrefresh(gameWin);
    while (1) {
        move = wgetch(gameWin);
        switch (move) {
            case KEY_UP:
                if (yCur > 1) {
                    yCur -= 3;
                }

                wmove(gameWin, yCur, xCur);
                break;
            case KEY_DOWN:
                if (yCur < yBoard - 1) {
                    yCur += 3;
                }

                wmove(gameWin, yCur, xCur);
                break;
            case KEY_LEFT:
                if (xCur > 2) {
                    xCur -= 5;
                }

                wmove(gameWin, yCur, xCur);
                break;
            case KEY_RIGHT:
                if (xCur < xBoard - 2) {
                    xCur += 5;
                }

                wmove(gameWin, yCur, xCur);
                break;
            case ' ':
                mvwprintw(gameWin, yCur, xCur, "X");
                break;
            default:
                break;
        }
        wrefresh(gameWin);
        if (move == 10) {
            break;
        }
    }
    erase();
}

char *inputNewBox(char *label) {
    char *name = malloc(sizeof(char) * 40);
    erase();
    noecho();
    curs_set(1);
    int yMax, xMax;
    int yTitle, xTitle;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW *gameWin = newwin(yMax / 2, xMax / 2, yMax / 4, xMax / 4);
    box(gameWin, 0, 0);
    refresh();
    wrefresh(gameWin);
    keypad(gameWin, true);
    getmaxyx(gameWin, yMax, xMax);
    char title[24] = "TIC TAC TOE ONLINE GAME";
    mvwprintw(gameWin, 0, xMax / 2 - strlen(title) / 2, title);
    mvwprintw(gameWin, 5, xMax / 2 - strlen(label), label);
    getyx(gameWin, yTitle, xTitle);
    wprintw(gameWin, "%s", name);
    int c;
    int i = 0;
    while ((c = wgetch(gameWin)) != 10) {
        switch (c) {
            case KEY_BACKSPACE:
                getyx(gameWin, yMax, xMax);
                if (xMax > xTitle || yMax > yTitle) {
                    name[--i] = '\0';
                }
                break;
            default:
                if (i < 39) // TODO: fix this to take c as printed value
                {
                    name[i++] = c;
                }
                break;
        }
        getmaxyx(stdscr, yMax, xMax);
        werase(gameWin);
        gameWin = newwin(yMax / 2, xMax / 2, yMax / 4, xMax / 4);
        box(gameWin, 0, 0);
        refresh();
        wrefresh(gameWin);
        keypad(gameWin, true);
        getmaxyx(gameWin, yMax, xMax);
        mvwprintw(gameWin, 0, xMax / 2 - strlen(title) / 2, title);
        mvwprintw(gameWin, 5, xMax / 2 - strlen(label), label);
        wprintw(gameWin, "%s", name);
        wrefresh(gameWin);
    }
    erase();
    name[i] = '\0';
    return name;
}

int main() {
    initscr();
    cbreak();
    curs_set(0);
    noecho();
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW *menuWin = newwin(yMax / 2, xMax / 2, yMax / 4, xMax / 4);
    box(menuWin, 0, 0);
    refresh();
    wrefresh(menuWin);

    keypad(menuWin, true);

    getmaxyx(menuWin, yMax, xMax);
    char title[24] = "TIC TAC TOE ONLINE GAME";
    mvwprintw(menuWin, 0, xMax / 2 - strlen(title) / 2, title);
    wrefresh(menuWin);

    char choices[MAX_MENU][40] = {"Start A New Game", "Join An Existing Game", "Game's Rule", "Exit"};
    int choice;
    int highlight = 0;
    while (1) {
        for (int i = 0; i < MAX_MENU; i++) {
            if (highlight == i) {
                wattron(menuWin, A_REVERSE);
            }
            mvwprintw(menuWin, i + 5, xMax / 2 - strlen(choices[i]) / 2, choices[i]);
            wattroff(menuWin, A_REVERSE);
        }
        choice = wgetch(menuWin);
        switch (choice) {
            case KEY_UP:
                highlight--;
                if (highlight == -1)
                    highlight = 0;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight > MAX_MENU - 1)
                    highlight = MAX_MENU - 1;
                break;
            default:
                break;
        }
        if (choice == 10)
            break;
    }
    char *name = NULL;
    char *gameCode = NULL;
    char *size = NULL;
    int gameSize = 0;
    switch (highlight) {
        case 0:
            // name = inputNewBox("Enter Your Name:");
            // gameCode = inputNewBox("Enter Code For New Room:");
            size = inputNewBox("Enter N Board Size (NxN):");
            gameSize = strtol(size, NULL, 10);
            while (gameSize < 3 || gameSize > 20) {
                size = inputNewBox("3<=Size<=20.Enter Again:");
                gameSize = strtol(size, NULL, 10);
            }
            drawGame(gameSize);
            break;
        case 1:
            gameCode = inputNewBox("Enter Exist Room Code:");
            name = inputNewBox("Enter Your Name:");
            break;
        case 2:
            break;
        default:
            break;
    }
    endwin();
    printf("%s %s %d\n", name, gameCode, gameSize);
    return 0;
}