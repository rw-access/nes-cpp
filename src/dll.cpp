#include "dll.h"
#include "game.h"
#include <stdio.h>

void *CreateInteractiveConsole(const char *path) {
    try {
        auto console = new nes::InteractiveConsole(path);
        std::cout << "loaded!" << std::endl;
        return console;
    } catch (std::runtime_error &exc) {
        std::cout << exc.what() << std::endl;
        return nullptr;
    }
}

void StepFrame(void *console) {
    static_cast<nes::InteractiveConsole *>(console)->StepAndDraw();
}

void HandleInteraction(void *console) {
    static_cast<nes::InteractiveConsole *>(console)->HandleInteraction();
}

void UpdateButtons(void *console, uint8_t buttons) {
    static_cast<nes::InteractiveConsole *>(console)->UpdateButtons(buttons);
}

void FrameLimit(void *console) {
    static_cast<nes::InteractiveConsole *>(console)->FrameLimit();
}

bool Done(void *console) {
    return static_cast<nes::InteractiveConsole *>(console)->Done();
}
