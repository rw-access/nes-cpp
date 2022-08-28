#include "cpu.h"
#include "nes.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "could not initialize sdl2" << SDL_GetError();
        return 1;
    }

    auto window = SDL_CreateWindow("hello_sdl2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "could not create window" << SDL_GetError();
        return 2;
    }

    auto screenSurface = SDL_GetWindowSurface(window);
    if (screenSurface == nullptr) {
        std::cout << "could not get window surface" << SDL_GetError();
        return 3;
    }

    if (SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0x00)) < 0) {
        std::cout << "could not draw on window" << SDL_GetError();
        return 4;
    }

    if (SDL_UpdateWindowSurface(window) < 0) {
        std::cout << "could not update window" << SDL_GetError();
        return 5;
    }

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                quit = true;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
