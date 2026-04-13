#ifndef GAME_H
#define GAME_H

#define BOARD_SIZE 5

struct Cell {
    unsigned char letter;
    int owner;
    int isFixed;
};

extern struct Cell board[BOARD_SIZE][BOARD_SIZE];
extern int currentPlayer;
extern int playerScore;
extern int botScore;
extern int selectedRow;
extern int selectedCol;
extern char currentInputLetter;
extern char playerName[20];
extern int askingName;
extern char nameInput[20];
extern int nameInputLen;


void initGame();
void placeRussianLetter(int row, int col, unsigned char letter);
int isCellEmpty(int row, int col);
void clearSelection();
unsigned char getRussianLetter(int key);
int checkAndAddWord(int row, int col, const char* word);
void collectWordFromBoard(char* word, int* letters, int letterCount);
int isValidWord(const char* word);
void addPoints(int points, int player);
int checkAndAddWordDirect(int row, int col, int player);
int isWordUsed(const char* word);
void addUsedWord(const char* word);

#endif