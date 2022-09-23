#pragma once
#include "cartridge.h"
#include "controller.h"
#include "nes.h"
#include <SDL_surface.h>

namespace nes {

using ProcessAudioSample  = std::function<void(float)>;
using ProcessAudioSamples = std::function<void(float samples[], size_t n)>;

class APU;
class CPU;
class PPU;

class Console : std::enable_shared_from_this<Console> {
    friend APU;
    friend CPU;
    friend PPU;
    friend std::unique_ptr<Console>;

#ifdef _NES_TEST
public:
#else
private:
#endif
    Controller controller;
    std::unique_ptr<Mapper> mapper;
    std::unique_ptr<APU> apu;
    std::unique_ptr<CPU> cpu;
    std::unique_ptr<PPU> ppu;

    ProcessAudioSamples processAudioSamplesFn = nullptr;

    std::array<float, 400> bufferedAudio      = {0};
    size_t samplePos                          = this->bufferedAudio.size() - 1;

    Console(std::unique_ptr<Mapper> &&);

    void bufferAudioSample(float);

public:
    static std::shared_ptr<Console> Create(std::unique_ptr<Mapper> &&);

    void StepFrame();

    void RegisterAudioCallback(ProcessAudioSamples processAudioSamplesFn);

    void DrawFrame(SDL_Surface *surface, uint8_t scaling) const;

    void SetButton(Buttons button, bool status);
};

} // namespace nes
