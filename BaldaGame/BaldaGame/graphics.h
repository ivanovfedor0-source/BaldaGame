#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GLFW/glfw3.h>
#include "menu.h"

void initFont(void);
void initTitleFont(void);
void initSmallFont(void);
void initMenuItemsFont(void);
void drawString(float x, float y, const char* str);
void drawCentered(float y, const char* str, float r, float g, float b);
void drawMenu(void);
void drawHelp(void);
void drawRecords(void);
void drawNameInput(void);
void drawGameOver(void);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void drawBackButton(void);

#endif