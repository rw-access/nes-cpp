#include "game.h"


namespace nes {

InteractiveConsole::~InteractiveConsole() {
    this->console->RegisterAudioCallback(nullptr);

    if (this->audioDeviceID)
        SDL_CloseAudioDevice(this->audioDeviceID);

    if (this->surface != nullptr)
        SDL_FreeSurface(this->surface);

    if (this->window != nullptr)
        SDL_DestroyWindow(this->window);
}

InteractiveConsole::InteractiveConsole(const std::string &romPath) {
    auto mapper   = nes::LoadRomFile(romPath);
    this->console = Console::Create(std::move(mapper));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        throw std::runtime_error(std::string("could not initialize sdl2: ").append(SDL_GetError()));

    SDL_AudioSpec desired{
            .freq     = 48000,
            .format   = AUDIO_F32SYS,
            .channels = 1,
            .samples  = 0,
            .size     = 0,
            .callback = nullptr,
            .userdata = nullptr,
    };

    this->audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &desired, &this->audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audioDeviceID <= 0) {
        throw std::runtime_error(std::string("could open SDL2 device: ").append(SDL_GetError()));
    }

    this->window = SDL_CreateWindow("nes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2 * nes::PPU::SCREEN_WIDTH,
                                    2 * nes::PPU::SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (this->window == nullptr) {
        throw std::runtime_error(std::string("could not create window: ").append(SDL_GetError()));
    }

    this->surface = SDL_GetWindowSurface(this->window);
    if (this->surface == nullptr) {
        throw std::runtime_error(std::string("could not create surface: ").append(SDL_GetError()));
    }


    const auto copyAudioDevice          = this->audioDeviceID;
    nes::ProcessAudioSamples queueAudio = [=](float samples[], size_t n) {
        const static size_t bytesPerSample = sizeof(*samples);
        SDL_QueueAudio(copyAudioDevice, samples, n * bytesPerSample);
    };

    this->console->RegisterAudioCallback(queueAudio);

    // resume audio
    SDL_PauseAudio(false);
    SDL_PauseAudioDevice(this->audioDeviceID, false);

    // poll for input
    SDL_Event ignored;
    SDL_PollEvent(&ignored);

    this->lastTick = std::chrono::high_resolution_clock::now();
}

void InteractiveConsole::StepAndDraw() {
    this->console->StepFrame();

    SDL_LockSurface(this->surface);
    console->DrawFrame(this->surface, scaling);
    SDL_UnlockSurface(this->surface);

    if (SDL_UpdateWindowSurface(this->window) < 0) {
        std::cout << "Failed to update window: " << SDL_GetError();
    }
}

void InteractiveConsole::FrameLimit() {
    const static auto frameTime = std::chrono::duration<double, std::micro>(1000000.0 / 60.0);

    std::this_thread::sleep_until(this->lastTick + frameTime);
    this->lastTick = std::chrono::high_resolution_clock::now();
}

void InteractiveConsole::HandleInteraction() {
    // A, B, Select, Start, Up, Down, Left, Right
    const static SDL_Scancode keyBindings[] = {
            SDL_SCANCODE_K, SDL_SCANCODE_J, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD,
            SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A,     SDL_SCANCODE_D,
    };

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                this->quit = true;
                return;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                for (auto i = 0; i < 8; i++) {
                    if (keyBindings[i] == e.key.keysym.scancode)
                        this->console->SetButton(nes::Buttons(i), e.type == SDL_KEYDOWN);
                }
                break;
        }
    }
}

void InteractiveConsole::Loop() {
    while (!this->quit) {
        this->StepAndDraw();
        this->HandleInteraction();
        this->FrameLimit();
    }
}

void InteractiveConsole::UpdateButtons(std::bitset<8> buttonStates) {
    for (size_t b = 0; b < 8; b++)
        this->console->SetButton(static_cast<Buttons>(b), buttonStates[b]);
}

bool InteractiveConsole::Done() const {
    return this->quit;
}
} // namespace nes