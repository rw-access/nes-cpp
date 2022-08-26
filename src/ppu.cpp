#include "ppu.h"
#include "cpu.h"

namespace nes {
PPU::PPU(Console &c) :
    registers{0},
    console(c){};

}