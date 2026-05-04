#include "menu.h"
#include "graphics.h"
#include "game.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

enum GameState gameState = STATE_MENU;
enum GameMode gameMode = MODE_PVE;
enum BotDifficulty botDifficulty = DIFFICULTY_MEDIUM;
int selectedMenuItem = 0;
int selectingMode = 0;
int selectingDifficulty = 0;

const char* menuItems[] = { "New Game", "Records", "Help", "Exit" };
const int menuItemsCount = 4;

void initMenu() {
    selectedMenuItem = 0;
    selectingMode = 0;
    selectingDifficulty = 0;
    gameState = STATE_MENU;
}

void menuUp() {
    if (selectingMode) {
        selectedMenuItem--;
        if (selectedMenuItem < 0) selectedMenuItem = 1;
    }
    else if (selectingDifficulty) {
        selectedMenuItem--;
        if (selectedMenuItem < 0) selectedMenuItem = 2;
    }
    else {
        selectedMenuItem--;
        if (selectedMenuItem < 0) selectedMenuItem = menuItemsCount - 1;
    }
}

void menuDown() {
    if (selectingMode) {
        selectedMenuItem++;
        if (selectedMenuItem > 1) selectedMenuItem = 0;
    }
    else if (selectingDifficulty) {
        selectedMenuItem++;
        if (selectedMenuItem > 2) selectedMenuItem = 0;
    }
    else {
        selectedMenuItem++;
        if (selectedMenuItem >= menuItemsCount) selectedMenuItem = 0;
    }
}

void menuSelect() {
    if (selectingDifficulty) {
        switch (selectedMenuItem) {
        case 0:
            botDifficulty = DIFFICULTY_EASY;
            break;
        case 1:
            botDifficulty = DIFFICULTY_MEDIUM;
            break;
        case 2:
            botDifficulty = DIFFICULTY_HARD;
            break;
        }
        selectingDifficulty = 0;
        askingName = 1;
        gameState = STATE_GAME;
        printf("Difficulty selected: %s\n",
            botDifficulty == DIFFICULTY_EASY ? "Easy" :
            botDifficulty == DIFFICULTY_MEDIUM ? "Medium" : "Hard");
        return;
    }

    if (selectingMode) {
        switch (selectedMenuItem) {
        case 0:
            gameMode = MODE_PVP;
            selectingMode = 0;
            askingName = 0;
            initGame();
            gameState = STATE_GAME;
            printf("Mode selected: PVP\n");
            break;
        case 1:
            gameMode = MODE_PVE;
            selectingMode = 0;
            selectingDifficulty = 1;
            selectedMenuItem = 0;
            printf("Mode selected: PVE, now choose difficulty\n");
            break;
        }
        return;
    }

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
void menuBack() {
    if (selectingDifficulty) {
        selectingDifficulty = 0;
        selectingMode = 1;
        selectedMenuItem = 0;
    }
    else if (selectingMode) {
        selectingMode = 0;
        selectedMenuItem = 0;
    }
}