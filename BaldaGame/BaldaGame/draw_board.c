#include "draw_board.h"
#include "game.h"
#include "menu.h"
#include "font.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <windows.h>

static float boardStartX = -0.55f;
static float boardStartY = -0.55f;
static float cellSize = 0.22f;

// Маленький шрифт для списка слов
static GLuint smallFontBase = 0;

void initSmallFont() {
    HDC hdc = wglGetCurrentDC();
    smallFontBase = glGenLists(256);

    HFONT hFont = CreateFontA(
        -16, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        RUSSIAN_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        "Arial"
    );

    SelectObject(hdc, hFont);
    wglUseFontBitmaps(hdc, 0, 255, smallFontBase);
    DeleteObject(hFont);
}

void drawSmallRussianString(float x, float y, const char* str) {
    if (!str || !str[0]) return;

    glRasterPos2f(x, y);
    glPushAttrib(GL_LIST_BIT);
    glListBase(smallFontBase);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
}

void drawPlayerWords(float x, float y, struct PlayerWords* pw, const char* title, float r, float g, float b) {
    // Заголовок (обычный шрифт)
    glColor3f(r, g, b);
    drawRussianString(x, y, title);

    float lineY = y - 0.06f;  // Увеличен отступ после заголовка

    // Слова (маленький шрифт)
    for (int i = 0; i < pw->count && i < 20; i++) {
        char line[50];
        sprintf(line, "%s (%d)", pw->words[i], pw->scores[i]);
        glColor3f(0.85f, 0.85f, 0.85f);
        drawSmallRussianString(x, lineY, line);
        lineY -= 0.045f;  // Увеличен отступ между словами

        if (lineY < -0.85f) break;
    }

    // Общий счёт
    char total[50];
    int totalScore = 0;
    for (int i = 0; i < pw->count; i++) {
        totalScore += pw->scores[i];
    }
    sprintf(total, "Total: %d", totalScore);
    glColor3f(1.0f, 0.8f, 0.0f);
    drawRussianString(x, lineY - 0.06f, total);
}

void drawGameBoard() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);

    // Рисуем сетку
    glBegin(GL_LINES);
    for (int i = 0; i <= BOARD_SIZE; i++) {
        float x = boardStartX + i * cellSize;
        glVertex2f(x, boardStartY);
        glVertex2f(x, boardStartY + BOARD_SIZE * cellSize);
    }
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
                float centerX = boardStartX + j * cellSize + cellSize / 2;
                float centerY = boardStartY + i * cellSize + cellSize / 2;
                float textOffset = 0.035f;

                char letterStr[2] = { (char)board[i][j].letter, '\0' };

                if (selectedRow == i && selectedCol == j) {
                    glColor3f(1.0f, 1.0f, 0.0f);  // Жёлтая буква
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

    // Рисуем рамку вокруг выбранной клетки (ПОВЕРХ буквы)
    if (selectedRow != -1 && selectedCol != -1) {
        float x = boardStartX + selectedCol * cellSize;
        float y = boardStartY + selectedRow * cellSize;

        glColor3f(1.0f, 1.0f, 0.0f);  // Жёлтая рамка
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 0.02f, y + 0.02f);
        glVertex2f(x + cellSize - 0.02f, y + 0.02f);
        glVertex2f(x + cellSize - 0.02f, y + cellSize - 0.02f);
        glVertex2f(x + 0.02f, y + cellSize - 0.02f);
        glEnd();
        glLineWidth(1.0f);
    }
}

void drawGameUI() {
    if (gameMode == MODE_PVP) {
        // Списки дальше от поля
        drawPlayerWords(-1.15f, 0.75f, &player1Words, "Player 1:", 0.3f, 0.8f, 0.3f);
        drawPlayerWords(0.80f, 0.75f, &player2Words, "Player 2:", 0.9f, 0.5f, 0.3f);
    }
    else {
        char title1[30];
        sprintf(title1, "%s:", playerName);
        drawPlayerWords(-1.15f, 0.75f, &player1Words, title1, 0.3f, 0.8f, 0.3f);
        drawPlayerWords(0.80f, 0.75f, &player2Words, "Bot:", 0.9f, 0.3f, 0.3f);
    }

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