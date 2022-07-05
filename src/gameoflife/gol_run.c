#include <stdlib.h>
// #include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "windowManager.h"
#include "textLoader.h"
#include "eventHandler.h"
#include "gameoflife/cell.h"
#include "gameoflife/text_to_cells.h"

#define STRBUFFER_SIZE 128 //TODO : make this global

// -- stuff used to make everything work

char strBuffer[STRBUFFER_SIZE];
int desiredFramerate = 60;
float desiredDelay; // in milliseconds
int delay = 3;
int size = 10;
int first_loop_runs = 1;
int second_loop_runs = 1;
int lock_frame_rate = 1;
int cameraX=0, cameraY=0;
int posX, posY;
int cameraXMovement=0;
int cameraYMovement=0;
int debug = 0;
unsigned int counter=0;
int mainphase=0;

// ------------------- drawing functions --------------------- //

static void drawSquare(int x, int y, int size) {
    SDL_Rect r;
    r.h=size;
    r.w=size;

    size+=size/5;
    r.x=x*size+screen_w/2-cameraX*size;
    r.y=y*size+screen_h/2+cameraY*size;
    SDL_SetRenderDrawColor(renderer, 255, 10, 10, 255); // red
    SDL_RenderDrawRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

static void drawInfo() {
    int a = cell_countLivingCells();
    int b = cell_countPotentialCells();
    int c = cell_countGarbageCells();
    sprintf(strBuffer, "living: %d", a);
    txt_drawText(strBuffer, 0, 0);
    sprintf(strBuffer, "potential: %d", b);
    txt_drawText(strBuffer, 0, 30);
    sprintf(strBuffer, "garbage: %d", c);
    txt_drawText(strBuffer, 0, 60);
    sprintf(strBuffer, "total: %d", a+b+c);
    txt_drawText(strBuffer, 0, 90);
    sprintf(strBuffer, "allocations: %d", cell_getAllocCounter()-cell_getFreeCounter());
    txt_drawText(strBuffer, 0, 120);
}

static void clear() {
    cell_quit();
}


// --- possible actions done by user --- // 

// --- key downs --- //

static void up() {
    if (!mainphase) posY--;
    else cameraYMovement=-1;
}
static void down() {
    if (!mainphase) posY++; 
    else cameraYMovement=1;
}
static void left() {
    if (!mainphase) posX--; 
    else cameraXMovement=-1;
}
static void right() {
    if (!mainphase) posX++; 
    else cameraXMovement=1;
}
static void zoomOut() {
    size--;
    if (size < 1) size = 1;
}
static void zoomIn() {
    size++;
    if (size > 100) size = 100;
}
static void createOrDeleteCell() {
    if (!mainphase) cell_switchLivingCell(posX,posY);
}
static void fasterDelay() {
    delay--;
    if (delay<1) delay = 1;
}
static void slowerDelay() {
    delay++;
    if (delay>desiredFramerate) delay = desiredFramerate; // 1 per second
}
static void resetDelay() {
    delay = 3;
}
static void resetDelay2() {
    delay = 1;
}
static void switchDebug() {
    debug=!debug;
}
static void switchState() {
    mainphase=!mainphase;
    cell_freeNonLivingCells();
    posX=cameraX;
    posY=-cameraY;
}

// cell_freeAllCells()

// win_toggleFullScreen()

// --- key ups --- //

static void stopUpOrDown() {
    cameraYMovement=0;
}
static void stopLeftOrRight() {
    cameraXMovement=0;
}

static void init_keys() {
    event_init_keyDowns(16);
    event_init_keyUps(4);

    event_setKeyDown(SDLK_UP, up);
    event_setKeyDown(SDLK_DOWN, down);
    event_setKeyDown(SDLK_LEFT, left);
    event_setKeyDown(SDLK_RIGHT, right);
    event_setKeyDown(SDLK_a, zoomIn);
    event_setKeyDown(SDLK_q, zoomOut);
    event_setKeyDown(SDLK_SPACE, createOrDeleteCell);
    event_setKeyDown(SDLK_w, resetDelay);
    event_setKeyDown(SDLK_x, slowerDelay);
    event_setKeyDown(SDLK_c, fasterDelay);
    event_setKeyDown(SDLK_v, resetDelay2);
    event_setKeyDown(SDLK_b, switchDebug);
    event_setKeyDown(SDLK_RETURN, switchState);
    event_setKeyDown(SDLK_r, cell_freeAllCells);
    event_setKeyDown(SDLK_f, win_toggleFullScreen);
    event_setKeyDown(SDLK_F11, win_toggleFullScreen);

    event_setKeyUp(SDLK_UP, stopUpOrDown);
    event_setKeyUp(SDLK_DOWN, stopUpOrDown);
    event_setKeyUp(SDLK_LEFT, stopLeftOrRight);
    event_setKeyUp(SDLK_RIGHT, stopLeftOrRight);

    event_setOnExitCurrent(clear);
}

// fonctions appellés depuis main :

void gol_start() {
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    
    desiredFramerate = 60;
    delay = 3;
    size = 10;
    first_loop_runs = 1;
    second_loop_runs = 1;
    lock_frame_rate = 1;
    cameraX=0, cameraY=0;
    posX=0, posY=0;
    cameraXMovement=0;
    cameraYMovement=0;
    debug = 0;
    mainphase = 0;
    counter=0;
    init_keys();
    memset(strBuffer, 0, STRBUFFER_SIZE);
    cell_init();
    // cell_stringToCells("Astro is gay");
}

void gol_draw() {
    cell_drawCells(size, 255, 255, 255);
    if (!mainphase) drawSquare(posX, posY, size);
    if (debug) drawInfo();
}
    
void gol_changeState() {

    if (cameraXMovement==1) cameraX+=(size>5)?1:6-size;
    else if (cameraXMovement==-1) cameraX-=(size>5)?1:6-size;
    if (cameraYMovement==1) cameraY-=(size>5)?1:6-size;
    else if (cameraYMovement==-1) cameraY+=(size>5)?1:6-size;
    
    if (mainphase) {
        counter++;
        if (counter>=delay) {
            cell_nextState();   
            counter=0;
        }
    }
}
    
/* cap at desiredFramerate fps
float elapsedMS = (t2 - t1) / (float)CLOCKS_PER_SEC * 1000.0f;
float time_delay = floor(desiredDelay - elapsedMS);
if (time_delay > 0.0f && lock_frame_rate) {
    SDL_Delay(time_delay);
}

//clock_t d2 = clock();
//printf("%f\n", (d2-d1)/(float)CLOCKS_PER_SEC);
//break;
*/

//desiredDelay = 1.0f / (float)desiredFramerate*1000.0f;
// cell_stringToCells(s);