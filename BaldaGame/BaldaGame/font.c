#include "font.h"
#include <windows.h>
#include <GLFW/glfw3.h>

static GLuint fontBase = 0;

void initRussianFont() {
    HDC hdc = wglGetCurrentDC();
    fontBase = glGenLists(256);

    HFONT hFont = CreateFontA(
        -36, 0, 0, 0, FW_BOLD,
        FALSE, FALSE, FALSE,
        RUSSIAN_CHARSET,
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

void drawRussianString(float x, float y, const char* str) {
    if (!str || !str[0]) return;

    glRasterPos2f(x, y);
    glPushAttrib(GL_LIST_BIT);
    glListBase(fontBase);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();
}