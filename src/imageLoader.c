#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "imageLoader.h"
#include "windowManager.h"

struct static_image {
    int w, h;
    double angle;
    SDL_Texture* texture;
};

struct animated_image {
    int w, h;
    int nbOfFrames;
    int currentFrame;
    int delay;
    int currentDelay;
    double angle;
    SDL_Texture* texture;
};

// x would usually equal positionX - cameraX - camOffsetX
void IMG_drawStaticImage(static_image* image, int x, int y) {
    SDL_Rect src, dst;
    src.w = image->w;
    src.h = image->h;
    dst.w = image->w;
    dst.h = image->w;
    src.x = 0;
    src.y = 0;
    dst.x = screen_w / 2 - image->w / 2 + x;// - cameraX - camOffsetX;
    dst.y = screen_h / 2 - image->h / 2 + y;// - cameraY - camOffsetY;

    if (image->angle == 0) {
        if (dst.x<screen_w && dst.y<screen_h && dst.x>-image->w && dst.y>-image->h) {
            SDL_RenderCopy(renderer, image->texture, &src, &dst);
        }
    }
    else {
        if (
            (dst.x<screen_w && dst.y<screen_h && dst.x>-image->w && dst.y>-image->h) ||
            (dst.x<screen_h && dst.y<screen_w && dst.x>-image->h && dst.y>-image->w)
            ) {
            SDL_Point center = { image->w / 2, image->h / 2 };
            SDL_RenderCopyEx(renderer, image->texture, &src, &dst, image->angle, &center, SDL_FLIP_NONE);
        }
    }
}

void IMG_drawFrameOfAnimatedImage(animated_image* image, int x, int y, int frameNb) {
    SDL_Rect src, dst;
    src.w = image->w;
    src.h = image->h;
    dst.w = image->w;
    dst.h = image->w;
    src.x = 0;
    src.y = image->h * frameNb;
    dst.x = screen_w / 2 - image->w / 2 + x;// - cameraX - camOffsetX;
    dst.y = screen_h / 2 - image->h / 2 + y;// - cameraY - camOffsetY;

    if (image->angle == 0) {
        if (dst.x<screen_w && dst.y<screen_h && dst.x>-image->w && dst.y>-image->h) {
            SDL_RenderCopy(renderer, image->texture, &src, &dst);
        }
    }
    else {
        if (
            (dst.x<screen_w && dst.y<screen_h && dst.x>-image->w && dst.y>-image->h) ||
            (dst.x<screen_h && dst.y<screen_w && dst.x>-image->h && dst.y>-image->w)
            ) {
            SDL_Point center = { image->w / 2, image->h / 2 };
            SDL_RenderCopyEx(renderer, image->texture, &src, &dst, image->angle, &center, SDL_FLIP_NONE);
        }
    }
}

void IMG_drawAnimatedImage(animated_image* image, int x, int y) {
    IMG_drawFrameOfAnimatedImage(image, x, y, image->currentFrame);

    image->currentDelay = (image->currentDelay + 1) % image->delay;
    if (image->currentDelay == 0)
        image->currentFrame = (image->currentFrame + 1) % image->nbOfFrames;
}



// call this after initialising the renderer
// also allows to store the width of the image in the variable sizeOfImage
static SDL_Texture* IMG_textureFromImage(const char* filename, int hasColorKey, int* widthOfImage, int* heightOfImage) {
    SDL_Surface* surf;
    surf = IMG_Load(filename);
    if (surf == NULL) {
        printf("Unable to load bitmap from %s\n", filename);
        exit(1);
    }
    if (hasColorKey) {
        SDL_SetColorKey(surf, SDL_RLEACCEL, (Uint32) SDL_MapRGB(surf->format, 255, 0, 255));
    }
    if (widthOfImage != NULL) *widthOfImage = surf->w;
    if (heightOfImage != NULL) *heightOfImage = surf->h;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

static_image* IMG_createStaticImage(const char* filename, int hasColorKey) {
    static_image* image = (static_image*) malloc(sizeof(static_image));
    image->texture = IMG_textureFromImage(filename, hasColorKey, &image->w, &image->h);
    image->angle = 0;
    return image;
}

// we assume each frame is a perfect square, arranged in a column
animated_image* IMG_createAnimatedImage(const char* filename, int hasColorKey, int delay) {

    animated_image* image = (animated_image*) malloc(sizeof(animated_image));
    image->texture = IMG_textureFromImage(filename, hasColorKey, &image->w, &image->h);
    image->angle = 0;
    image->nbOfFrames = image->h / image->w;
    image->h = image->w;
    image->delay = delay;
    image->currentDelay = 0;
    image->currentFrame = 0;

    return image;
}

void IMG_resetAnimation(animated_image* image) {
    image->currentDelay = 0;
    image->currentFrame = 0;
}

void IMG_freeStaticImage(static_image* image) {
    SDL_DestroyTexture(image->texture);
    free(image);
}

void IMG_freeAnimatedImage(animated_image* image) {
    SDL_DestroyTexture(image->texture);
    free(image);
}

void IMG_setStaticAngle(static_image* image, double angle) {
    image->angle = angle;
}

void IMG_setAnimatedAngle(animated_image* image, double angle) {
    image->angle = angle;
}

int IMG_getStaticWidth(static_image* image) {
    return image->w;
}
int IMG_getAnimatedWidth(animated_image* image) {
    return image->w;
}

int IMG_getNbOfFrames(animated_image* image) {
    return image->nbOfFrames;
}