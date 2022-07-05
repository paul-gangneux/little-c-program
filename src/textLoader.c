#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include "textLoader.h"
#include "windowManager.h"

const int TEXT_BLENDED = 0;
const int TEXT_SHADED = 1;
const int TEXT_SOLID = 2;

typedef struct {
    int x, y, w, h;
    char* text;
} dialogBox;

typedef struct {
    SDL_Texture* texture;
    int w, h;
} charTexture;

charTexture* alphabet;
int alphabet_length;

void txt_drawAlphabet() { // for debugging
    int x = 0;
    int y = 200;
    int del = 10;
    for (int i = 0; i < alphabet_length; i++) {
        charTexture ct = alphabet[i];
        if (x + ct.w >= screen_w) {
            x = 0;
            y += ct.h;
            del *= 2;
        }
        SDL_Rect src = { 0, 0, ct.w, ct.h };
        SDL_Rect dst = { x, y, ct.w, ct.h };
        SDL_RenderCopy(renderer, ct.texture, &src, &dst);
        x += ct.w;
        if (x >= screen_w) {

        }
    }
}

// don't ask how it works
void txt_drawTextColored(char* text, int x, int y, int R, int G, int B) {
    int i = 0;
    while (text[i]) {
        charTexture ct;
        int num = text[i] & 0x000000ff;
        if (num == 0xc2) {
            i++;
            ct = alphabet[(text[i] & 0x000000ff) - 65];
        }
        else if (num == 0xc3) {
            i++;
            ct = alphabet[(text[i] & 0x000000ff) - 1];
        }
        else if (num >= 0x20 && num <= 0x7e) {
            ct = alphabet[text[i] - 32];
        }
        else ct = alphabet[0];
        SDL_Rect src = { 0, 0, ct.w, ct.h };
        SDL_Rect dst = { x, y, ct.w, ct.h };
        SDL_SetTextureColorMod(ct.texture, R, G, B);
        SDL_RenderCopy(renderer, ct.texture, &src, &dst);
        x += ct.w;
        i++;
    }
}

void txt_drawText(char* text, int x, int y) {
    txt_drawTextColored(text, x, y, 255, 255, 255);
}

void txt_drawInt(int nb, int x, int y) {
    char string[32];
    sprintf(string, "%d", nb);
    txt_drawText(string, x, y);
}

void txt_drawLong(long nb, int x, int y) {
    char string[32];
    sprintf(string, "%ld", nb);
    txt_drawText(string, x, y);
}

void txt_drawFloat(double nb, int x, int y) {
    char string[32];
    sprintf(string, "%f", nb);
    txt_drawText(string, x, y);
}

static void makeCharTextures(int min, int max, int index, int* n, char* letter, SDL_Color color, SDL_Color bgColor, TTF_Font* font, int renderMode) {
    for (int i = 0; i <= max - min; i++) {

        letter[index] = i + min;
        SDL_Surface* letterSurface;

        if (renderMode == TEXT_SOLID) letterSurface = TTF_RenderUTF8_Solid(font, letter, color);
        else if (renderMode == TEXT_SHADED) letterSurface = TTF_RenderUTF8_Shaded(font, letter, color, bgColor);
        else letterSurface = TTF_RenderUTF8_Blended(font, letter, color);

        if (letterSurface == NULL) {
            printf("Failed to create letter surface: %s\n", TTF_GetError());
            exit(1);
        }
        alphabet[*n].texture = SDL_CreateTextureFromSurface(renderer, letterSurface);
        if (alphabet[*n].texture == NULL) {
            printf("Failed to create letter texture\n");
            exit(1);
        }
        alphabet[*n].w = letterSurface->w;
        alphabet[*n].h = letterSurface->h;
        SDL_FreeSurface(letterSurface);
        *n = *n + 1;
    }
}

void txt_initTextLoader(int renderMode) {
    alphabet_length = (0xc3bf - 0xc380) + (0xc2bf - 0xc2a0) + (0x7e - 0x20) + 3;
    alphabet = (charTexture*) malloc(sizeof(charTexture) * alphabet_length);
    int n = 0;

    if (TTF_Init() != 0) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(1);
    }

    TTF_Font* font;

    font = TTF_OpenFont("ressource/font/VeraMono.ttf", 24);

    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        exit(1);
    }

    SDL_Color color = { 255, 255, 255 };
    SDL_Color bgColor = { 10, 10, 10 };

    char* letter = malloc(sizeof(char) * 3);
    letter[1] = 0;
    letter[2] = 0;

    makeCharTextures(0x20, 0x7e, 0, &n, letter, color, bgColor, font, renderMode);
    letter[0] = (char) 0xc2;
    makeCharTextures(0xa0, 0xbf, 1, &n, letter, color, bgColor, font, renderMode);
    letter[0] = (char) 0xc3;
    makeCharTextures(0x80, 0xbf, 1, &n, letter, color, bgColor, font, renderMode);

    free(letter);

    TTF_CloseFont(font);
    TTF_Quit();
}