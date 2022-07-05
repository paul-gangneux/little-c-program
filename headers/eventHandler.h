#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H
#include <SDL2/SDL.h>

#define INPUT_EXIT 0
#define INPUT_RETURN 1
#define INPUT_CONTINUE 2

void event_init_keyDowns(int n);
void event_init_keyUps(int n);
void event_init_mouseDowns(int n);
void event_init_mouseUps(int n);

void event_setKeyDown(SDL_Keycode key, void (*function)(void));
void event_setKeyUp(SDL_Keycode key, void (*function)(void));
void event_setMouseDown(Uint8 code, void (*function)(void));
void event_setMouseUp(Uint8 code, void (*function)(void));

void event_reset();

void event_setOnExitCurrent(void (*function)(void));
void event_exitCurrent();

int event_handleInputs();

#endif