#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <dlfcn.h>
#include "imageLoader.h"
#include "textLoader.h"
#include "windowManager.h"
#include "eventHandler.h"
#include "gameoflife/gol_run.h"
#include "space/space_run.h"

#define NB_OF_OPT 3

static int opt;
char* options[NB_OF_OPT];
int loops;

typedef void (*function)(void);

function changeState;
function draw;

void nullfonction() {}

void prev_option() {
    opt = (opt <= 0) ? (NB_OF_OPT - 1) : (opt - 1);
}

void next_option() {
    opt = (opt >= NB_OF_OPT - 1) ? (0) : (opt + 1);
}

void menu_changeState() {

    switch (opt) {
        case 0:
            changeState = gol_changeState;
            draw = gol_draw;
            event_reset();
            gol_start();
            break;
        case 1:
            changeState = space_changeState;
            draw = space_draw;
            event_reset();
            space_start();
            break;
        case 2: loops = 0; break;
    }
}

void quit() {
    loops = 0;
}

void draw_main() {

    for (int i = 0; i < NB_OF_OPT; i++) {
        txt_drawText(options[i], 50, i * 50);
    }
    txt_drawText("->", 5, opt * 50);

    /*
    txt_drawAlphabet();
    txt_drawText(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijk",0,100);
    txt_drawText("lmnopqrstuvwxyz{|}~ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×Ø",0,130);
    txt_drawText("ÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ",0,160);
    */

}

void init_main() {
    opt = 0;
    event_init_keyDowns(7);
    event_setKeyDown(SDLK_UP, prev_option);
    event_setKeyDown(SDLK_DOWN, next_option);
    event_setKeyDown(SDLK_RETURN, menu_changeState);
    event_setKeyDown(SDLK_SPACE, menu_changeState);
    event_setKeyDown(SDLK_F11, win_toggleFullScreen);
    event_setKeyDown(SDLK_f, win_toggleFullScreen);
    event_setKeyDown(SDLK_ESCAPE, quit);

    draw = draw_main;
    changeState = nullfonction;
    event_setOnExitCurrent(quit);
}

int main(int count, char* arg[]) {
    win_init("programm", count, arg);
    win_clear();
    win_swapBuffer();
    txt_initTextLoader(TEXT_BLENDED);
    options[0] = "game of life";
    options[1] = "space!";
    options[2] = "quit";
    init_main();

    int desiredFramerate = 60;
    float desiredDelay = 1.0f / (float) desiredFramerate * 1000.0f; // in milliseconds

    loops = 1;
    while (loops) {
        clock_t t1 = clock();

        int input = event_handleInputs();
        changeState();

        win_clear();
        draw();
        win_swapBuffer();

        switch (input) {
            case INPUT_EXIT:
                event_exitCurrent();
                init_main();
                loops = 0;
                break;
            case INPUT_RETURN:
                event_exitCurrent();
                init_main();
                break;
        }

        clock_t t2 = clock();

        // cap at 60 fps
        float elapsedMS = (t2 - t1) / (float) CLOCKS_PER_SEC * 1000.0f;
        float delay = floor(desiredDelay - elapsedMS);
        if (delay > 0.0f) {
            SDL_Delay(delay);
        }
    }

    SDL_Quit();
    return 0;
}