#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_image.h>
#include <time.h>
#include <math.h>
#include "textLoader.h"
#include "imageLoader.h"
#include "windowManager.h"
#include "eventHandler.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif


#define MAX_SPEED 1

/*int HEIGHT = 720;
int WIDTH = 1080;

int bh_frame; //current frame for black hole
int max_bh_frame;
double delay; // for animation purposes
double max_delay;
*/

typedef struct {
    double x, y;   // position
    double dx, dy; // velocity 
    double ax, ay; // acceleration 
    double angle;
    int isAdvancing; // 1 if should be accellerating forward
    int isRotatingL; // 1 if rotating left
    int isRotatingR; // 1 if rotating right
    double rotationL; // >0 if rotating left
    double rotationR; // >0 if rotating right
    double speed; // speed granted by the propellers. has a limit
} spaceship;

typedef struct {
    double x, y; // position
    double distance; // distance to spaceship
    double angle; // angle relative to spaceship, no need to always be calculated
} blackhole;

spaceship* ship;
blackhole* hole;

animated_image* ship_image;
animated_image* hole_image;

animated_image* stars1;
animated_image* stars2;

int tiles1w;
int tiles1h;
int tiles2w;
int tiles2h;

int* tiles1;
int* tiles2;

static spaceship* createSpaceship() {
    spaceship* s = malloc(sizeof(spaceship));
    s->x = 0;
    s->y = 0;
    s->dx = 0;
    s->dy = 0;
    s->ax = 0;
    s->ay = 0;
    s->angle = 0;
    s->isAdvancing = 0;
    s->isRotatingR = 0;
    s->isRotatingL = 0;
    s->rotationL = 0;
    s->rotationR = 0;
    s->speed = 0;
    return s;
}

static blackhole* createBlackhole(double x, double y) {
    blackhole* b = malloc(sizeof(blackhole));
    b->x = x;
    b->y = y;
    b->angle = 0.0f;
    b->distance = 1000000.0f;
    return b;
}

static void updateDistance(spaceship* ship, blackhole* bh) {
    bh->distance = sqrt((ship->x - bh->x) * (ship->x - bh->x) + (ship->y - bh->y) * (ship->y - bh->y));
}

// we only check for the one blackhole we have rn cuz we're lazy
static void updateAcceleration(spaceship* ship, blackhole* bh) {

    bh->angle = atan2(ship->y - bh->y, ship->x - bh->x); //update the angle
    double ax = -cos(bh->angle);
    double ay = -sin(bh->angle);
    double d = bh->distance / 140;
    ship->ax = ax * 10 / (1 + d * d) / 2;
    ship->ay = ay * 10 / (1 + d * d) / 2;

}

static void updateShip(spaceship* ship, double amount) {
    if (ship->isAdvancing) ship->speed += 0.1 * amount;
    else ship->speed = ship->speed * (10 - amount) / 10;
    if (ship->speed > MAX_SPEED) ship->speed = MAX_SPEED;
    else if (ship->speed < 0) ship->speed = 0;
    if (ship->isRotatingR) ship->rotationR += 0.3 * amount;
    if (ship->isRotatingL) ship->rotationL += 0.3 * amount;
    if (ship->rotationR > 5 * amount) ship->rotationR = 5 * amount;
    if (ship->rotationL > 5 * amount) ship->rotationL = 5 * amount;
    ship->dx += (sin(ship->angle * M_PI / 180) * ship->speed) * amount;
    ship->dy += (-cos(ship->angle * M_PI / 180) * ship->speed) * amount;
    ship->dx += ship->ax * amount;
    ship->dy += ship->ay * amount;
    ship->x += ship->dx * amount;
    ship->y += ship->dy * amount;
    ship->dx *= (100 - amount * 4) / 100;
    ship->dy *= (100 - amount * 4) / 100;
    ship->angle += ship->rotationR - ship->rotationL;
    ship->rotationR -= 0.15 * amount;
    ship->rotationL -= 0.15 * amount;
    if (ship->rotationR < 0) ship->rotationR = 0;
    if (ship->rotationL < 0) ship->rotationL = 0;
}

static void updatePysics(spaceship* ship, blackhole* bh, double amount) {
    updateDistance(ship, bh);
    updateAcceleration(ship, bh);
    updateShip(ship, amount);
}

static void drawship() {
    int a;
    if (ship->speed == 0) a = 0;
    else {
        double i = ship->speed * 3 / MAX_SPEED;
        a = (int) i;
    }

    IMG_setAnimatedAngle(ship_image, ship->angle);
    IMG_drawFrameOfAnimatedImage(ship_image, 0, 0, a);
}

static void drawBlackHole() {
    IMG_drawAnimatedImage(hole_image, hole->x - ship->x, hole->y - ship->y);
}

static void putRandomNumbersInArray(int array[], int size, int max) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % max;
    }
}

// TODO : not this
// this is really bad
static void drawStars(animated_image* img, int tiles[], int w, int h, double rate) {
    int imageSize = IMG_getAnimatedWidth(img);
    int posx = -ship->x;
    int posy = -ship->y;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int j2 = (j + posx) % w;
            int i2 = (i + posy) % h;
            if (j2 < 0) j2 += w;
            if (i2 < 0) i2 += h;
            int destx = (j2 * imageSize + (int) (posx * rate)) % (w * imageSize);
            int desty = (i2 * imageSize + (int) (posy * rate)) % (h * imageSize);
            if (destx < -imageSize) destx += (w * imageSize);
            if (desty < -imageSize) desty += (h * imageSize);
            destx -= (w * imageSize) / 2;
            desty -= (h * imageSize) / 2;
            IMG_drawFrameOfAnimatedImage(img, destx, desty, tiles[i2 * h + j2]);
        }
    }
}

// ------- 
//   UwU
// -------

void space_draw() {
    drawStars(stars2, tiles2, tiles2w, tiles2h, 0.2);
    drawStars(stars1, tiles1, tiles1w, tiles1h, 0.5);
    drawBlackHole();
    drawship();
}

static void forward() { ship->isAdvancing = 1; }
static void left() { ship->isRotatingL = 1; }
static void right() { ship->isRotatingR = 1; }
static void forward_stop() { ship->isAdvancing = 0; }
static void left_stop() { ship->isRotatingL = 0; }
static void right_stop() { ship->isRotatingR = 0; }

static void toggleFullScreen() {
    win_toggleFullScreen();

    // dirty but works for now. fix later
    tiles1w = (screen_w / IMG_getAnimatedWidth(stars1)) + 4;
    tiles1h = (screen_h / IMG_getAnimatedWidth(stars1)) + 4;
    tiles2w = (screen_w / IMG_getAnimatedWidth(stars2)) + 4;
    tiles2h = (screen_h / IMG_getAnimatedWidth(stars2)) + 4;

    free(tiles1);
    free(tiles2);

    tiles1 = (int*) malloc(tiles1w * tiles1h * sizeof(int));
    tiles2 = (int*) malloc(tiles2w * tiles2h * sizeof(int));

    putRandomNumbersInArray(tiles1, tiles1w * tiles1h, IMG_getNbOfFrames(stars1));
    putRandomNumbersInArray(tiles2, tiles2w * tiles2h, IMG_getNbOfFrames(stars2));
}

static void init_keys() {
    event_init_keyDowns(5);
    event_init_keyUps(3);

    event_setKeyDown(SDLK_UP, forward);
    event_setKeyDown(SDLK_LEFT, left);
    event_setKeyDown(SDLK_RIGHT, right);
    event_setKeyDown(SDLK_F11, toggleFullScreen);
    event_setKeyDown(SDLK_f, toggleFullScreen);

    event_setKeyUp(SDLK_UP, forward_stop);
    event_setKeyUp(SDLK_LEFT, left_stop);
    event_setKeyUp(SDLK_RIGHT, right_stop);
}

void space_quit() {
    free(hole);
    free(ship);
    IMG_freeAnimatedImage(stars1);
    IMG_freeAnimatedImage(stars2);
    IMG_freeAnimatedImage(ship_image);
    IMG_freeAnimatedImage(hole_image);

    free(tiles1);
    free(tiles2);
}

void space_start() {
    hole = createBlackhole(600, -700);
    ship = createSpaceship();
    stars1 = IMG_createAnimatedImage("ressource/stars_alpha.png", 0, 0);
    stars2 = IMG_createAnimatedImage("ressource/stars_w_sky.png", 0, 0);
    ship_image = IMG_createAnimatedImage("ressource/spaceship_frames.png", 1, 0);
    hole_image = IMG_createAnimatedImage("ressource/blackhole.png", 0, 3);

    //these values represent the sizes of the arrays, not the image
    //we always use width for calculating because one tiles is a square 
    tiles1w = (screen_w / IMG_getAnimatedWidth(stars1)) + 4;
    tiles1h = (screen_h / IMG_getAnimatedWidth(stars1)) + 4;
    tiles2w = (screen_w / IMG_getAnimatedWidth(stars2)) + 4;
    tiles2h = (screen_h / IMG_getAnimatedWidth(stars2)) + 4;

    tiles1 = (int*) malloc(tiles1w * tiles1h * sizeof(int));
    tiles2 = (int*) malloc(tiles2w * tiles2h * sizeof(int));

    putRandomNumbersInArray(tiles1, tiles1w * tiles1h, IMG_getNbOfFrames(stars1));
    putRandomNumbersInArray(tiles2, tiles2w * tiles2h, IMG_getNbOfFrames(stars2));

    init_keys();
    event_setOnExitCurrent(space_quit);
}

void space_changeState() {
    updatePysics(ship, hole, 1);
}


