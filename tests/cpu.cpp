#include <gtest/gtest.h>

#define _NES_TEST
#include "../src/console.h"
#include "../src/cpu.h"
#include "../src/rom.h"

// Demonstrate some basic assertions.
TEST(NESTest, NesTestRom) {
    auto mapper  = nes::LoadRomFile("roms/nestest.nes");
    auto console = nes::Console::Create(std::move(mapper));

    auto &cpu    = *console->cpu;
    cpu.PC(0xc000);

    for (uint64_t i = 0; i < 9000; i++) {
        uint16_t numCycles = cpu.step();
        (void) numCycles;
    }
}
