#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <SDL2/SDL.h>

extern SDL_Renderer* renderer;
extern SDL_Window* window;
extern int screen_w;
extern int screen_h;
extern int cameraX;
extern int cameraY;

void win_init(const char* name, int count, char* arg[]);
void win_toggleFullScreen();
void win_swapBuffer();
void win_clear();

#endif