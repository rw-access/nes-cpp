//
// Created by Ross Wolf on 8/25/22.
//

#include "console.h"
#include "cpu.h"
#include "ppu.h"

namespace nes {

Console::Console(std::unique_ptr<Mapper> &&m) :
    mapper(std::move(m)) {
}

std::shared_ptr<Console> Console::Create(std::unique_ptr<Mapper> &&m) {
    std::shared_ptr<Console> console(new Console(std::move(m)));

    console->ppu = std::make_unique<PPU>(*console);
    console->cpu = std::make_unique<CPU>(*console);
    return console;
}

void Console::StepFrame() {
    uint64_t prevFrame = this->ppu->currentFrame();

    while (prevFrame == this->ppu->currentFrame()) {
        uint8_t numCycles = this->cpu->step();
        for (auto cycle = 0; cycle < numCycles; cycle++) {
            this->ppu->step();
            this->ppu->step();
            this->ppu->step();
        }
    }
}


void Console::DrawFrame(SDL_Surface *surface, uint8_t scaling) const {
    auto pixels     = static_cast<uint32_t *>(surface->pixels);
    auto format     = surface->format;
    auto &screen    = this->ppu->completedScreen();

    auto pixelIndex = 0;
    for (auto y = 0; y < this->ppu->SCREEN_HEIGHT; y++) {
        for (auto x = 0; x < this->ppu->SCREEN_WIDTH; x++) {
            auto pixelRGBA = screen[y][x];

            for (int drawY = y * scaling; drawY < (y + 1) * scaling; drawY++) {
                for (int drawX = x * scaling; drawX < (x + 1) * scaling; drawX++) {
                    auto pixelIndex = drawY * this->ppu->SCREEN_WIDTH * scaling + drawX;
                    pixels[pixelIndex] =
                            SDL_MapRGBA(format, uint8_t(pixelRGBA >> 16), uint8_t(pixelRGBA >> 8), uint8_t(pixelRGBA), 0xff);
                }
            }
        }
    }
}

void Console::SetButton(Buttons button, bool status) {
    this->controller.buttons[uint8_t(button)] = status;
}

} // namespace nes