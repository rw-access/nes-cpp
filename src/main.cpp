#include "cpu.h"
#include "nes.h"
#include "ppu.h"
#include "rom.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>


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

int runRom(std::string romPath) {
    auto mapper = nes::LoadRomFile(romPath);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "could not initialize sdl2" << SDL_GetError();
        return 1;
    }

    SDL_AudioSpec obtained, desired{
                                    .freq     = 48000,
                                    .format   = AUDIO_F32SYS,
                                    .channels = 1,
                                    .samples  = 256,
                                    .size     = 256,
                                    .callback = nullptr,
                                    .userdata = nullptr,
                            };

    auto audioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audioDevice <= 0) {
        std::cout << "could not open sdl2 audio device" << SDL_GetError();
        return 1;
    }

    auto console           = nes::Console::Create(std::move(mapper));
    static uint8_t scaling = 2;

    std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>> window(
            SDL_CreateWindow("nes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2 * nes::PPU::SCREEN_WIDTH,
                             2 * nes::PPU::SCREEN_HEIGHT, SDL_WINDOW_SHOWN),
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

    SDL_PauseAudioDevice(audioDevice, 0);
    nes::ProcessAudioSamples queueAudio = [&](float samples[], size_t n) {
        const static size_t bytesPerSample = sizeof(*samples);
        auto resp                          = SDL_QueueAudio(audioDevice, samples, n * bytesPerSample);
        if (resp != 0) {
            (void) resp;
        }
    };
    console->RegisterAudioCallback(queueAudio);

    bool quit = false;

    // A, B, Select, Start, Up, Down, Left, Right
    const static SDL_Scancode keyBindings[] = {
            SDL_SCANCODE_K, SDL_SCANCODE_J, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD,
            SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A,     SDL_SCANCODE_D,
    };

    while (!quit) {
        auto preDrawTicks = SDL_GetTicks();

        console->StepFrame();
        console->DrawFrame(screenSurface, scaling);

        if (SDL_UpdateWindowSurface(window.get()) < 0) {
            std::cout << "could not update window" << SDL_GetError();
            return 5;
        }

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    for (auto i = 0; i < 8; i++) {
                        if (keyBindings[i] == e.key.keysym.scancode)
                            console->SetButton(nes::Buttons(i), e.type == SDL_KEYDOWN);
                    }
                    break;
            }
        }

        // hacky scheduler for now
        const auto elapsedTicks         = SDL_GetTicks() - preDrawTicks;
        const static uint32_t frameTime = 16; // 16.66666

        if (elapsedTicks < frameTime)
            SDL_Delay(frameTime - elapsedTicks);

        // maybe get a larger fraction of time
        SDL_Delay(0);
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
        return runRom(file);
    else if (command == "dump")
        return drawTiles(file);
    else
        printf("usage: %s <play|dump> <rom file>\n", argv[0]);

    return 1;
}