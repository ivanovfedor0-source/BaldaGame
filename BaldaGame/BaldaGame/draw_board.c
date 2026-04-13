#include "draw_board.h"
#include "game.h"
#include "menu.h"
#include "font.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

static float boardStartX = -0.55f;
static float boardStartY = -0.55f;
static float cellSize = 0.22f;

void drawGameBoard() {
    // Рисуем сетку
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    // Вертикальные линии
    for (int i = 0; i <= BOARD_SIZE; i++) {
        float x = boardStartX + i * cellSize;
        glVertex2f(x, boardStartY);
        glVertex2f(x, boardStartY + BOARD_SIZE * cellSize);
    }
    // Горизонтальные линии
    for (int i = 0; i <= BOARD_SIZE; i++) {
        float y = boardStartY + i * cellSize;
        glVertex2f(boardStartX, y);
        glVertex2f(boardStartX + BOARD_SIZE * cellSize, y);
    }
    glEnd();

    // Рисуем буквы
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j].letter != 0) {
                // Центр клетки
                float centerX = boardStartX + j * cellSize + cellSize / 2;
                float centerY = boardStartY + i * cellSize + cellSize / 2;

                // Смещение для центрирования текста
                float textOffset = 0.035f;

                char letterStr[2] = { (char)board[i][j].letter, '\0' };

                // Цвета
                if (selectedRow == i && selectedCol == j) {
                    glColor3f(1.0f, 1.0f, 0.0f);  // Жёлтый (выбрана)
                }
                else if (board[i][j].owner == 1) {
                    glColor3f(0.3f, 0.8f, 0.3f);  // Зелёный (игрок 1)
                }
                else if (board[i][j].owner == 2) {
                    glColor3f(0.9f, 0.3f, 0.3f);  // Красный (игрок 2 / бот)
                }
                else {
                    glColor3f(1.0f, 1.0f, 1.0f);  // Белый
                }

                drawRussianString(centerX - textOffset, centerY - textOffset, letterStr);
            }
        }
    }
}

void drawGameUI() {
    char scoreText[100];

    if (gameMode == MODE_PVP) {
        sprintf(scoreText, "Player 1: %d    Player 2: %d", playerScore, botScore);
    }
    else {
        // PVE: отображаем имя игрока
        sprintf(scoreText, "%s: %d    Bot: %d", playerName, playerScore, botScore);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    drawRussianString(-0.85f, 0.85f, scoreText);

    if (selectedRow != -1 && selectedCol != -1) {
        char hint[50];
        sprintf(hint, "Selected: (%d,%d) Type a letter", selectedRow + 1, selectedCol + 1);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawRussianString(-0.85f, -0.85f, hint);
    }
    else {
        glColor3f(0.5f, 0.5f, 0.5f);
        drawRussianString(-0.85f, -0.85f, "Click on a cell to select it");
    }

    char turnText[50];
    if (gameMode == MODE_PVP) {
        if (currentPlayer == 1) {
            sprintf(turnText, "Player 1 turn");
            glColor3f(0.3f, 0.8f, 0.3f);
        }
        else {
            sprintf(turnText, "Player 2 turn");
            glColor3f(0.9f, 0.5f, 0.3f);
        }
    }
    else {
        if (currentPlayer == 1) {
            sprintf(turnText, "%s's turn", playerName);
            glColor3f(0.3f, 0.8f, 0.3f);
        }
        else {
            sprintf(turnText, "Bot turn");
            glColor3f(0.9f, 0.3f, 0.3f);
        }
    }
    drawRussianString(0.55f, 0.85f, turnText);
}