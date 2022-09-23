#include "cpu.h"
#include "game.h"
#include "nes.h"
#include "ppu.h"
#include "rom.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>
#include <thread>


int drawTiles(std::string romPath) {
    auto mapper = nes::LoadRomFile(romPath);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "could not initialize sdl2" << SDL_GetError();
        return 1;
    }

    const auto bitsPerPx     = 2;
    const auto imagesPerLine = 32;
    const auto numImages     = mapper->cartridge->chrROM.size() / 16;
    const auto imagesTall    = numImages / imagesPerLine;

    auto width               = 1 + (1 + 8 * bitsPerPx) * imagesPerLine;
    auto height              = 1 + (1 + 8 * bitsPerPx) * imagesTall;

    std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>> window(
            SDL_CreateWindow("nes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN),
            SDL_DestroyWindow);

    if (window == nullptr) {
        std::cout << "could not create window" << SDL_GetError();
        return 2;
    }

    auto screenSurface = SDL_GetWindowSurface(window.get());
    if (screenSurface == nullptr) {
        std::cout << "could not get window surface" << SDL_GetError();
        return 3;
    }

    uint32_t *pixel32 = static_cast<uint32_t *>(screenSurface->pixels);
    auto &chr         = mapper->cartridge->chrROM;

    for (size_t img = 0; img < numImages; img++) {
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

    if (SDL_UpdateWindowSurface(window.get()) < 0) {
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

    window.reset();
    SDL_Quit();
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s <play|dump> <rom file>\n", argv[0]);
        return 1;
    }

    auto command = std::string(argv[1]);
    auto file    = std::string(argv[2]);

    if (command == "play")
        nes::InteractiveConsole(file).Loop();
    else if (command == "dump")
        return drawTiles(file);
    else
        printf("usage: %s <play|dump> <rom file>\n", argv[0]);

    return 1;
}