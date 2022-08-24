#pragma once
#include "nes.h"

namespace nes {


class BaseMemory {
public:
    uint16_t Read16(Address lowAddress) const;

    virtual uint8_t Read(Address address) const;
    virtual void Write(Address address, uint8_t data);
};

class Cartridge : public BaseMemory {
private:
    // std::vector<uint8_t> prgRom;
    // std::vector<uint8_t> chrRom;
    // std::vector<uint8_t> instRom;
    // std::vector<uint8_t> prgRam;

public:
    virtual ~Cartridge() = 0;
};

class Memory final : public BaseMemory {
private:
    std::array<uint8_t, 0x800> ram;
    std::array<uint8_t, 0x008> ppuReg;
    std::array<uint8_t, 0x0018> apu;
    std::array<uint8_t, 0x008> io;

    std::unique_ptr<Cartridge> cartridge;

public:
    Memory(std::unique_ptr<Cartridge> &&cartridge);

    uint8_t Read(Address address) const override;
    void Write(Address address, uint8_t data) override;
};

}