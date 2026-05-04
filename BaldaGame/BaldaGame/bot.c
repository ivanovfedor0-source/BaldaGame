#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker, "/STACK:167772160")
#include "bot.h"
#include "game.h"
#include "dictionary.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int row;
    int col;
    unsigned char letter;
    int points;
    char word[30];
} BotMove;

static const unsigned char allRussianLetters[] = {
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xA8
};

static const unsigned char frequentLetters[] = {
    0xC0, 0xC5, 0xC8, 0xCE, 0xD3, 0xD2, 0xCD, 0xD1, 0xD0, 0xC2
};

#define RUSSIAN_LETTERS_COUNT 33
#define FREQUENT_LETTERS_COUNT 10
#define MAX_QUEUE_EVAL 5000
#define MAX_CACHE_SIZE 2000

typedef struct {
    int row;
    int col;
    unsigned char letter;
    int player;
    int points;
    char word[30];
    int valid;
} CachedMove;

static CachedMove moveCache[MAX_CACHE_SIZE];
static int cacheSize = 0;

typedef struct {
    int row;
    int col;
    char word[30];
    int len;
    int visited[BOARD_SIZE][BOARD_SIZE];
} BFSStateEval;

int hasAdjacentLetter(int row, int col) {
    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };
    for (int i = 0; i < 4; i++) {
        int newRow = row + dr[i];
        int newCol = col + dc[i];
        if (newRow >= 0 && newRow < BOARD_SIZE && newCol >= 0 && newCol < BOARD_SIZE) {
            if (board[newRow][newCol].letter != 0) return 1;
        }
    }
    return 0;
}

void reverseStringLocal(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

int getCachedPoints(int row, int col, unsigned char letter, int player, char* word) {
    for (int i = 0; i < cacheSize; i++) {
        if (moveCache[i].row == row && moveCache[i].col == col &&
            moveCache[i].letter == letter && moveCache[i].player == player) {
            if (word) strcpy(word, moveCache[i].word);
            return moveCache[i].points;
        }
    }
    return -1;
}

void addToCache(int row, int col, unsigned char letter, int player, int points, const char* word) {
    if (cacheSize < MAX_CACHE_SIZE) {
        moveCache[cacheSize].row = row;
        moveCache[cacheSize].col = col;
        moveCache[cacheSize].letter = letter;
        moveCache[cacheSize].player = player;
        moveCache[cacheSize].points = points;
        strcpy(moveCache[cacheSize].word, word);
        moveCache[cacheSize].valid = 1;
        cacheSize++;
    }
}

void clearCache() {
    cacheSize = 0;
}

void saveGameState(int* savedPlayerScore, int* savedBotScore, int* savedUsedCount, char savedUsedWords[1000][30]) {
    *savedPlayerScore = playerScore;
    *savedBotScore = botScore;
    *savedUsedCount = getUsedWordsCount();
    copyUsedWords(savedUsedWords, savedUsedCount);
}

void restoreGameState(int savedPlayerScore, int savedBotScore, int savedUsedCount, char savedUsedWords[1000][30]) {
    playerScore = savedPlayerScore;
    botScore = savedBotScore;
    restoreUsedWords(savedUsedWords, savedUsedCount);
}

void placeRandomLetter() {
    int emptyCells[25][2];
    int emptyCount = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter == 0 && hasAdjacentLetter(i, j)) {
                emptyCells[emptyCount][0] = i;
                emptyCells[emptyCount][1] = j;
                emptyCount++;
            }
        }
    }
    if (emptyCount > 0) {
        srand((unsigned int)time(NULL));
        int cellIdx = rand() % emptyCount;
        int letterIdx = rand() % RUSSIAN_LETTERS_COUNT;
        int row = emptyCells[cellIdx][0];
        int col = emptyCells[cellIdx][1];
        unsigned char letter = allRussianLetters[letterIdx];
        printf("Bot places random letter '%c' at (%d,%d)\n", letter, row + 1, col + 1);
        placeRussianLetter(row, col, letter);
        checkAndAddWordDirect(row, col, 2);
    }
    else {
        printf("Bot has no valid moves at all!\n");
    }
}

int evaluateMove(int row, int col, unsigned char letter, int player, char* bestWord) {
    int cached = getCachedPoints(row, col, letter, player, bestWord);
    if (cached >= 0) return cached;

    unsigned char oldLetter = board[row][col].letter;
    int oldOwner = board[row][col].owner;
    board[row][col].letter = letter;
    board[row][col].owner = player;

    int savedPlayerScore = playerScore;
    int savedBotScore = botScore;

    char tempBestWord[30] = "";
    int bestPoints = 0;

    BFSStateEval queue[MAX_QUEUE_EVAL];
    int front = 0, rear = 0;
    queue[rear].row = row;
    queue[rear].col = col;
    queue[rear].word[0] = board[row][col].letter;
    queue[rear].word[1] = '\0';
    queue[rear].len = 1;
    memset(queue[rear].visited, 0, sizeof(queue[rear].visited));
    queue[rear].visited[row][col] = 1;
    rear++;

    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };

    while (front < rear) {
        BFSStateEval curr = queue[front++];
        if (curr.len >= 3 && curr.len <= 8) {
            if (isWordInDictionary(curr.word) && !isWordUsedGlobal(curr.word)) {
                if (curr.len > bestPoints) {
                    bestPoints = curr.len;
                    strcpy(tempBestWord, curr.word);
                }
            }
            else {
                char rev[30];
                strcpy(rev, curr.word);
                reverseStringLocal(rev);
                if (isWordInDictionary(rev) && !isWordUsedGlobal(rev)) {
                    if (curr.len > bestPoints) {
                        bestPoints = curr.len;
                        strcpy(tempBestWord, rev);
                    }
                }
            }
        }
        if (curr.len >= 8) continue;
        for (int i = 0; i < 4; i++) {
            int nr = curr.row + dr[i];
            int nc = curr.col + dc[i];
            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                if (board[nr][nc].letter != 0 && !curr.visited[nr][nc]) {
                    if (rear < MAX_QUEUE_EVAL) {
                        queue[rear].row = nr;
                        queue[rear].col = nc;
                        strcpy(queue[rear].word, curr.word);
                        queue[rear].word[curr.len] = board[nr][nc].letter;
                        queue[rear].word[curr.len + 1] = '\0';
                        queue[rear].len = curr.len + 1;
                        memcpy(queue[rear].visited, curr.visited, sizeof(curr.visited));
                        queue[rear].visited[nr][nc] = 1;
                        rear++;
                    }
                }
            }
        }
    }

    board[row][col].letter = oldLetter;
    board[row][col].owner = oldOwner;
    playerScore = savedPlayerScore;
    botScore = savedBotScore;

    addToCache(row, col, letter, player, bestPoints, tempBestWord);
    if (bestWord && bestPoints > 0) strcpy(bestWord, tempBestWord);
    return bestPoints;
}

int findAllMoves(BotMove* moves, int maxMoves, int useAllLetters) {
    int moveCount = 0;
    const unsigned char* letters = useAllLetters ? allRussianLetters : frequentLetters;
    int letterCount = useAllLetters ? RUSSIAN_LETTERS_COUNT : FREQUENT_LETTERS_COUNT;
    for (int i = 0; i < BOARD_SIZE && moveCount < maxMoves; i++) {
        for (int j = 0; j < BOARD_SIZE && moveCount < maxMoves; j++) {
            if (board[i][j].letter == 0 && hasAdjacentLetter(i, j)) {
                for (int k = 0; k < letterCount && moveCount < maxMoves; k++) {
                    unsigned char letter = letters[k];
                    char bestWord[30] = "";
                    int points = evaluateMove(i, j, letter, 2, bestWord);
                    if (points > 0) {
                        moves[moveCount].row = i;
                        moves[moveCount].col = j;
                        moves[moveCount].letter = letter;
                        moves[moveCount].points = points;
                        strcpy(moves[moveCount].word, bestWord);
                        moveCount++;
                    }
                }
            }
        }
    }
    return moveCount;
}

void botMakeMoveEasy() {
    BotMove winningMoves[500];
    int winCount = findAllMoves(winningMoves, 500, 1);
    srand((unsigned int)time(NULL));
    int r = rand() % 100;
    if (winCount > 0 && r < 30) {
        int idx = rand() % winCount;
        BotMove m = winningMoves[idx];
        printf("Bot (Easy) places '%c' at (%d,%d) for %d points (word: %s)\n",
            m.letter, m.row + 1, m.col + 1, m.points, m.word);
        placeRussianLetter(m.row, m.col, m.letter);
        checkAndAddWordDirect(m.row, m.col, 2);
    }
    else {
        placeRandomLetter();
    }
}

void botMakeMoveMedium() {
    BotMove bestMove;
    bestMove.points = -1;
    bestMove.row = -1;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter == 0 && hasAdjacentLetter(i, j)) {
                for (int k = 0; k < RUSSIAN_LETTERS_COUNT; k++) {
                    unsigned char letter = allRussianLetters[k];
                    char bestWord[30] = "";
                    int points = evaluateMove(i, j, letter, 2, bestWord);
                    if (points > bestMove.points) {
                        bestMove.points = points;
                        bestMove.row = i;
                        bestMove.col = j;
                        bestMove.letter = letter;
                        strcpy(bestMove.word, bestWord);
                    }
                }
            }
        }
    }
    if (bestMove.row != -1) {
        printf("Bot (Medium) places '%c' at (%d,%d) for %d points (word: %s)\n",
            bestMove.letter, bestMove.row + 1, bestMove.col + 1, bestMove.points, bestMove.word);
        placeRussianLetter(bestMove.row, bestMove.col, bestMove.letter);
        checkAndAddWordDirect(bestMove.row, bestMove.col, 2);
    }
    else {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j].letter == 0 && hasAdjacentLetter(i, j)) {
                    for (int k = 0; k < RUSSIAN_LETTERS_COUNT; k++) {
                        unsigned char letter = allRussianLetters[k];
                        int points = evaluateMove(i, j, letter, 2, NULL);
                        if (points > bestMove.points) {
                            bestMove.points = points;
                            bestMove.row = i;
                            bestMove.col = j;
                            bestMove.letter = letter;
                        }
                    }
                }
            }
        }
        if (bestMove.row != -1) {
            printf("Bot (Medium) places '%c' at (%d,%d) (no winning moves, %d points)\n",
                bestMove.letter, bestMove.row + 1, bestMove.col + 1, bestMove.points);
            placeRussianLetter(bestMove.row, bestMove.col, bestMove.letter);
            checkAndAddWordDirect(bestMove.row, bestMove.col, 2);
        }
        else {
            printf("Bot has no valid moves!\n");
        }
    }
}

int evaluateMoveHard(int row, int col, unsigned char letter, int player) {
    unsigned char oldLetter = board[row][col].letter;
    int oldOwner = board[row][col].owner;

    board[row][col].letter = letter;
    board[row][col].owner = player;

    int bestPoints = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter == 0) continue;

            BFSStateEval queue[5000];
            int front = 0, rear = 0;

            queue[rear].row = i;
            queue[rear].col = j;
            queue[rear].word[0] = board[i][j].letter;
            queue[rear].word[1] = '\0';
            queue[rear].len = 1;
            memset(queue[rear].visited, 0, sizeof(queue[rear].visited));
            queue[rear].visited[i][j] = 1;
            rear++;

            int dr[] = { -1, 1, 0, 0 };
            int dc[] = { 0, 0, -1, 1 };

            while (front < rear) {
                BFSStateEval curr = queue[front++];

                if (curr.len >= 3 && curr.len <= 8) {
                    if (curr.visited[row][col]) {
                        if (isWordInDictionary(curr.word) && !isWordUsedGlobal(curr.word)) {
                            if (curr.len > bestPoints) bestPoints = curr.len;
                        }
                        else {
                            char rev[30];
                            strcpy(rev, curr.word);
                            reverseStringLocal(rev);
                            if (isWordInDictionary(rev) && !isWordUsedGlobal(rev)) {
                                if (curr.len > bestPoints) bestPoints = curr.len;
                            }
                        }
                    }
                }

                if (curr.len >= 8) continue;

                for (int d = 0; d < 4; d++) {
                    int nr = curr.row + dr[d];
                    int nc = curr.col + dc[d];
                    if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                        if (board[nr][nc].letter != 0 && !curr.visited[nr][nc]) {
                            if (rear < 4999) {
                                queue[rear].row = nr;
                                queue[rear].col = nc;
                                strcpy(queue[rear].word, curr.word);
                                queue[rear].word[curr.len] = board[nr][nc].letter;
                                queue[rear].word[curr.len + 1] = '\0';
                                queue[rear].len = curr.len + 1;
                                memcpy(queue[rear].visited, curr.visited, sizeof(curr.visited));
                                queue[rear].visited[nr][nc] = 1;
                                rear++;
                            }
                        }
                    }
                }
            }
        }
    }

    board[row][col].letter = oldLetter;
    board[row][col].owner = oldOwner;

    return bestPoints;
}

void botMakeMoveHard() {
    BotMove bestMove;
    bestMove.points = -1;
    bestMove.row = -1;
    bestMove.col = -1;
    bestMove.letter = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter == 0 && hasAdjacentLetter(i, j)) {
                for (int k = 0; k < RUSSIAN_LETTERS_COUNT; k++) {
                    unsigned char letter = allRussianLetters[k];
                    int points = evaluateMoveHard(i, j, letter, 2);

                    if (points > bestMove.points) {
                        bestMove.points = points;
                        bestMove.row = i;
                        bestMove.col = j;
                        bestMove.letter = letter;
                    }
                }
            }
        }
    }

    if (bestMove.row != -1) {
        printf("Bot (Hard) places '%c' at (%d,%d) for %d points\n",
            bestMove.letter, bestMove.row + 1, bestMove.col + 1, bestMove.points);
        placeRussianLetter(bestMove.row, bestMove.col, bestMove.letter);
        checkAndAddWordDirect(bestMove.row, bestMove.col, 2);
    }
    else {
        printf("Bot (Hard) has no valid moves!\n");
    }
}

void botMakeMove() {
    extern enum BotDifficulty botDifficulty;
    switch (botDifficulty) {
    case DIFFICULTY_EASY:
        botMakeMoveEasy();
        break;
    case DIFFICULTY_MEDIUM:
        botMakeMoveMedium();
        break;
    case DIFFICULTY_HARD:
        botMakeMoveHard();
        break;
    }
}