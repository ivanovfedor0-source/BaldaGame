#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GLFW/glfw3.h>
#include "menu.h"

void initFont();
void drawString(float x, float y, const char* str);
void drawCentered(float y, const char* str, float r, float g, float b);
void drawMenu();
void drawHelp();
void drawRecords();
void drawNameInput();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

#endif