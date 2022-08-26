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

} // namespace nes