#include <SDL2/SDL.h>
#include "eventHandler.h"

#define safe_free(var) if(var!=NULL) {free(var); var=NULL;}

typedef void (*function)(void);
typedef Uint8 mousecode;

SDL_Event event;

SDL_Keycode* keys_down = NULL;
SDL_Keycode* keys_up = NULL;
mousecode* button_downs = NULL;
mousecode* button_ups = NULL;

function* func_keyDown = NULL;
function* func_keyUp = NULL;
function* func_mouseButtonDown = NULL;
function* func_mouseButtonUp = NULL;

function clear_and_exit;

int n_keydown = 0;
int n_keyup = 0;
int n_mouseButtonDown = 0;
int n_mouseButtonUp = 0;

int curr_n_keydown = 0;
int curr_n_keyup = 0;
int curr_n_mouseButtonDown = 0;
int curr_n_mouseButtonUp = 0;

void nullfunction() {}

static void init_func_array(int* num, function** func, int n) {
    *num = n;
    *func = (function*) malloc(sizeof(function) * n);
    for (int i = 0; i < n; i++) {
        (*func)[i] = nullfunction;
    }
}

void event_init_keyDowns(int n) {
    keys_down = (SDL_Keycode*) malloc(sizeof(SDL_Keycode) * n);
    init_func_array(&n_keydown, &func_keyDown, n);
}
void event_init_keyUps(int n) {
    keys_up = (SDL_Keycode*) malloc(sizeof(SDL_Keycode) * n);
    init_func_array(&n_keyup, &func_keyUp, n);
}
void event_init_mouseDowns(int n) {
    button_downs = (mousecode*) malloc(sizeof(mousecode) * n);
    init_func_array(&n_mouseButtonDown, &func_mouseButtonDown, n);
}
void event_init_mouseUps(int n) {
    button_ups = (mousecode*) malloc(sizeof(mousecode) * n);
    init_func_array(&n_mouseButtonUp, &func_mouseButtonUp, n);
}

static void setKeyIfPossible(SDL_Keycode* keys, function* funcs, SDL_Keycode key, function f, int* n, int max_n) {
    if (*n >= max_n) {
        printf("error : trying to set more inputs than allocated\n");
        exit(1);
    }
    funcs[*n] = f;
    keys[*n] = key;
    *n = (*n + 1);
}
static void setButtonIfPossible(mousecode* codes, function* funcs, mousecode code, function f, int* n, int max_n) {
    if (*n >= max_n) {
        printf("error : trying to set more inputs than allocated\n");
        exit(1);
    }
    funcs[*n] = f;
    codes[*n] = code;
    *n = (*n + 1);
}

void event_setKeyDown(SDL_Keycode key, function f) {
    setKeyIfPossible(keys_down, func_keyDown, key, f, &curr_n_keydown, n_keydown);
}
void event_setKeyUp(SDL_Keycode key, function f) {
    setKeyIfPossible(keys_up, func_keyUp, key, f, &curr_n_keyup, n_keyup);
}
void event_setMouseDown(mousecode code, function f) {
    setButtonIfPossible(button_downs, func_mouseButtonDown, code, f, &curr_n_mouseButtonDown, n_mouseButtonDown);
}
void event_setMouseUp(mousecode code, function f) {
    setButtonIfPossible(button_ups, func_mouseButtonUp, code, f, &curr_n_mouseButtonUp, n_mouseButtonUp);
}

void event_reset() {
    n_keydown = 0;
    n_keyup = 0;
    n_mouseButtonDown = 0;
    n_mouseButtonUp = 0;

    curr_n_keydown = 0;
    curr_n_keyup = 0;
    curr_n_mouseButtonDown = 0;
    curr_n_mouseButtonUp = 0;

    safe_free(keys_down);
    safe_free(keys_up);
    safe_free(button_downs);
    safe_free(button_ups);

    safe_free(func_keyDown);
    safe_free(func_keyUp);
    safe_free(func_mouseButtonDown);
    safe_free(func_mouseButtonUp);

    clear_and_exit = nullfunction;
}

static void onKeyDown(SDL_Keycode key) {
    for (int i = 0; i < n_keydown; i++) {
        if (keys_down[i] == key) {
            func_keyDown[i]();
            break;
        }
    }
}

static void onKeyUp(SDL_Keycode key) {
    for (int i = 0; i < n_keyup; i++) {
        if (keys_up[i] == key) {
            func_keyUp[i]();
            break;
        }
    }
}

static void onMouseDown(mousecode code) {
    for (int i = 0; i < n_mouseButtonDown; i++) {
        if (button_downs[i] == code) {
            func_mouseButtonDown[i]();
            break;
        }
    }
}

static void onMouseUp(mousecode code) {
    for (int i = 0; i < n_mouseButtonUp; i++) {
        if (button_ups[i] == code) {
            func_mouseButtonUp[i]();
            break;
        }
    }
}


int event_handleInputs() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:; break;
            case SDL_MOUSEBUTTONDOWN: onMouseDown(event.button.button); break;
            case SDL_MOUSEBUTTONUP: onMouseUp(event.button.button); break;
            case SDL_KEYDOWN:
                onKeyDown(event.key.keysym.sym);
                if (event.key.keysym.sym == SDLK_ESCAPE) return INPUT_RETURN;
                break;
            case SDL_KEYUP: onKeyUp(event.key.keysym.sym); break;
            case SDL_QUIT: return INPUT_EXIT; break;
        }
    }
    return INPUT_CONTINUE;
}

void event_setOnExitCurrent(function f) {
    clear_and_exit = f;
}

void event_exitCurrent() {
    clear_and_exit();
    event_reset();
}