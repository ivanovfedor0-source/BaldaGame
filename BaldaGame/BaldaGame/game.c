#define _CRT_SECURE_NO_WARNINGS
#include "game.h"
#include "dictionary.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <GLFW/glfw3.h>
#include "record.h"

#define MAX_QUEUE 5000

struct Cell board[BOARD_SIZE][BOARD_SIZE];
int currentPlayer = 1;
int playerScore = 0;
int botScore = 0;
int selectedRow = -1;
int selectedCol = -1;
char currentInputLetter = '\0';

char playerName[20] = "Player";
int askingName = 0;
char nameInput[20] = "";
int nameInputLen = 0;

struct PlayerWords player1Words = { .count = 0 };
struct PlayerWords player2Words = { .count = 0 };

static char usedWords[1000][30];
static int usedWordsCount = 0;

typedef struct {
    int row;
    int col;
    char word[30];
    int len;
    int visited[BOARD_SIZE][BOARD_SIZE];
} BFSState;

static const unsigned char rusKeyMap[] = {
    /* A */ 0xD4,  // Ф
    /* B */ 0xC8,  // И
    /* C */ 0xD1,  // С
    /* D */ 0xC2,  // В
    /* E */ 0xD3,  // У
    /* F */ 0xC0,  // А
    /* G */ 0xCF,  // П
    /* H */ 0xD0,  // Р
    /* I */ 0xD8,  // Ш
    /* J */ 0xCE,  // О
    /* K */ 0xCB,  // Л
    /* L */ 0xC4,  // Д
    /* M */ 0xDC,  // Ь
    /* N */ 0xD2,  // Т
    /* O */ 0xD9,  // Щ
    /* P */ 0xC7,  // З
    /* Q */ 0xC9,  // Й
    /* R */ 0xCA,  // К
    /* S */ 0xDB,  // Ы
    /* T */ 0xC5,  // Е
    /* U */ 0xC3,  // Г
    /* V */ 0xCC,  // М
    /* W */ 0xD6,  // Ц
    /* X */ 0xD7,  // Ч
    /* Y */ 0xCD,  // Н
    /* Z */ 0xDF,  // Я
};

unsigned char getRussianLetter(int key) {
    switch (key) {
    case GLFW_KEY_COMMA:        return 0xC1;  // Б
    case GLFW_KEY_PERIOD:       return 0xDE;  // Ю
    case GLFW_KEY_SEMICOLON:    return 0xC6;  // Ж
    case GLFW_KEY_APOSTROPHE:   return 0xDD;  // Э
    case GLFW_KEY_LEFT_BRACKET: return 0xD5;  // Х
    case GLFW_KEY_RIGHT_BRACKET:return 0xDA;  // Ъ
    case GLFW_KEY_GRAVE_ACCENT: return 0xA8;  // Ё
    }

    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        return rusKeyMap[key - GLFW_KEY_A];
    }
    return 0;
}

void placeRussianLetter(int row, int col, unsigned char letter) {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        if (board[row][col].letter == 0) {
            board[row][col].letter = letter;
            board[row][col].owner = currentPlayer;
        }
    }
}

int isWordUsed(const char* word) {
    for (int i = 0; i < usedWordsCount; i++) {
        if (strcmp(usedWords[i], word) == 0) return 1;
    }
    return 0;
}

void addUsedWord(const char* word) {
    if (usedWordsCount < 1000) {
        strcpy(usedWords[usedWordsCount], word);
        usedWordsCount++;
    }
}

void addPoints(int points, int player) {
    if (player == 1) {
        playerScore += points;
    }
    else {
        botScore += points;
    }
}

void addWordToPlayer(int player, const char* word, int points) {
    struct PlayerWords* pw;
    if (player == 1) {
        pw = &player1Words;
    }
    else {
        pw = &player2Words;
    }

    if (pw->count < MAX_PLAYER_WORDS) {
        strcpy(pw->words[pw->count], word);
        pw->scores[pw->count] = points;
        pw->count++;
    }
}

void clearPlayerWords() {
    player1Words.count = 0;
    player2Words.count = 0;
}

void reverseString(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}
int checkAndAddWordDirect(int row, int col, int player) {
    int totalPoints = 0;
    int bestPoints = 0;
    char bestWord[30] = "";

    printf("\n=== Checking words containing cell (%d,%d) ===\n", row, col);

    printf("Current board:\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter == 0) printf(" . ");
            else printf(" %c ", board[i][j].letter);
        }
        printf("\n");
    }

    printf("\n--- Found candidates ---\n");

    // Запускаем BFS от КАЖДОЙ клетки с буквой
    for (int startRow = 0; startRow < BOARD_SIZE; startRow++) {
        for (int startCol = 0; startCol < BOARD_SIZE; startCol++) {
            if (board[startRow][startCol].letter == 0) continue;

            BFSState queue[MAX_QUEUE];
            int front = 0, rear = 0;

            queue[rear].row = startRow;
            queue[rear].col = startCol;
            queue[rear].word[0] = board[startRow][startCol].letter;
            queue[rear].word[1] = '\0';
            queue[rear].len = 1;
            memset(queue[rear].visited, 0, sizeof(queue[rear].visited));
            queue[rear].visited[startRow][startCol] = 1;
            rear++;

            while (front < rear) {
                BFSState current = queue[front];
                front++;

                if (current.len >= 3 && current.len <= 8) {
                    if (current.visited[row][col]) {
                        char wordCopy[30];
                        strcpy(wordCopy, current.word);

                        if (isWordInDictionary(wordCopy) && !isWordUsed(wordCopy)) {
                            printf("  Candidate: '%s' (len=%d)\n", wordCopy, current.len);
                            if (current.len > bestPoints) {
                                bestPoints = current.len;
                                strcpy(bestWord, wordCopy);
                            }
                        }
                        else {
                            char reversed[30];
                            strcpy(reversed, wordCopy);
                            reverseString(reversed);
                            if (isWordInDictionary(reversed) && !isWordUsed(reversed)) {
                                printf("  Candidate (reversed): '%s' -> '%s' (len=%d)\n", wordCopy, reversed, current.len);
                                if (current.len > bestPoints) {
                                    bestPoints = current.len;
                                    strcpy(bestWord, reversed);
                                }
                            }
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
                            if (rear < MAX_QUEUE) {
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
        }
    }

    printf("--- End of candidates ---\n");

    if (bestPoints > 0) {
        addPoints(bestPoints, player);
        addUsedWord(bestWord);
        addWordToPlayer(player, bestWord, bestPoints);
        totalPoints = bestPoints;
        printf(">>> BEST WORD: '%s' +%d points\n", bestWord, bestPoints);
    }
    else {
        printf(">>> No valid words found!\n");
    }

    printf(">>> Total +%d points!\n", totalPoints);

    // ========== ПРОВЕРКА ОКОНЧАНИЯ ИГРЫ ==========
    if (isGameOver()) {
        printf("\n========================================\n");
        printf("GAME OVER! No empty cells left.\n");
        printf("========================================\n");

        if (gameMode == MODE_PVE && playerScore > botScore) {
            if (isHighScore(playerScore, botDifficulty)) {
                addRecord(playerName, playerScore, botDifficulty);
                printf("New record for %s difficulty! %s scored %d points\n",
                    botDifficulty == DIFFICULTY_EASY ? "Easy" :
                    botDifficulty == DIFFICULTY_MEDIUM ? "Medium" : "Hard",
                    playerName, playerScore);
            }
        }

        gameState = STATE_GAME_OVER;
    }

    return totalPoints;
}

int isWordUsedGlobal(const char* word) {
    for (int i = 0; i < usedWordsCount; i++) {
        if (strcmp(usedWords[i], word) == 0) {
            return 1;
        }
    }
    return 0;
}

void addUsedWordGlobal(const char* word) {
    if (usedWordsCount < 1000) {
        strcpy(usedWords[usedWordsCount], word);
        usedWordsCount++;
    }
}

int getUsedWordsCount(void) {
    return usedWordsCount;
}

void clearUsedWords(void) {
    usedWordsCount = 0;
}


void copyUsedWords(char dest[1000][30], int* destCount) {
    *destCount = usedWordsCount;
    for (int i = 0; i < usedWordsCount; i++) {
        strcpy(dest[i], usedWords[i]);
    }
}

void restoreUsedWords(char src[1000][30], int srcCount) {
    usedWordsCount = srcCount;
    for (int i = 0; i < srcCount; i++) {
        strcpy(usedWords[i], src[i]);
    }
}

void initGame() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j].letter = 0;
            board[i][j].owner = 0;
            board[i][j].isFixed = 0;
        }
    }

    unsigned char startWord[] = { 0xC1, 0xC0, 0xCB, 0xC4, 0xC0 };
    for (int j = 0; j < 5; j++) {
        board[2][j].letter = startWord[j];
        board[2][j].owner = 0;
        board[2][j].isFixed = 1;
    }

    currentPlayer = 1;
    playerScore = 0;
    botScore = 0;
    selectedRow = -1;
    selectedCol = -1;
    currentInputLetter = '\0';
    usedWordsCount = 0;
    clearPlayerWords();

    addUsedWord("БАЛДА");
    printf("Added starting word 'БАЛДА' to used words list\n");
}

int isCellEmpty(int row, int col) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return 0;
    return (board[row][col].letter == 0);
}

void clearSelection() {
    selectedRow = -1;
    selectedCol = -1;
    currentInputLetter = '\0';
}

// Проверка, заполнено ли всё поле
int isGameOver() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter == 0) {
                return 0; // Есть хотя бы одна пустая клетка — игра продолжается
            }
        }
    }
    return 1; // Нет пустых клеток — игра окончена
}