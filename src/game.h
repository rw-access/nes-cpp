#include "apu.h"
#include "cpu.h"
#include "nes.h"
#include "ppu.h"
#include "rom.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <stdio.h>
#include <thread>

namespace nes {

class InteractiveConsole {
    static const uint8_t scaling = 2;

private:
    std::shared_ptr<Console> console;
    bool quit = false;

    SDL_AudioSpec audioSpec;
    SDL_Window *window              = nullptr;
    SDL_Surface *surface            = nullptr;
    SDL_AudioDeviceID audioDeviceID = 0;

    std::chrono::steady_clock::time_point lastTick;

public:
    InteractiveConsole(const std::string &romPath);
    void Loop();
    void StepAndDraw();
    void FrameLimit();

    // A, B, Select, Start, Up, Down, Left, Right
    void UpdateButtons(std::bitset<8> buttonStates);
    void HandleInteraction();
    bool Done() const;
    ~InteractiveConsole();
};
} // namespace nes