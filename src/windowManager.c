//#include <glad/glad.h>
//#include <SDL2/SDL.h>
#include "windowManager.h"
#define DEFAULT_WIDTH 1080
#define DEFAULT_HEIGHT 720

static int isFullScreen;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_GLContext context;

int screen_w;
int screen_h;

int cameraX, cameraY; // cameraPos

static void initMainScreen(const char* name, int win_opt, int ren_opt) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);  // Make sure SDL_Quit gets called when the program exits 

    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_3_3);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_3_3);
    // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_w, screen_h, SDL_WINDOW_OPENGL | win_opt);
    if (window == NULL) {
        printf("Unable to set video mode: %s\n", SDL_GetError());
        exit(1);
    }

    context = SDL_GL_CreateContext(window);
    if (!context) {
        fprintf(stderr, "Couldn't create context: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_GetWindowSize(window, &screen_w, &screen_h);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | ren_opt);
    // if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))  {
    //     fprintf(stderr, "Failed to initialize GLAD");
    //     exit(1);
    // }

    // glViewport(0, 0, screen_w, screen_h);
}

void win_init(const char* name, int count, char* arg[]) {
    isFullScreen = 0;
    cameraX = 0; cameraY = 0;
    screen_w = DEFAULT_WIDTH;
    screen_h = DEFAULT_HEIGHT;
    // TODO : parse arguments
    initMainScreen(name, 0, 0);
}

void win_toggleFullScreen() {
    if (isFullScreen) {
        screen_w = DEFAULT_WIDTH;
        screen_h = DEFAULT_HEIGHT;
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, screen_w, screen_h);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        isFullScreen = 0;
    }
    else {
        SDL_ShowCursor(SDL_DISABLE);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_GetWindowSize(window, &screen_w, &screen_h);
        isFullScreen = 1;
    }
}


void win_swapBuffer() {
    SDL_RenderPresent(renderer);
}

void win_clear() {
    SDL_RenderClear(renderer);
}