#define _CRT_SECURE_NO_WARNINGS
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "menu.h"
#include "graphics.h"
#include "game.h"
#include "draw_board.h"
#include "font.h"
#include "dictionary.h"
#include "bot.h"
#include "record.h"

GLFWwindow* window;

void getCellFromMouse(double xpos, double ypos, int* row, int* col) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float nx = (float)xpos / width;
    float ny = 1.0f - (float)ypos / height;

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    float aspect = (float)fbWidth / (float)fbHeight;

    float glX, glY;
    if (aspect > 1.0f) {
        glX = (nx - 0.5f) * 2.0f * aspect;
        glY = (ny - 0.5f) * 2.0f;
    }
    else {
        glX = (nx - 0.5f) * 2.0f;
        glY = (ny - 0.5f) * 2.0f / aspect;
    }

    float boardStartX = -0.55f;
    float boardStartY = -0.55f;
    float cellSize = 0.22f;

    if (glX >= boardStartX && glX <= boardStartX + BOARD_SIZE * cellSize &&
        glY >= boardStartY && glY <= boardStartY + BOARD_SIZE * cellSize) {
        *col = (int)((glX - boardStartX) / cellSize);
        *row = (int)((glY - boardStartY) / cellSize);

        if (*col < 0) *col = 0;
        if (*col >= BOARD_SIZE) *col = BOARD_SIZE - 1;
        if (*row < 0) *row = 0;
        if (*row >= BOARD_SIZE) *row = BOARD_SIZE - 1;
    }
    else {
        *row = -1;
        *col = -1;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (gameState == STATE_GAME && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int row, col;
        getCellFromMouse(xpos, ypos, &row, &col);

        if (row >= 0 && col >= 0 && isCellEmpty(row, col)) {
            selectedRow = row;
            selectedCol = col;
            currentInputLetter = '\0';
        }
    }
}


void displayWithHighlight(int highlight) {
    glClear(GL_COLOR_BUFFER_BIT);

    int savedRow = selectedRow;
    int savedCol = selectedCol;

    if (!highlight) {
        selectedRow = -1;
        selectedCol = -1;
    }

    switch (gameState) {
    case STATE_MENU:
        drawMenu();
        break;
    case STATE_HELP:
        drawHelp();
        break;
    case STATE_RECORDS:
        drawRecords();
        break;
    case STATE_GAME:
        if (askingName) {
            drawNameInput();
        }
        else {
            drawGameBoard();
            drawGameUI();
        }
        break;
    case STATE_GAME_OVER:
        drawGameOver();
        break;
    }

    selectedRow = savedRow;
    selectedCol = savedCol;

    glfwSwapBuffers(window);
}

void display() {
    displayWithHighlight(1);
}

void forceRedraw() {
    display();
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void botTurn() {
    printf("Bot turn\n");

    displayWithHighlight(0);
    glfwPollEvents();

    Sleep(100);

    botMakeMove();

    displayWithHighlight(0);
    glfwPollEvents();

    Sleep(100);

    if (isGameOver()) {
        gameState = STATE_GAME_OVER;
        display();
        glfwSwapBuffers(window);

        // Ждём нажатия пробела или Enter
        while (1) {
            glfwPollEvents();
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                break;
            }
            Sleep(50);
        }

        gameState = STATE_MENU;
        selectingMode = 0;
        selectingDifficulty = 0;
        selectedMenuItem = 0;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // ========== ГЛОБАЛЬНЫЕ КЛАВИШИ (РАБОТАЮТ ВЕЗДЕ) ==========
        switch (key) {
        case GLFW_KEY_F1:
            selectingMode = 0;
            selectingDifficulty = 0;
            selectedMenuItem = 0;
            gameState = STATE_MENU;
            printf("Returned to main menu\n");
            return;
        case GLFW_KEY_F2:
            gameState = STATE_RECORDS;
            loadRecordsForDisplay();
            return;
        case GLFW_KEY_F3:
            gameState = STATE_HELP;
            return;
        case GLFW_KEY_F4:
            glfwSetWindowShouldClose(window, 1);
            return;
        }

        // ========== УПРАВЛЕНИЕ В ТАБЛИЦЕ РЕКОРДОВ ==========
        if (gameState == STATE_RECORDS) {
            switch (key) {
            case GLFW_KEY_LEFT:
            {
                int newDiff = getRecordsDifficulty() - 1;
                if (newDiff < 0) newDiff = 2;
                setRecordsDifficulty(newDiff);
                loadRecordsForDisplay();
            }
            break;
            case GLFW_KEY_RIGHT:
            {
                int newDiff = getRecordsDifficulty() + 1;
                if (newDiff > 2) newDiff = 0;
                setRecordsDifficulty(newDiff);
                loadRecordsForDisplay();
            }
            break;
            }
            return;
        }

        // ========== ВВОД ИМЕНИ ==========
        if (gameState == STATE_GAME && askingName) {
            if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
                if (nameInputLen < 15) {
                    nameInput[nameInputLen++] = 'A' + (key - GLFW_KEY_A);
                    nameInput[nameInputLen] = '\0';
                }
            }
            else if (key == GLFW_KEY_BACKSPACE && nameInputLen > 0) {
                nameInput[--nameInputLen] = '\0';
            }
            else if (key == GLFW_KEY_ENTER && nameInputLen > 0) {
                strcpy(playerName, nameInput);
                askingName = 0;
                nameInputLen = 0;
                nameInput[0] = '\0';
                initGame();
                printf("Player name set to: %s\n", playerName);
            }
            else if (key == GLFW_KEY_LEFT) {
                // Возврат к выбору сложности
                askingName = 0;
                nameInputLen = 0;
                nameInput[0] = '\0';
                selectingMode = 0;
                selectingDifficulty = 1;
                selectedMenuItem = botDifficulty;
                gameState = STATE_MENU;
            }
            return;
        }

        // ========== УПРАВЛЕНИЕ В МЕНЮ ==========
        if (gameState == STATE_MENU) {
            switch (key) {
            case GLFW_KEY_UP:
                menuUp();
                break;
            case GLFW_KEY_DOWN:
                menuDown();
                break;
            case GLFW_KEY_LEFT:
                menuBack();  // Возврат в предыдущее меню
                break;
            case GLFW_KEY_ENTER:
                menuSelect();
                break;
            }
            return;
        }

        // ========== УПРАВЛЕНИЕ В ИГРЕ ==========
        if (gameState == STATE_GAME && !askingName) {
            if (selectedRow != -1 && selectedCol != -1) {
                unsigned char rusLetter = getRussianLetter(key);
                if (rusLetter != 0) {
                    if (gameMode == MODE_PVP || (gameMode == MODE_PVE && currentPlayer == 1)) {
                        printf("\n========================================\n");
                        printf("PLAYER %d puts letter at (%d, %d)\n", currentPlayer, selectedRow + 1, selectedCol + 1);
                        printf("Letter code: 0x%02X ('%c')\n", rusLetter, rusLetter);
                        printf("========================================\n");

                        placeRussianLetter(selectedRow, selectedCol, rusLetter);

                        int row = selectedRow;
                        int col = selectedCol;

                        clearSelection();

                        int points = checkAndAddWordDirect(row, col, currentPlayer);

                        if (points == 0) {
                            printf("No valid words found!\n");
                        }
                        // Принудительно показываем ход игрока
                        displayWithHighlight(1);
                        glfwSwapBuffers(window);
                        glfwPollEvents();
                        Sleep(80);

                        // Проверяем, заполнено ли поле
                        int gameOver = 1;
                        for (int i = 0; i < BOARD_SIZE; i++) {
                            for (int j = 0; j < BOARD_SIZE; j++) {
                                if (board[i][j].letter == 0) {
                                    gameOver = 0;
                                    break;
                                }
                            }
                        }

                        if (gameOver) {
                            // Показываем экран окончания
                            drawGameOver();
                            glfwSwapBuffers(window);

                            // Ждём нажатия пробела или Enter
                            while (1) {
                                glfwPollEvents();
                                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                                    glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                                    break;
                                }
                                Sleep(50);
                            }

                            gameState = STATE_MENU;
                            selectingMode = 0;
                            selectingDifficulty = 0;
                            selectedMenuItem = 0;
                            return;
                        }

                        if (gameMode == MODE_PVP) {
                            currentPlayer = (currentPlayer == 1) ? 2 : 1;
                            printf("Now Player %d turn\n", currentPlayer);
                        }
                        else {
                            currentPlayer = 2;
                            botTurn();

                            // После хода бота проверяем окончание игры
                            if (gameState == STATE_GAME_OVER) {
                                displayWithHighlight(1);
                                glfwSwapBuffers(window);

                                while (1) {
                                    glfwPollEvents();
                                    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                                        glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                                        break;
                                    }
                                    Sleep(50);
                                }

                                gameState = STATE_MENU;
                                selectingMode = 0;
                                selectingDifficulty = 0;
                                selectedMenuItem = 0;
                                return;
                            }

                            currentPlayer = 1;
                        }
                    }
                }
            }
        }
    }
}



int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    printf("Starting GLFW...\n");

    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    window = glfwCreateWindow(800, 800, "Balda Game", NULL, NULL);
    if (!window) {
        printf("Failed to create window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    framebufferSizeCallback(window, 800, 800);

    initFont();
    initRussianFont();
    initSmallFont();
    initTitleFont();
    initMenuItemsFont();

    loadDictionary("C:/BaldaCoursework/data/dictionary.txt");
    initRecords();
    initMenu();
    initGame();

    printf("Program started. Use arrows and Enter for menu.\n");
    printf("F1 - menu, F2 - records, F3 - help, F4 - exit\n");

    while (!glfwWindowShouldClose(window)) {
        display();
        glfwPollEvents();
    }

    freeDictionary();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}