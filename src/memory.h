#pragma once

#include "mapper.h"
#include "nes.h"

namespace nes {


class Memory final {
private:
    std::array<Word, 0x800> ram;
    std::array<Word, 0x008> ppuReg;
    std::array<Word, 0x0018> apu;
    std::array<Word, 0x008> io;
    std::unique_ptr<Mapper> mapper;

private:
    const Word *unmap(Address addr) const;

public:
    Memory(std::unique_ptr<Mapper> &&mapper);

    uint8_t Read(Address address) const;
    void Write(Address address, uint8_t data);
};

} // namespace nes