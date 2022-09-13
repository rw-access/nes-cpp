#include "console.h"
#include "apu.h"
#include "cpu.h"
#include "ppu.h"

namespace nes {

Console::Console(std::unique_ptr<Mapper> &&m) :
    mapper(std::move(m)) {
}

std::shared_ptr<Console> Console::Create(std::unique_ptr<Mapper> &&m) {
    std::shared_ptr<Console> console(new Console(std::move(m)));
    auto &consoleRef = *console;

    console->apu     = std::make_unique<APU>(*console);
    console->ppu     = std::make_unique<PPU>(*console);
    console->cpu     = std::make_unique<CPU>(*console);

    console->apu->registerAudioCallback(std::bind(&Console::bufferAudioSample, console.get(), std::placeholders::_1));
    return console;
}

void Console::StepFrame() {
    uint64_t prevFrame = this->ppu->currentFrame();

    while (prevFrame == this->ppu->currentFrame()) {
        auto numCycles = this->cpu->step();
        for (auto cycle = 0; cycle < numCycles; cycle++) {
            this->apu->step();

            this->ppu->step();
            this->ppu->step();
            this->ppu->step();
        }
    }
}


void Console::DrawFrame(SDL_Surface *surface, uint8_t scaling) const {
    auto pixels  = static_cast<uint32_t *>(surface->pixels);
    auto format  = surface->format;
    auto &screen = this->ppu->completedScreen();

    for (auto y = 0; y < this->ppu->SCREEN_HEIGHT; y++) {
        for (auto x = 0; x < this->ppu->SCREEN_WIDTH; x++) {
            auto pixelRGBA = screen[y][x];

            for (int drawY = y * scaling; drawY < (y + 1) * scaling; drawY++) {
                for (int drawX = x * scaling; drawX < (x + 1) * scaling; drawX++) {
                    auto pixelIndex    = drawY * this->ppu->SCREEN_WIDTH * scaling + drawX;
                    pixels[pixelIndex] = SDL_MapRGBA(format, uint8_t(pixelRGBA >> 16), uint8_t(pixelRGBA >> 8),
                                                     uint8_t(pixelRGBA), 0xff);
                }
            }
        }
    }
}

void Console::SetButton(Buttons button, bool status) {
    this->controller.buttons[uint8_t(button)] = status;
}

void Console::bufferAudioSample(float sample) {
    this->bufferedAudio[this->samplePos] = sample;
    this->samplePos++;

    if (this->samplePos == this->bufferedAudio.size()) {
        this->samplePos = 0;

        if (this->processAudioSamplesFn != nullptr)
            this->processAudioSamplesFn(this->bufferedAudio.data(), this->bufferedAudio.size());
    }
}

void Console::RegisterAudioCallback(ProcessAudioSamples processSamplesFn) {
    this->processAudioSamplesFn = processSamplesFn;
}

} // namespace nes