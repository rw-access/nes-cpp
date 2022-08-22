#include "nes.h"

namespace nes::mem {

Memory::Memory() :
    rawMemory() {
}

uint8_t Memory::Read(uint16_t address) const {
    if (address < this->rawMemory.size()) return this->rawMemory[address];

    return 0xff;
}

uint16_t Memory::Read16(uint16_t lowAddress) const {
    auto highAddress = (lowAddress & 0xff00) | 0x00ff & (lowAddress + 1);
    auto lowBits = this->Read(lowAddress);
    auto highBits = this->Read(highAddress);

    return highBits << 8 | lowBits;
}
} // namespace nes::mem