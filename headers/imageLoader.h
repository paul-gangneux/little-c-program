#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct static_image static_image;
typedef struct animated_image animated_image;

void IMG_drawStaticImage(static_image* image, int x, int y);
void IMG_drawAnimatedImage(animated_image* image, int x, int y);
void IMG_drawFrameOfAnimatedImage(animated_image* image, int x, int y, int frameNb);
static_image* IMG_createStaticImage(const char* filename, int hasColorKey);
animated_image* IMG_createAnimatedImage(const char* filename, int hasColorKey, int delay);
void IMG_resetAnimation(animated_image* image);

void IMG_freeStaticImage(static_image* image);
void IMG_freeAnimatedImage(animated_image* image);

void IMG_setStaticAngle(static_image* image, double angle);
void IMG_setAnimatedAngle(animated_image* image, double angle);

int IMG_getStaticWidth(static_image* image);
int IMG_getAnimatedWidth(animated_image* image);

int IMG_getNbOfFrames(animated_image* image);

#endif