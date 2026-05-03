#include "graphics.h"
#include "menu.h"
#include "game.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "record.h"

static GLuint fontBase = 0;
extern int selectedMenuItem;
extern int selectingMode;
extern int selectingDifficulty;
extern char nameInput[20];
static GLuint titleFontBase = 0;
static GLuint menuItemsFontBase = 0;

void initFont() {
    HDC hdc = wglGetCurrentDC();
    fontBase = glGenLists(256);

    HFONT hFont = CreateFontA(
        33, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        "Arial"
    );

    SelectObject(hdc, hFont);
    wglUseFontBitmaps(hdc, 0, 255, fontBase);
    DeleteObject(hFont);
}
void initTitleFont() {
    HDC hdc = wglGetCurrentDC();
    titleFontBase = glGenLists(256);

    HFONT hFont = CreateFontA(
        72, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        "Arial"
    );

    SelectObject(hdc, hFont);
    wglUseFontBitmaps(hdc, 0, 255, titleFontBase);
    DeleteObject(hFont);
}
void drawTitleString(float x, float y, const char* str) {
    if (str == NULL || str[0] == '\0') return;
    glRasterPos2f(x, y);
    glPushAttrib(GL_LIST_BIT);
    glListBase(titleFontBase);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
}

void drawTitleCentered(float y, const char* str, float r, float g, float b) {
    glColor3f(r, g, b);
    float textWidth = strlen(str) * 0.055f;  // Ширина буквы в координатах
    drawTitleString(-textWidth / 2, y, str);
}


void drawString(float x, float y, const char* str) {
    if (str == NULL || str[0] == '\0') return;

    glRasterPos2f(x, y);
    glPushAttrib(GL_LIST_BIT);
    glListBase(fontBase);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
}

void drawCentered(float y, const char* str, float r, float g, float b) {
    glColor3f(r, g, b);
    float textWidth = strlen(str) * 0.028f;
    drawString(-textWidth / 2, y, str);
}

void drawMenuItem(float y, const char* str, int isSelected, float offsetX) {
    if (isSelected) {
        glColor3f(1.0f, 0.8f, 0.0f);
        drawString(offsetX - 0.08f, y, ">");

        glColor3f(1.0f, 1.0f, 1.0f);
        drawString(offsetX + 0.05f, y, str);
    }
    else {
        glColor3f(0.8f, 0.8f, 0.8f);
        drawString(offsetX, y, str);
    }
}


void initMenuItemsFont() {
    HDC hdc = wglGetCurrentDC();
    menuItemsFontBase = glGenLists(256);

    HFONT hFont = CreateFontA(
        44, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        "Arial"
    );

    SelectObject(hdc, hFont);
    wglUseFontBitmaps(hdc, 0, 255, menuItemsFontBase);
    DeleteObject(hFont);
}

void drawMenuItemString(float x, float y, const char* str) {
    if (str == NULL || str[0] == '\0') return;
    glRasterPos2f(x, y);
    glPushAttrib(GL_LIST_BIT);
    glListBase(menuItemsFontBase);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
}

void drawGameOver() {
    // Временно сохраняем текущий цвет фона
    GLfloat oldColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, oldColor);

    // Устанавливаем тёмный фон для экрана окончания
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Заголовок
    glColor3f(1.0f, 0.8f, 0.0f);
    drawCentered(0.75f, "GAME OVER", 1.0f, 0.8f, 0.0f);

    // Победитель
    char winnerText[50];
    char scoreText[50];

    if (gameMode == MODE_PVP) {
        if (playerScore > botScore) {
            sprintf(winnerText, "PLAYER 1 WINS!");
            glColor3f(0.3f, 0.8f, 0.3f);
        }
        else if (botScore > playerScore) {
            sprintf(winnerText, "PLAYER 2 WINS!");
            glColor3f(0.9f, 0.5f, 0.3f);
        }
        else {
            sprintf(winnerText, "DRAW!");
            glColor3f(0.7f, 0.7f, 0.7f);
        }
    }
    else {
        if (playerScore > botScore) {
            sprintf(winnerText, "%s WINS!", playerName);
            glColor3f(0.3f, 0.8f, 0.3f);
        }
        else if (botScore > playerScore) {
            sprintf(winnerText, "BOT WINS!");
            glColor3f(0.9f, 0.3f, 0.3f);
        }
        else {
            sprintf(winnerText, "DRAW!");
            glColor3f(0.7f, 0.7f, 0.7f);
        }
    }
    drawCentered(0.55f, winnerText, 1.0f, 1.0f, 1.0f);

    // Счёт
    if (gameMode == MODE_PVP) {
        sprintf(scoreText, "%d  :  %d", playerScore, botScore);
    }
    else {
        sprintf(scoreText, "%s %d  :  %d BOT", playerName, playerScore, botScore);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCentered(0.35f, scoreText, 1.0f, 1.0f, 1.0f);

    // Подсказка
    glColor3f(0.5f, 0.5f, 0.5f);
    drawCentered(-0.65f, "Press SPACE or ENTER to return to menu", 0.5f, 0.5f, 0.5f);

    // Восстанавливаем исходный цвет фона
    glClearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
}


void drawMenu() {
    // Огромный заголовок
    drawTitleCentered(0.78f, "BALDA", 1.0f, 0.8f, 0.0f);

    // Подзаголовок (старый шрифт)
    glColor3f(0.7f, 0.7f, 0.7f);
    drawCentered(0.60f, "WORD GAME", 0.7f, 0.7f, 0.7f);

    // Меню выбора сложности
    if (selectingDifficulty) {
        const char* diffItems[] = { "Easy", "Medium", "Hard" };
        for (int i = 0; i < 3; i++) {
            float y = 0.35f - i * 0.12f;
            if (i == selectedMenuItem) {
                glColor3f(1.0f, 0.8f, 0.0f);
                drawString(-0.40f, y, ">");                    // стрелка
                glColor3f(1.0f, 1.0f, 1.0f);
                drawMenuItemString(-0.22f, y, diffItems[i]);  // пункт со смещением
            }
            else {
                glColor3f(0.8f, 0.8f, 0.8f);
                drawMenuItemString(-0.28f, y, diffItems[i]);
            }
        }
        glColor3f(0.5f, 0.5f, 0.5f);
        drawCentered(-0.85f, "Select difficulty", 0.5f, 0.5f, 0.5f);
        return;
    }

    // Меню выбора режима
    if (selectingMode) {
        const char* modeItems[] = { "Player vs Player", "Player vs Bot" };
        for (int i = 0; i < 2; i++) {
            float y = 0.35f - i * 0.12f;
            if (i == selectedMenuItem) {
                glColor3f(1.0f, 0.8f, 0.0f);
                drawString(-0.40f, y, ">");
                glColor3f(1.0f, 1.0f, 1.0f);
                drawMenuItemString(-0.22f, y, modeItems[i]);
            }
            else {
                glColor3f(0.8f, 0.8f, 0.8f);
                drawMenuItemString(-0.28f, y, modeItems[i]);
            }
        }
        glColor3f(0.5f, 0.5f, 0.5f);
        drawCentered(-0.85f, "Select game mode", 0.5f, 0.5f, 0.5f);
        return;
    }

    // Главное меню
    const char* items[] = { "New Game", "Records", "Help", "Exit" };
    for (int i = 0; i < 4; i++) {
        float y = 0.35f - i * 0.12f;
        if (i == selectedMenuItem) {
            glColor3f(1.0f, 0.8f, 0.0f);
            drawString(-0.30f, y, ">");
            glColor3f(1.0f, 1.0f, 1.0f);
            drawMenuItemString(-0.12f, y, items[i]);
        }
        else {
            glColor3f(0.8f, 0.8f, 0.8f);
            drawMenuItemString(-0.18f, y, items[i]);
        }
    }
    glColor3f(0.5f, 0.5f, 0.5f);
    drawCentered(-0.85f, "Use arrows and Enter", 0.5f, 0.5f, 0.5f);
}

void drawHelp() {
    glColor3f(1.0f, 0.8f, 0.0f);
    drawCentered(0.85f, "HELP", 1.0f, 0.8f, 0.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawString(-0.7f, 0.65f, "Controls:");
    drawString(-0.7f, 0.50f, "F1 - Main Menu");
    drawString(-0.7f, 0.38f, "F2 - Records");
    drawString(-0.7f, 0.26f, "F3 - Help");
    drawString(-0.7f, 0.14f, "F4 - Exit");

    drawString(-0.7f, -0.10f, "Game rules:");
    drawString(-0.7f, -0.25f, "1. Click on an empty cell");
    drawString(-0.7f, -0.37f, "2. Type a letter");
    drawString(-0.7f, -0.49f, "3. Form a word through that cell");
    drawString(-0.7f, -0.61f, "4. Press Enter to submit");

    glColor3f(0.5f, 0.5f, 0.5f);
    drawCentered(-0.85f, "Press F1 to return", 0.5f, 0.5f, 0.5f);
}

void drawRecords() {
    glColor3f(1.0f, 0.8f, 0.0f);
    drawCentered(0.85f, "RECORDS", 1.0f, 0.8f, 0.0f);

    // Кнопки выбора сложности
    const char* diffLabels[] = { "Easy", "Medium", "Hard" };
    for (int i = 0; i < 3; i++) {
        float x = -0.5f + i * 0.35f;
        float y = 0.70f;

        if (i == getRecordsDifficulty()) {
            glColor3f(1.0f, 0.8f, 0.0f);
            drawString(x, y, ">");
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        else {
            glColor3f(0.7f, 0.7f, 0.7f);
        }
        drawString(x + 0.05f, y, diffLabels[i]);
    }

    // Загружаем рекорды для отображения
    loadRecordsForDisplay();
    Record* records = getCurrentRecords();

    // Отображаем таблицу
    glColor3f(1.0f, 1.0f, 1.0f);
    drawString(-0.6f, 0.55f, "#  Name             Score     Date");

    for (int i = 0; i < 5; i++) {
        char line[50];
        if (records[i].score > 0) {
            sprintf(line, "%d. %-16s %-8d %s", i + 1, records[i].name, records[i].score, records[i].date);
        }
        else {
            sprintf(line, "%d. ---", i + 1);
        }
        drawString(-0.6f, 0.48f - i * 0.08f, line);
    }

    glColor3f(0.5f, 0.5f, 0.5f);
    drawCentered(-0.85f, "Use left/right arrows to change difficulty, F1 to return", 0.5f, 0.5f, 0.5f);
}

void drawNameInput() {
    glColor3f(1.0f, 0.8f, 0.0f);
    drawCentered(0.75f, "ENTER YOUR NAME", 1.0f, 0.8f, 0.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawCentered(0.30f, nameInput, 1.0f, 1.0f, 1.0f);

    float width = strlen(nameInput) * 0.028f;
    glBegin(GL_LINES);
    glVertex2f(-width / 2, 0.25f);
    glVertex2f(width / 2, 0.25f);
    glEnd();

    glColor3f(0.5f, 0.5f, 0.5f);
    drawCentered(0.0f, "Type your name using A-Z", 0.5f, 0.5f, 0.5f);
    drawCentered(-0.20f, "Press Enter to continue", 0.5f, 0.5f, 0.5f);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (width > height) {
        float aspect = (float)width / (float)height;
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    }
    else {
        float aspect = (float)height / (float)width;
        glOrtho(-1.0, 1.0, -aspect, aspect, -1.0, 1.0);
    }

    glMatrixMode(GL_MODELVIEW);
}