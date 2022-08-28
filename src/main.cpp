#include "cpu.h"
#include "nes.h"
#include "rom.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>


int main() {
    const char game[] = "tests/roms/nestest.nes";
    auto mapper       = nes::LoadRomFile(game);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "could not initialize sdl2" << SDL_GetError();
        return 1;
    }

    const auto bitsPerPx     = 4;
    const auto imagesPerLine = 32;
    const auto numImages     = mapper->cartridge->chrROM.size() / 16;
    const auto imagesTall    = numImages / imagesPerLine;

    auto width               = 1 + (1 + 8 * bitsPerPx) * imagesPerLine;
    auto height              = 1 + (1 + 8 * bitsPerPx) * imagesTall;

    auto window = SDL_CreateWindow("hello_sdl2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                   SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "could not create window" << SDL_GetError();
        return 2;
    }

    auto screenSurface = SDL_GetWindowSurface(window);
    if (screenSurface == nullptr) {
        std::cout << "could not get window surface" << SDL_GetError();
        return 3;
    }

    uint32_t *pixel32 = static_cast<uint32_t *>(screenSurface->pixels);
    auto &chr         = mapper->cartridge->chrROM;

    for (int img = 0; img < numImages; img++) {
        for (int y = 0; y < 8; y++) {
            auto imgCol = img % imagesPerLine;
            auto imgRow = img / imagesPerLine;

            for (int x = 0; x < 8; x++) {
                // 16 bytes per tile = 128 bits
                // planeOne = offset + [0 ... 7]
                // planeTwo = planeOne + 8
                auto tileStartByte = img * 16;
                auto planeOne      = (chr[tileStartByte + y] >> (7 - x)) & 1;
                auto planeTwo      = (chr[tileStartByte + y + 8] >> (7 - x)) & 1;

                auto startX        = 1 + imgCol * (1 + bitsPerPx * 8) + bitsPerPx * x;
                auto startY        = 1 + imgRow * (1 + bitsPerPx * 8) + bitsPerPx * y;

                // convert coordinates to the screen
                auto intensity = planeOne << 7 | planeTwo << 6;
                auto color     = SDL_MapRGBA(screenSurface->format, intensity, intensity, intensity, 0xff);

                for (auto dx = 0; dx < bitsPerPx; dx++) {
                    for (auto dy = 0; dy < bitsPerPx; dy++) {
                        pixel32[(startY + dy) * width + (startX + dx)] = color;
                    }
                }
            }
        }
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
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
