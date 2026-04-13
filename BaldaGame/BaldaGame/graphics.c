#include "graphics.h"
#include "menu.h"
#include "game.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

static GLuint fontBase = 0;
extern int selectedMenuItem;
extern int selectingMode;
extern int selectingDifficulty;
extern char nameInput[20];

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

// Универсальный пункт меню со стрелкой и сдвигом текста
void drawMenuItem(float y, const char* str, int isSelected, float offsetX) {
    if (isSelected) {
        // Стрелка слева
        glColor3f(1.0f, 0.8f, 0.0f);
        drawString(offsetX - 0.08f, y, ">");

        // Текст со смещением вправо
        glColor3f(1.0f, 1.0f, 1.0f);
        drawString(offsetX + 0.05f, y, str);
    }
    else {
        // Обычный пункт (без стрелки, без смещения)
        glColor3f(0.8f, 0.8f, 0.8f);
        drawString(offsetX, y, str);
    }
}

void drawMenu() {
    // Заголовки — строго по центру
    glColor3f(1.0f, 0.8f, 0.0f);
    drawCentered(0.68f, "BALDA", 1.0f, 0.8f, 0.0f);

    glColor3f(0.7f, 0.7f, 0.7f);
    drawCentered(0.53f, "WORD GAME", 0.7f, 0.7f, 0.7f);

    // Меню выбора сложности
    if (selectingDifficulty) {
        const char* diffItems[] = { "Easy", "Medium", "Hard" };
        for (int i = 0; i < 3; i++) {
            float y = 0.35f - i * 0.12f;
            drawMenuItem(y, diffItems[i], (i == selectedMenuItem), -0.25f);
        }
        glColor3f(0.5f, 0.5f, 0.5f);
        drawCentered(-0.85f, "Select difficulty", 0.5f, 0.5f, 0.5f);
        return;
    }

    // Меню выбора режима (PVP / PVE)
    if (selectingMode) {
        const char* modeItems[] = { "Player vs Player", "Player vs Bot" };
        for (int i = 0; i < 2; i++) {
            float y = 0.35f - i * 0.12f;
            drawMenuItem(y, modeItems[i], (i == selectedMenuItem), -0.25f);
        }
        glColor3f(0.5f, 0.5f, 0.5f);
        drawCentered(-0.85f, "Select game mode", 0.5f, 0.5f, 0.5f);
        return;
    }

    // Главное меню
    const char* items[] = { "New Game", "Records", "Help", "Exit" };
    for (int i = 0; i < 4; i++) {
        float y = 0.35f - i * 0.12f;
        drawMenuItem(y, items[i], (i == selectedMenuItem), -0.15f);
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

    glColor3f(1.0f, 1.0f, 1.0f);
    drawString(-0.4f, 0.65f, "1. Player - 100 points");
    drawString(-0.4f, 0.53f, "2. Player - 85 points");
    drawString(-0.4f, 0.41f, "3. Player - 70 points");
    drawString(-0.4f, 0.29f, "4. Player - 55 points");
    drawString(-0.4f, 0.17f, "5. Player - 40 points");

    glColor3f(0.5f, 0.5f, 0.5f);
    drawCentered(-0.85f, "Press F1 to return", 0.5f, 0.5f, 0.5f);
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