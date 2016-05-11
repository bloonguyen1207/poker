//
// Created by bloo on 18/04/2016.
//

#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "poker.h"
#define POKER_COLOR_BACKGROUND    6
#define POKER_COLOR_RED           1
#define POKER_COLOR_YELLOW        3
#define POKER_COLOR_WHITE         7

static void init_screen();
static void finish(int sig);

void drawCard(Card card, int y, int x) {
    if (card.suit == SPADES) {
        mvaddstr(y, x, ".______."); y++;
        mvprintw(y, x, "|%d  .  |", card.suit); y++;
        mvaddstr(y, x, "|  / \\ |"); y++;
        mvaddstr(y, x, "| (_,_)|"); y++;
        mvaddstr(y, x, "|   I  |"); y++;
        mvaddstr(y, x, ".______.");
    } else if (card.suit == CLUBS) {
        mvaddstr(y, x, ".______."); y++;
        mvprintw(y, x, "|%d  _  |", card.suit); y++;
        mvaddstr(y, x, "|  ( ) |"); y++;
        mvaddstr(y, x, "| (_x_)|"); y++;
        mvaddstr(y, x, "|   I  |"); y++;
        mvaddstr(y, x, ".______.");
    } else if (card.suit == DIAMONDS) {
        mvaddstr(y, x, ".______."); y++;
        mvprintw(y, x, "|%d /\\  |", card.suit); y++;
        mvaddstr(y, x, "| /  \\ |"); y++;
        mvaddstr(y, x, "| \\  / |"); y++;
        mvaddstr(y, x, "|  \\/  |"); y++;
        mvaddstr(y, x, ".______.");
    } else if (card.suit == HEARTS) {
        mvaddstr(y, x, ".______."); y++;
        mvprintw(y, x, "|%d_  _ |", card.suit); y++;
        mvaddstr(y, x, "|( \\/ )|"); y++;
        mvaddstr(y, x, "| \\  / |"); y++;
        mvaddstr(y, x, "|  \\/  |"); y++;
        mvaddstr(y, x, ".______.");
    } else if (card.suit == NONE) {
        mvaddstr(y, x, ".______."); y++;
        mvaddstr(y, x, "|______|"); y++;
        mvaddstr(y, x, "|______|"); y++;
        mvaddstr(y, x, "|______|"); y++;
        mvaddstr(y, x, "|______|"); y++;
        mvaddstr(y, x, ".______.");
    }

    move(0, 0);
    refresh();
}

void center(int row, char *title) {
    int len, start_point;
    for (len = 0; ; len++) {
        if (title[len] == '\0') {
            break;
        }
    }
    start_point = (COLS - len) / 2;
    mvprintw(row, start_point, title);
    refresh();
}

void drawPlayerCards(Player * players, int num_player) {
    Card card;
    card.suit = NONE;

    int players_posyx[5][2] = {{25, 30}, {5, 50}, {5, 30}, {5, 10}, {25, 10}};
    if (num_player == 2) {
        players_posyx[1][0] = 5; players_posyx[1][1] = 30;
    }
    if (num_player == 3) {
        players_posyx[2][0] = 5; players_posyx[2][1] = 10;
    }

    for (int i = 0; i < num_player; i++) {
        if (i < 4 && i > 0 ) {
            mvprintw(players_posyx[i][0] - 1, players_posyx[i][1], "%s", players[i].name);
            if (players[i].isBigBlind) {
                mvaddstr(players_posyx[i][0] - 1, players_posyx[i][1] + 13, "BB");
            } else if (players[i].isSmallBlind) {
                mvaddstr(players_posyx[i][0] - 1, players_posyx[i][1] + 13, "SB");
            }
        } else {
            mvprintw(players_posyx[i][0] + 6, players_posyx[i][1], "%s", players[i].name);
            if (players[i].isBigBlind) {
                mvaddstr(players_posyx[i][0] + 6, players_posyx[i][1] + 13, "BB");
            } else if (players[i]. isSmallBlind) {
                mvaddstr(players_posyx[i][0] + 6, players_posyx[i][1] + 13, "SB");
            }
        }
        if (players[i].status == 0) {
            continue;
        }
        if (i == 0) {
            drawCard(players[i].hand[0], players_posyx[i][0], players_posyx[i][1]);
            drawCard(players[i].hand[1], players_posyx[i][0], players_posyx[i][1] + 7);
        } else {
            drawCard(card, players_posyx[i][0], players_posyx[i][1]);
            drawCard(card, players_posyx[i][0], players_posyx[i][1] + 7);
        }
    }
}

void drawPlayerInfo(Player * players, int num_player) {
    int info_y = 0;
    mvaddstr(info_y, 100, "Money   Bet"); info_y += 2;
    for (int i = 0; i < num_player; i++) {
        mvprintw(info_y, 90, "%s", players[i].name);
        mvprintw(info_y, 100, "%d", players[i].money);
        mvprintw(info_y, 107, "%d", players[i].bet);
        info_y += 2;
    }
}

void drawPot(Table table) {
    mvaddstr(12, 90, "----------------------");
    mvprintw(14, 90, "Pot: %d", table.pot_money);
}

void drawSharedCard(Table table, int roundIdx) {
    if (roundIdx > 0) {
        int x = 20;
        for (int i = 0; i < roundIdx + 2; i++) {
            drawCard(table.card[i], 15, x);
            x += 7;
        }
    }
}

void drawRangeMoney(int min, int max, int item, int money) {
    int c;
    char menu[4][10] = {"<", ">", "OK", "Cancel"};

    for (c = 0; c < 4; c++) {
        if (c == item) {
            attron(A_REVERSE);
        }
        if (c == 0) {
            if (money != min) {
                mvaddstr(28, 70, menu[c]);
            }
        } else if (c == 1) {
            if (money != max) {
                mvaddstr(28, 80, menu[c]);
            }
        } else if (c == 2) {
            mvaddstr(29, 70, menu[c]);
        } else if (c == 3) {
            mvaddstr(29, 75, menu[c]);
        }
        attroff(A_REVERSE);
    }
    mvaddstr(27, 71, "How much?");
    mvprintw(28, 73, "%d", money);


    move(0, 0);

    refresh();
}

int interactRangeMoney(int min, int max, int money, int item) {
    int key = 0;

    keypad(stdscr, TRUE);

    while (key != 13) { //13 is Enter
        drawRangeMoney(min, max, item, money);
        key = getch();
        switch (key) {
            case KEY_DOWN:
                if (item == 0 || item == 1) {
                    item += 2;
                } else if (item < 3){
                    item++;
                }
                break;
            case KEY_UP:
                if (item == 2 || item == 3) {
                    if (money == min && item == 2) {
                        item--;
                    } else if (money == max && item == 3) {
                        item++;
                    } else {
                        item -= 2;
                    }
                }
                break;
            case KEY_RIGHT:
                if ((item == 0 && money != max) || item == 2) {
                    item++;
                }
                break;
            case KEY_LEFT:
                if ((item == 1 && money != min) || item == 3) {
                    item--;
                }
                break;
            default: break;
        }
    }
    return item;
}

int rangeMoney(int min, int max) {
    int money = min;
    int choice = 1;
    int endMenu = 0;
    int item;

    while (!endMenu) {
        item = choice;
        choice = interactRangeMoney(min, max, money, item);
        if (choice == 0) {
            if (money > min) {
                money--;
            }
        } else if (choice == 1) {
            if (money < max) {
                money++;
            }
        } else if (choice == 2 || choice == 3) {
            if (choice == 3) {
                money = -1;
            }
            endMenu = 1;
        }
    }
    return money;
}

void drawGame(int num_player, int roundIdx, Player * players, Table * table, int playerIdx, int item) {
    clear();
    drawPlayerCards(players, num_player);
    drawSharedCard(*table, roundIdx);
    drawPlayerInfo(players, num_player);
    drawPot(*table);

    //draw Option menu
    int c;
    char menu[7][15] = {"1. Call", "2. Raise", "1. Check", "2. Bet", "1-2. Allin", "3. Fold", "Exit"};

    for (c = 0; c < 7; c++) {
        if (c == item) {
            attron(A_REVERSE);
        }
        if (playerIdx == 0 && players[playerIdx].status != 0) {
            if (isCallRaise(players[0], *table)) {
                if (isAllin(players[0], *table)) {
                    if (c == 4) {
                        mvaddstr(26, 55, menu[c]);
                    }
                } else if (players[0].money < table->highest_bet - players[0].bet + table->last_bet) {
                    if (c == 0) {
                        mvaddstr(26, 55, menu[c]);
                    } else if (c == 4) {
                        mvaddstr(28, 55, menu[c]);
                    }
                } else {
                    if (c == 0) {
                        mvaddstr(26, 55, menu[c]);
                    } else if (c == 1) {
                        mvaddstr(28, 55, menu[c]);
                    }
                }
            } else if (isCheckBet(players[0], *table)) {
                if (c == 2) {
                    mvaddstr(26, 55, menu[c]);
                }
                if (players[0].money >= table->ante * 2) {
                    if (c == 3) {
                        mvaddstr(28, 55, menu[c]);
                    }
                } else {
                    if (c == 4) {
                        mvaddstr(28, 55, menu[c]);
                    }
                }
            }
            if (c == 5) {
                mvaddstr(30, 55, menu[c]);
            }
        }
        if (c == 6) {
            mvaddstr(30, 100, menu[6]);
        }
        attroff(A_REVERSE);
    }

    move(0, 0);
    refresh();
}

int interactGame(int num_player, int roundIdx, Player * players, Table * table, int playerIdx) {
    int key = 0;
    int item = 0;

    //initialize the first option that player can choose
    if (playerIdx == 0 && players[playerIdx].status != 0) {
        if (isCallRaise(players[0], *table)) {
            if (isAllin(players[0], *table)) {
                item = 4;
            } else {
                item = 0;
            }
        } else if (isCheckBet(players[0], *table)) {
            item = 2;

        } else {
            item = 5;
        }
    } else {
        item = 6;
    }

    keypad(stdscr, TRUE);

    while (key != 13) { //13 is Enter
        drawGame(num_player, roundIdx, players, table, playerIdx, item);
        key = getch();
        if (playerIdx == 0 && players[playerIdx].status != 0) {
            if (isCallRaise(players[0], *table)) {
                if (isAllin(players[0], *table)) {
                    switch (key) {
                        case KEY_DOWN:
                            item++;
                            if (item > 6) {
                                item = 4;
                            }
                            break;
                        case KEY_UP:
                            item--;
                            if (item < 4) {
                                item = 6;
                            }
                            break;
                        case KEY_RIGHT:
                            item = 6;
                            break;
                        case KEY_LEFT:
                            item = 5;
                            break;
                        default: break;
                    }
                } else if (players[0].money < table->highest_bet - players[0].bet + table->last_bet) {
                    switch (key) {
                        case KEY_DOWN:
                            if (item == 0) {
                                item = 4;
                            } else {
                                item++;
                            }
                            if (item > 6) {
                                item = 0;
                            }
                            break;
                        case KEY_UP:
                            if (item == 4) {
                                item = 0;
                            } else {
                                item--;
                            }
                            if (item < 0) {
                                item = 6;
                            }
                            break;
                        case KEY_RIGHT:
                            item = 6;
                            break;
                        case KEY_LEFT:
                            item = 5;
                            break;
                        default: break;
                    }
                } else {
                    switch (key) {
                        case KEY_DOWN:
                            if (item == 1) {
                                item = 5;
                            } else {
                                item++;
                            }
                            if (item > 6) {
                                item = 0;
                            }
                            break;
                        case KEY_UP:
                            if (item == 5) {
                                item = 1;
                            } else {
                                item--;
                            }
                            if (item < 0) {
                                item = 6;
                            }
                            break;
                        case KEY_RIGHT:
                            item = 6;
                            break;
                        case KEY_LEFT:
                            item = 5;
                            break;
                        default: break;
                    }
                }
            } else if (isCheckBet(players[0], *table)) {
                if (players[0].money >= table->ante * 2) {
                    switch (key) {
                        case KEY_DOWN:
                            if (item == 3) {
                                item = 5;
                            } else {
                                item++;
                            }
                            if (item > 6) {
                                item = 2;
                            }
                            break;
                        case KEY_UP:
                            if (item == 5) {
                                item = 3;
                            } else {
                                item--;
                            }
                            if (item < 2) {
                                item = 6;
                            }
                            break;
                        case KEY_RIGHT:
                            item = 6;
                            break;
                        case KEY_LEFT:
                            item = 5;
                            break;
                        default: break;
                    }
                } else {
                    switch (key) {
                        case KEY_DOWN:
                            if (item == 2) {
                                item = 4;
                            } else {
                                item++;
                            }
                            if (item > 6) {
                                item = 2;
                            }
                            break;
                        case KEY_UP:
                            if (item == 4) {
                                item = 2;
                            } else {
                                item--;
                            }
                            if (item < 2) {
                                item = 6;
                            }
                            break;
                        case KEY_RIGHT:
                            item = 6;
                            break;
                        case KEY_LEFT:
                            item = 5;
                            break;
                        default: break;
                    }
                }
            }
        }
    }
    return item;
}

void gamePoker(int num_player) {
    int item = 0;
    Player * players = createPlayers(num_player);
    Table * table = createTable();
    for (int i = 0; i < 5; i++) {
        table->card[i].suit = SPADES;
        table->card[i].rank = 5;
    }
    players[0].hand[0].rank = 4;
    players[0].hand[0].suit = CLUBS;
    players[0].hand[1].rank = 2;
    players[0].hand[1].suit = DIAMONDS;
    players[0].isBigBlind = 1;
    players[0].money = 20000;
    players[0].bet = 10000;
    for (int i = 0; i < 4; i++) {
        int money = -1;
        table->pot_money += 200;
        table->highest_bet += 15000;
        int playerIdx = 0;
        while (money == -1) {
            item = interactGame(num_player, i, players, table, playerIdx);
            if (item == 0) {
                //call "call" func
                money = 0;
            } else if (item == 2) {
                //call "check" func
                money = 0;
            } else if (item == 1 || item == 3) {
                int min = minMoney(players[0], *table);
                money = rangeMoney(min, players[0].money);
                if (money == -1) {
                    mvaddstr(36, 30, "Cancel");
                } else {
                    mvprintw(36, 30, "Money: %d", money);
                }
                refresh();
            } else if (item == 4) {
                //call "all in" func
                money = players[0].money;
            } else if (item == 5) {
                players[0].status = 0;
                money = 0;
            } else if (item == 6) {
                break;

            }
        }
        if (item == 6) {
            //break;
            //to test, use the bellow code, will be erased late
            getch();
        }

        mvprintw(38, 30, "Item is: %d", item);
        refresh();
        getch();
    }
    free(players);
    free(table);
}

void drawStartMenu(int item, int num_player) {
    int c;
    char menu[4][10] = {"<", ">", "Start", "Main Menu"};

    clear();
    center(5, "Number of Players:");
    for (c = 0; c < 4; c++) {
        if (c == item) {
            attron(A_REVERSE);
        }
        if (c == 0) {
            if (num_player != 2) {
                mvaddstr(7, COLS / 2 - 2, menu[c]);
            }
        } else if (c == 1) {
            if (num_player != 5) {
                mvaddstr(7, COLS / 2 + 2, menu[c]);
            }
        } else if (c == 2) {
            center(9, menu[c]);
        } else if (c == 3) {
            center(11, menu[c]);
        }
        attroff(A_REVERSE);
    }

    mvprintw(7, COLS / 2, "%d", num_player);
    move(0, 0);

    refresh();
}

int interactStartMenu(int num_player, int item) {
    int key = 0;

    keypad(stdscr, TRUE);

    while (key != 13) { //13 is Enter
        drawStartMenu(item, num_player);
        key = getch();
        switch (key) {
            case KEY_DOWN:
                if (item == 0) {
                    item = 2;
                    break;
                }
                item++;
                if (item > 3) {
                    if (num_player == 2) {
                        item = 1;
                    } else {
                        item = 0;
                    }
                }
                break;
            case KEY_UP:
                if (item == 1) {
                    item = 3;
                    break;
                }
                if (item == 2 && num_player == 5) {
                    item = 0;
                    break;
                }
                item--;
                if (item < 0) {
                    item = 3;
                }
                break;
            case KEY_LEFT:
                if (item == 1) {
                    if (num_player != 2) {
                        item = 0;
                    }
                    break;
                }
                break;
            case KEY_RIGHT:
                if (item == 0) {
                    if (num_player != 5) {
                        item = 1;
                    }
                    break;
                }
                break;
            default: break;
        }
    }
    return item;
}

void startMenu() {
    int endMenu = 0;
    int num_player = 2;
    int item;
    int choice = 1;

    while (!endMenu) {
        item = choice;
        choice = interactStartMenu(num_player, item);
        if (choice == 0) {
            if (num_player > 2) {
                num_player--;
            }
        } else if (choice == 1) {
            if (num_player < 5) {
                num_player++;
            }
        } else if (choice == 2) {
            gamePoker(num_player);
        } else if (choice == 3) {
            endMenu = 1;
        }
    }
}

void drawMainMenu(int item) {
    int c;
    char menu[5][20] = {"Start", "Load", "Credit", "Highscore", "Exit"};

    clear();
    center(3, "POKER");
    center(7, "Main Menu");
    for (c = 0; c < 5; c++) {
        if (c == item) {
            attron(A_REVERSE);
        }
        mvaddstr(11 + (c * 2), COLS / 2 - 5, menu[c]);
        attroff(A_REVERSE);
    }
    mvaddstr(LINES - 2, COLS - 20, "Move: Arrow keys");
    mvaddstr(LINES - 1, COLS - 20, "Select: Enter");
    move(0, 0);

    refresh();
}

int interactMainMenu() {
    int key = 0;
    int item = 0;

    keypad(stdscr, TRUE);

    while (key != 13) { //13 is Enter
        drawMainMenu(item);
        key = getch();
        switch (key) {
            case KEY_DOWN:
                item++;
                if (item > 4) {
                    item = 0;
                }
                break;
            case KEY_UP:
                item--;
                if (item < 0) {
                    item = 4;
                }
                break;
            default: break;
        }
    }
    return item;
}

int main() {
    int endProgram = 0;
    init_screen();
    while (!endProgram) {
        struct timespec delay = {0, 500000000L},
                rem;
        int choice = interactMainMenu();
        clear();
        if (choice == 0) {
            startMenu();
        } else if (choice == 1) {
            center(1, "LOAD GAME");
            mvaddstr(LINES - 1, COLS - 20, "Back: Any key");
            refresh();
            getch();
        } else if (choice == 2) {
            center(1, "CREDIT");
            mvaddstr(LINES - 1, COLS - 20, "Back: Any key");
            refresh();
            getch();
        } else if (choice == 3) {
            center(1, "HIGHSCORE");
            mvaddstr(LINES - 1, COLS - 20, "Back: Any key");
            refresh();
            getch();
        } else if (choice == 4) {
            endProgram = 1;
        }
    }
    finish(0);
}

static void init_screen() {
    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */
    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, don't wait for \n */
    (void) noecho();       /* do not echo input */
    //timeout(500);          /* wait maximum 500ms for a character */
    /* Use timeout(-1) for blocking mode */
    timeout(-1);

    if (has_colors()) {

        start_color();
        // initialise you color pairs (foreground, background)
        init_pair(1, POKER_COLOR_YELLOW, POKER_COLOR_BACKGROUND);
        init_pair(2, POKER_COLOR_RED, POKER_COLOR_BACKGROUND);
        init_pair(3, POKER_COLOR_WHITE, POKER_COLOR_BACKGROUND);
    }
    /* set default color pair */
    attrset(COLOR_PAIR(1));
    bkgd(COLOR_PAIR(1));
}

static void finish(int sig) {
    endwin();

    /* do your non-curses wrap up here, like freeing the memory allocated */


    exit(sig);
}

int xmain1() {
    chtype c;
    init_screen();
    struct timespec delay = {0, 500000000L},
            rem;
    int endProgram = 0, x = 0, y = 0, maxx, maxy;
    while (!endProgram) {
        c = (chtype) getch();
        if (c == 'q') {
            endProgram = 1;
        }
        getmaxyx(stdscr, maxy, maxx);
        raw();
        keypad(stdscr, TRUE);        /* Get keyboard	input	*/
        noecho();
        if (c == KEY_DOWN && y < maxy - 1) {
            y++;
        } else if (c == KEY_RIGHT && x < maxx - 1) {
            x++;
        } else if (c == ' ' || c == -1) {
            c = mvinch(y, x);
            mvaddch(y, x, c+1);
        }
        move(y, x);
    }
    finish(0);
    return 0;
}

//Menu
//static void init_screen();
//static void finish(int sig);
//
//#define WIDTH 30
//#define HEIGHT 10
//
//int startx = 0;
//int starty = 0;
//
//void center(int row, char *title) {
//    int len, start_point;
//    for (len = 0; ; len++) {
//        if (title[len] == '\0') {
//            break;
//        }
//    }
//    start_point = (COLS - len) / 2;
//    mvprintw(row, start_point, title);
//    refresh();
//}
//
//char *choices[] = {
//        "Start",
//        "Load",
//        "Options",
//        "Highscore",
//        "Exit",
//};
//int n_choices = sizeof(choices) / sizeof(char *);
//void print_menu(WINDOW *menu_win, int highlight);
//
//int main()
//{	WINDOW *menu_win;
//    int highlight = 1;
//    int choice = 0;
//    int c;
//
//    initscr();
//    clear();
//    noecho();
//    cbreak();	/* Line buffering disabled. pass on everything */
//    startx = (80 - WIDTH) / 2;
//    starty = (24 - HEIGHT) / 2;
//
//    menu_win = newwin(HEIGHT, WIDTH, starty, startx);
//    keypad(menu_win, TRUE);
//    mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
//    print_menu(menu_win, highlight);
//    refresh();
//    while(1) {
//        c = wgetch(menu_win);
//        switch(c) {
//            case KEY_UP:
//                if(highlight == 1)
//                    highlight = n_choices;
//                else
//                    --highlight;
//                break;
//            case KEY_DOWN:
//                if(highlight == n_choices)
//                    highlight = 1;
//                else
//                    ++highlight;
//                break;
//            case 10:
//                choice = highlight;
//                break;
//            default:
//                break;
//        }
//        print_menu(menu_win, highlight);
//        clear();
//        if (choice == 1) {
//            center(1, "START GAME");
//            mvaddstr(LINES - 1, COLS - 20, "Back: Any key");
//            refresh();
//            getch();
//        } else if (choice == 2) {
//            center(1, "LOAD GAME");
//            mvaddstr(LINES - 1, COLS - 20, "Back: Any key");
//            refresh();
//            getch();
//        } else if (choice == 3) {
//            center(1, "OPTIONS");
//            mvaddstr(LINES - 1, COLS - 20, "Back: Any key");
//            refresh();
//            getch();
//        } else if (choice == 4) {
//            center(1, "HIGHSCORE");
//            mvaddstr(LINES - 1, COLS - 20, "Back: Any key");
//            refresh();
//            getch();
//        } else if(choice == 5) {
//            break;
//        }
//    }
//    mvprintw(23, 0, "You chose choice %d with choice string %s\n", choice, choices[choice - 1]);
//    clrtoeol();
//    refresh();
//    endwin();
//    return 0;
//}
//
//
//void print_menu(WINDOW *menu_win, int highlight)
//{
//    int x, y, i;
//
//    x = 2;
//    y = 2;
//    center(3, "POKER");
//    center(6, "Main menu");
//    box(menu_win, 0, 0);
//    for(i = 0; i < n_choices; ++i)
//    {	if(highlight == i + 1) /* High light the present choice */
//        {	wattron(menu_win, A_REVERSE);
//            mvwprintw(menu_win, y, x, "%s", choices[i]);
//            wattroff(menu_win, A_REVERSE);
//        }
//        else
//            mvwprintw(menu_win, y, x, "%s", choices[i]);
//        ++y;
//    }
//    wrefresh(menu_win);
//}

//int main(int argc, char *argv[]) {
//
//    int x = 0, y = 0, maxx, maxy;
//    chtype c;
//
//    struct timespec delay = {0, 500000000L},
//            rem;
//
//    init_screen();
//
//    for (int i = 1; ; i++) {
//        c = (chtype) getch();     /* refresh, accept single keystroke of input */
//
//        /* process the command keystroke */
//        if (c == 'q') {
//            break;
//        }
//
//        getmaxyx(stdscr, maxy, maxx);
//        if (c == KEY_DOWN && y < maxy-1) {
//            y++;
//        } else if (c == KEY_RIGHT && x < maxx-1) {
//            x++;
//        } else if (c == ' ' || c == -1) {
//            attrset(COLOR_PAIR(i%3+1));
//            c = mvinch(y, x);
//            mvaddch(y, x, c+1);
//        }
//        move(y, x);
//
//        //nanosleep(&delay, &rem);
//    }
//
//    finish(0);               /* we're done */
//}
//
//
//static void init_screen() {
//    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */
//    (void) initscr();      /* initialize the curses library */
//    keypad(stdscr, TRUE);  /* enable keyboard mapping */
//    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
//    (void) cbreak();       /* take input chars one at a time, don't wait for \n */
//    (void) noecho();       /* do not echo input */
//    timeout(500);          /* wait maximum 500ms for a character */
//    /* Use timeout(-1) for blocking mode */
//    printw("Hello World !!!");
//    if (has_colors()) {
//
//        start_color();
//        // initialise you color pairs (foreground, background)
//        init_pair(1, POKER_COLOR_TEXT, POKER_COLOR_BACKGROUND);
//        init_pair(2, POKER_COLOR_RED, POKER_COLOR_BACKGROUND);
//        init_pair(3, POKER_COLOR_BLUE, POKER_COLOR_BACKGROUND);
//    }
//    /* set default color pair */
//    attrset(COLOR_PAIR(1));
//}
//
//static void finish(int sig) {
//    endwin();
//
//    /* do your non-curses wrap up here, like freeing the memory allocated */
//
//
//    exit(sig);
//}


