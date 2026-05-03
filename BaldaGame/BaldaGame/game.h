#ifndef GAME_H
#define GAME_H

#define BOARD_SIZE 5
#define MAX_PLAYER_WORDS 100

struct Cell {
    unsigned char letter;
    int owner;
    int isFixed;
};

struct PlayerWords {
    char words[MAX_PLAYER_WORDS][30];
    int scores[MAX_PLAYER_WORDS];
    int count;
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

extern struct PlayerWords player1Words;
extern struct PlayerWords player2Words;

void initGame();
void placeRussianLetter(int row, int col, unsigned char letter);
int isCellEmpty(int row, int col);
void clearSelection();
unsigned char getRussianLetter(int key);
int checkAndAddWordDirect(int row, int col, int player);
void addPoints(int points, int player);
void addWordToPlayer(int player, const char* word, int points);
void clearPlayerWords();

int isWordUsedGlobal(const char* word);
void addUsedWordGlobal(const char* word);
int getUsedWordsCount(void);
void clearUsedWords(void);

void copyUsedWords(char dest[1000][30], int* destCount);
void restoreUsedWords(char src[1000][30], int srcCount);
int isGameOver(void);
void forceRedraw(void);

#endif