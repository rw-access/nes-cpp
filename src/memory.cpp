#include "memory.h"

namespace nes {

Memory::Memory(std::unique_ptr<Mapper> &&mapper) :
    ram{0},
    ppuReg{0},
    apu{0},
    mapper(std::move(mapper)){};

const Word *Memory::unmap(Address addr) const {

    if (addr < 0x2000)
        return &this->ram[addr % this->ram.size()];

    if (addr < 0x4000)
        return &this->ppuReg[addr % this->ppuReg.size()];

    if (addr < 0x4018)
        return &this->apu[addr - 0x4000];

    if (addr < 0x401f)
        // only enabled for CPU test mode
        return nullptr;

    return this->mapper->unmap(addr);
}

Word Memory::Read(Address addr) const {
    // https://www.nesdev.org/wiki/CPU_memory_map
    auto unmappedAddr = this->unmap(addr);
    if (unmappedAddr == nullptr)
        return 0;

    return *unmappedAddr;
}

void Memory::Write(Address addr, Word data) {
    // https://www.nesdev.org/wiki/CPU_memory_map
    auto unmappedAddr = const_cast<Word *>(this->unmap(addr));
    if (unmappedAddr != nullptr)
        *unmappedAddr = data;
}


} // namespace nes