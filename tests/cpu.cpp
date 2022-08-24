#include "../src/cpu.h"
#include "../src/nes.h"
#include "../src/rom.h"
#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(NESTest, NesTestRom) {
    auto mapper = nes::LoadROM("roms/nestest.nes");
    auto memory = std::make_unique<nes::Memory>(std::move(mapper));
    auto cpu = nes::CPU(std::move(memory));

    cpu.PC(0xc000);

    for (uint64_t i = 0; i < 8000; i++) {
        uint8_t numCycles = cpu.step();
        (void) numCycles;
    }
}
