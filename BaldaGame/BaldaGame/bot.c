#define _CRT_SECURE_NO_WARNINGS
#include "bot.h"
#include "game.h"
#include "dictionary.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Структура для хранения информации о возможном ходе
typedef struct {
    int row;
    int col;
    unsigned char letter;
    int points;
    char word[30];
} BotMove;

// Коды всех русских заглавных букв (33 буквы)
static const unsigned char allRussianLetters[] = {
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xA8  // Ё
};

// Самые частые буквы в русском языке (для оптимизации)
static const unsigned char frequentLetters[] = {
    0xC0,  // А
    0xC5,  // Е
    0xC8,  // И
    0xCE,  // О
    0xD3,  // У
    0xD2,  // Т
    0xCD,  // Н
    0xD1,  // С
    0xD0,  // Р
    0xC2   // В
};

#define RUSSIAN_LETTERS_COUNT 33
#define FREQUENT_LETTERS_COUNT 10
#define MAX_QUEUE_EVAL 5000
#define MAX_CACHE_SIZE 2000

// Структура для кэша результатов evaluateMove
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

// Вспомогательная структура для BFS
typedef struct {
    int row;
    int col;
    char word[30];
    int len;
    int visited[BOARD_SIZE][BOARD_SIZE];
} BFSStateEval;

// Проверка, есть ли рядом с клеткой хотя бы одна буква
int hasAdjacentLetter(int row, int col) {
    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };

    for (int i = 0; i < 4; i++) {
        int newRow = row + dr[i];
        int newCol = col + dc[i];

        if (newRow >= 0 && newRow < BOARD_SIZE && newCol >= 0 && newCol < BOARD_SIZE) {
            if (board[newRow][newCol].letter != 0) {
                return 1;
            }
        }
    }
    return 0;
}

// Переворот строки
void reverseStringLocal(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

// Получение результата из кэша
int getCachedPoints(int row, int col, unsigned char letter, int player, char* word) {
    for (int i = 0; i < cacheSize; i++) {
        if (moveCache[i].row == row && moveCache[i].col == col &&
            moveCache[i].letter == letter && moveCache[i].player == player) {
            if (word != NULL) {
                strcpy(word, moveCache[i].word);
            }
            return moveCache[i].points;
        }
    }
    return -1;
}

// Добавление результата в кэш
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

// Очистка кэша
void clearCache() {
    cacheSize = 0;
}

// Поставить случайную букву в случайную пустую клетку
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

        printf("Bot places random letter '%c' at (%d,%d) (no winning moves)\n",
            letter, row + 1, col + 1);
        placeRussianLetter(row, col, letter);
        checkAndAddWordDirect(row, col, 2);
    }
    else {
        printf("Bot has no valid moves at all!\n");
    }
}

// Оценка хода (сколько очков принесёт буква на клетке)
int evaluateMove(int row, int col, unsigned char letter, int player, char* bestWord) {
    int cached = getCachedPoints(row, col, letter, player, bestWord);
    if (cached >= 0) {
        return cached;
    }

    unsigned char originalLetter = board[row][col].letter;
    int originalOwner = board[row][col].owner;

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

    while (front < rear) {
        BFSStateEval current = queue[front];
        front++;

        if (current.len >= 3 && current.len <= 8) {
            char wordCopy[30];
            strcpy(wordCopy, current.word);

            if (isWordInDictionary(wordCopy)) {
                if (!isWordUsedGlobal(wordCopy) && current.len > bestPoints) {
                    bestPoints = current.len;
                    strcpy(tempBestWord, wordCopy);
                }
            }

            char reversed[30];
            strcpy(reversed, wordCopy);
            reverseStringLocal(reversed);
            if (isWordInDictionary(reversed)) {
                if (!isWordUsedGlobal(reversed) && current.len > bestPoints) {
                    bestPoints = current.len;
                    strcpy(tempBestWord, reversed);
                }
            }
        }

        if (current.len >= 8) continue;

        int dr[] = { -1, 1, 0, 0 };
        int dc[] = { 0, 0, -1, 1 };

        for (int i = 0; i < 4; i++) {
            int newRow = current.row + dr[i];
            int newCol = current.col + dc[i];

            if (newRow >= 0 && newRow < BOARD_SIZE && newCol >= 0 && newCol < BOARD_SIZE) {
                if (board[newRow][newCol].letter != 0 && !current.visited[newRow][newCol]) {
                    if (rear < MAX_QUEUE_EVAL) {
                        queue[rear].row = newRow;
                        queue[rear].col = newCol;
                        strcpy(queue[rear].word, current.word);
                        queue[rear].word[current.len] = board[newRow][newCol].letter;
                        queue[rear].word[current.len + 1] = '\0';
                        queue[rear].len = current.len + 1;
                        memcpy(queue[rear].visited, current.visited, sizeof(current.visited));
                        queue[rear].visited[newRow][newCol] = 1;
                        rear++;
                    }
                }
            }
        }
    }

    board[row][col].letter = originalLetter;
    board[row][col].owner = originalOwner;
    playerScore = savedPlayerScore;
    botScore = savedBotScore;

    addToCache(row, col, letter, player, bestPoints, tempBestWord);

    if (bestWord != NULL && bestPoints > 0) {
        strcpy(bestWord, tempBestWord);
    }

    return bestPoints;
}

// Поиск всех возможных выигрышных ходов
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

// Easy бот: 70% случайная буква, 30% выигрышный ход
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

// Medium бот: жадный, выбирает ход с максимальными очками
void botMakeMoveMedium() {
    BotMove bestMove;
    bestMove.points = -1;
    bestMove.row = -1;
    bestMove.col = -1;
    bestMove.letter = 0;

    // Перебираем только клетки, рядом с которыми есть буквы
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter == 0 && hasAdjacentLetter(i, j)) {
                for (int k = 0; k < RUSSIAN_LETTERS_COUNT; k++) {
                    unsigned char letter = allRussianLetters[k];
                    char bestWord[30] = "";
                    int points = evaluateMove(i, j, letter, 2, bestWord);

                    // Игнорируем ходы с 0 очков
                    if (points > 0 && points > bestMove.points) {
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
        // Нет выигрышных ходов — ставим случайную букву в лучшую доступную клетку
        // (перебираем все клетки, даже с 0 очков)
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

// Hard бот: минимакс (учитывает лучший ответ игрока)
void botMakeMoveHard() {
    BotMove bestMove;
    bestMove.points = -999999;
    bestMove.row = -1;

    BotMove botMoves[500];
    int botMoveCount = findAllMoves(botMoves, 500, 1);

    if (botMoveCount == 0) {
        placeRandomLetter();
        return;
    }

    // Сортируем ходы по очкам
    for (int i = 0; i < botMoveCount - 1; i++) {
        for (int j = i + 1; j < botMoveCount; j++) {
            if (botMoves[i].points < botMoves[j].points) {
                BotMove temp = botMoves[i];
                botMoves[i] = botMoves[j];
                botMoves[j] = temp;
            }
        }
    }

    int topMoves = botMoveCount < 10 ? botMoveCount : 10;
    printf("Hard bot: evaluating top %d moves (total %d possible)\n", topMoves, botMoveCount);

    for (int m = 0; m < topMoves; m++) {
        BotMove move = botMoves[m];

        if (move.points >= 10) {
            bestMove = move;
            break;
        }

        unsigned char originalLetter = board[move.row][move.col].letter;
        int originalOwner = board[move.row][move.col].owner;
        board[move.row][move.col].letter = move.letter;
        board[move.row][move.col].owner = 2;

        int bestPlayerResponse = 0;
        int checked = 0;

        for (int i = 0; i < BOARD_SIZE && checked < 50; i++) {
            for (int j = 0; j < BOARD_SIZE && checked < 50; j++) {
                if (board[i][j].letter == 0 && hasAdjacentLetter(i, j)) {
                    for (int k = 0; k < FREQUENT_LETTERS_COUNT && checked < 50; k++) {
                        unsigned char letter = frequentLetters[k];
                        int points = evaluateMove(i, j, letter, 1, NULL);
                        if (points > bestPlayerResponse) {
                            bestPlayerResponse = points;
                        }
                        checked++;
                    }
                }
            }
        }

        board[move.row][move.col].letter = originalLetter;
        board[move.row][move.col].owner = originalOwner;

        int score = move.points - bestPlayerResponse;

        if (score > bestMove.points) {
            bestMove.points = score;
            bestMove.row = move.row;
            bestMove.col = move.col;
            bestMove.letter = move.letter;
            strcpy(bestMove.word, move.word);
        }
    }

    if (bestMove.row != -1) {
        printf("Bot (Hard) places '%c' at (%d,%d) (score: %d, word: %s)\n",
            bestMove.letter, bestMove.row + 1, bestMove.col + 1, bestMove.points, bestMove.word);
        placeRussianLetter(bestMove.row, bestMove.col, bestMove.letter);
        checkAndAddWordDirect(bestMove.row, bestMove.col, 2);
    }
    else {
        botMakeMoveMedium();
    }
}

// Основная функция бота
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