#ifndef MENU_H
#define MENU_H

enum GameState {
    STATE_MENU = 0,
    STATE_GAME = 1,
    STATE_HELP = 2,
    STATE_RECORDS = 3
};

enum GameMode {
    MODE_PVP = 0,
    MODE_PVE = 1
};

enum BotDifficulty {
    DIFFICULTY_EASY = 0,
    DIFFICULTY_MEDIUM = 1,
    DIFFICULTY_HARD = 2
};

extern enum GameState gameState;
extern enum GameMode gameMode;
extern enum BotDifficulty botDifficulty;
extern int selectedMenuItem;
extern int selectingMode;
extern int selectingDifficulty;

void initMenu();
void menuUp();
void menuDown();
void menuSelect();

#endif