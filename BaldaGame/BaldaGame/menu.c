#include "menu.h"
#include "graphics.h"
#include "game.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

enum GameState gameState = STATE_MENU;
enum GameMode gameMode = MODE_PVE;
int selectedMenuItem = 0;
int selectingMode = 0;

const char* menuItems[] = { "New Game", "Records", "Help", "Exit" };
const int menuItemsCount = 4;

void initMenu() {
    selectedMenuItem = 0;
    selectingMode = 0;
    gameState = STATE_MENU;
}

void menuUp() {
    if (selectingMode == 0) {
        selectedMenuItem--;
        if (selectedMenuItem < 0) selectedMenuItem = menuItemsCount - 1;
    }
    else {
        selectedMenuItem--;
        if (selectedMenuItem < 0) selectedMenuItem = 1;
    }
}

void menuDown() {
    if (selectingMode == 0) {
        selectedMenuItem++;
        if (selectedMenuItem >= menuItemsCount) selectedMenuItem = 0;
    }
    else {
        selectedMenuItem++;
        if (selectedMenuItem > 1) selectedMenuItem = 0;
    }
}

void menuSelect() {
    if (selectingMode == 0) {
        switch (selectedMenuItem) {
        case 0:
            selectingMode = 1;
            selectedMenuItem = 0;
            break;
        case 1:
            gameState = STATE_RECORDS;
            break;
        case 2:
            gameState = STATE_HELP;
            break;
        case 3:
            exit(0);
            break;
        }
    }
    else {
        switch (selectedMenuItem) {
        case 0:
            gameMode = MODE_PVP;
            break;
        case 1:
            gameMode = MODE_PVE;
            break;
        }
        selectingMode = 0;

        printf("Выбран режим: %s\n", gameMode == MODE_PVP ? "PVP" : "PVE");

        if (gameMode == MODE_PVE) {
            askingName = 1;
            gameState = STATE_GAME;
        }
        else {

            askingName = 0;
            initGame();
            gameState = STATE_GAME;
        }
    }
}

void updateMenu() {
}