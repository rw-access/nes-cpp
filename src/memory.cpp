#include "memory.h"

namespace nes {

Memory::Memory(std::unique_ptr<Cartridge> &&cartridge) :
    ram{0},
    ppuReg{0},
    apu{0},
    cartridge(std::move(cartridge)){};

uint8_t Memory::Read(Address addr) const {
    // https://www.nesdev.org/wiki/CPU_memory_map
    if (addr < 0x2000)
        return this->ram[addr % this->ram.size()];

    if (addr < 0x4000)
        return this->ppuReg[addr % this->ppuReg.size()];

    if (addr < 0x4018)
        return this->apu[addr - 0x4000];

    if (addr < 0x401f)
        // only enabled for CPU test mode
        return 0;

    return this->cartridge->Read(addr);
}

void Memory::Write(Address addr, uint8_t data) {
    // https://www.nesdev.org/wiki/CPU_memory_map
    if (addr < 0x2000)
        this->ram[addr % this->ram.size()] = data;
    else if (addr < 0x4000)
        this->ppuReg[addr % this->ppuReg.size()] = data;
    else if (addr < 0x4018)
        this->apu[addr - 0x4000] = data;
    else if (addr < 0x401f)
        // only enabled for CPU test mode
        return;
    else
        this->cartridge->Write(addr, data);
}

uint16_t BaseMemory::Read16(uint16_t lowAddress) const {
    // there was a bug in the original if the high byte crosses a page boundary,
    // and it instead will wrap around
    auto highAddress = (lowAddress & 0xff00) | 0x00ff & (lowAddress + 1);
    auto lowBits = this->Read(lowAddress);
    auto highBits = this->Read(highAddress);

    return highBits << 8 | lowBits;
}


} // namespace nes