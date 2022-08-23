#include "nes.h"

namespace nes::mem {

Memory::Memory() :
    rawMemory() {
}

Memory::Memory(std::vector<uint8_t> &&rawMem) :
    rawMemory(std::move(rawMem)) {
}

uint8_t Memory::Read(uint16_t address) const {
    if (address < this->rawMemory.size())
        return this->rawMemory[address];

    return 0xff;
}

void Memory::Write(uint16_t address, uint8_t data) {
    if (address < this->rawMemory.size())
        this->rawMemory[address] = data;
}

uint16_t Memory::Read16(uint16_t lowAddress) const {
    // there was a bug in the original if the high byte crosses a page boundary,
    // and it instead will wrap around
    auto highAddress = (lowAddress & 0xff00) | 0x00ff & (lowAddress + 1);
    auto lowBits = this->Read(lowAddress);
    auto highBits = this->Read(highAddress);

    return highBits << 8 | lowBits;
}


} // namespace nes::mem