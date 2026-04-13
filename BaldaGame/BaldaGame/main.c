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

GLFWwindow* window;

// Преобразование координат мыши в клетку поля
void getCellFromMouse(double xpos, double ypos, int* row, int* col) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Нормализация координат мыши в диапазон [0,1]
    float nx = (float)xpos / width;
    float ny = 1.0f - (float)ypos / height;

    // Преобразование в координаты OpenGL с учётом пропорций
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

    // Координаты поля
    float boardStartX = -0.55f;
    float boardStartY = -0.55f;
    float cellSize = 0.22f;

    // Проверка попадания в поле
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

// Обработка клика мыши
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

// Обработка клавиш
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Глобальные клавиши F1-F4
        switch (key) {
        case GLFW_KEY_F1:
            gameState = STATE_MENU;
            return;
        case GLFW_KEY_F2:
            gameState = STATE_RECORDS;
            return;
        case GLFW_KEY_F3:
            gameState = STATE_HELP;
            return;
        case GLFW_KEY_F4:
            glfwSetWindowShouldClose(window, 1);
            return;
        }

        // Ввод имени
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
            }
            return;
        }

        // Управление в меню
        if (gameState == STATE_MENU) {
            switch (key) {
            case GLFW_KEY_UP:
                menuUp();
                break;
            case GLFW_KEY_DOWN:
                menuDown();
                break;
            case GLFW_KEY_ENTER:
                menuSelect();
                break;
            }
            return;
        }

        // Управление в игре
        if (gameState == STATE_GAME && !askingName) {
            if (selectedRow != -1 && selectedCol != -1) {
                unsigned char rusLetter = getRussianLetter(key);
                if (rusLetter != 0) {
                    if (gameMode == MODE_PVP || (gameMode == MODE_PVE && currentPlayer == 1)) {
                        // ОТЛАДКА: выводим информацию о вставляемой букве
                        printf("\n========================================\n");
                        printf("PLAYER %d puts letter at (%d, %d)\n", currentPlayer, selectedRow + 1, selectedCol + 1);
                        printf("Letter code: 0x%02X ('%c')\n", rusLetter, rusLetter);
                        printf("========================================\n");

                        placeRussianLetter(selectedRow, selectedCol, rusLetter);

                        // Сохраняем координаты до очистки
                        int row = selectedRow;
                        int col = selectedCol;

                        printf("DEBUG: Saving coordinates (%d,%d) before clearSelection\n", row, col);

                        clearSelection();

                        printf("DEBUG: Calling checkAndAddWordDirect with (%d,%d)\n", row, col);

                        // Проверяем слова
                        int points = checkAndAddWordDirect(row, col, currentPlayer);

                        if (points == 0) {
                            printf(">>> No valid words found!\n");
                        }

                        // Смена хода
                        if (gameMode == MODE_PVP) {
                            currentPlayer = (currentPlayer == 1) ? 2 : 1;
                            printf("Now Player %d turn\n", currentPlayer);
                        }
                        else {
                            currentPlayer = 2;
                            printf("Bot turn\n");
                        }
                    }
                }
            }
        }
    }
}

// Отрисовка кадра
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

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
    }

    glfwSwapBuffers(window);
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

    // Настройка координат
    framebufferSizeCallback(window, 800, 800);

    // Инициализация шрифтов
    initFont();          // Для английского меню
    initRussianFont();   // Для русских букв в игре

    // Загрузка словаря
    loadDictionary("C:/BaldaCoursework/data/dictionary.txt");

    // Инициализация состояния
    initMenu();
    initGame();

    printf("Program started. Use arrows and Enter for menu.\n");
    printf("F1 - menu, F2 - records, F3 - help, F4 - exit\n");
    printf("Dictionary loaded. Words must be 3-8 letters.\n");

    while (!glfwWindowShouldClose(window)) {
        display();
        glfwPollEvents();
    }

    // Освобождение памяти словаря
    freeDictionary();

    glfwDestroyWindow(window);
    glfwTerminate();
    printf("Program terminated.\n");
    return 0;
}