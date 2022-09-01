#pragma once
#include "nes.h"

namespace nes {

struct Cartridge {
    enum class MirroringMode {
        Horizontal            = 0,
        Vertical              = 1,
        SingleScreenLowerBank = 2,
        FourScreen            = 3,
        SingleScreenUpperBank = 2,
    };

    std::vector<Byte> prgROM; // multiple of 16 KiB
    std::vector<Byte> chrROM; // multiple of 8 KiB
    std::vector<Byte> instROM;
    std::vector<Byte> prgRAM; // multiple of 8 KiB
    std::vector<Byte> sRAM;
    MirroringMode mirroringMode;
};

using PCartridge = std::unique_ptr<Cartridge>;

enum class MapperType : uint16_t {
    INESMapper000 = 0,
    INESMapper001 = 1,
    INESMapper002 = 2,
};

class Mapper {
public:
    PCartridge cartridge;

public:
    Mapper(PCartridge &&c);
    virtual ~Mapper()                           = default;

    virtual Byte Read(Address addr) const       = 0;
    virtual void Write(Address addr, Byte data) = 0;

    static std::unique_ptr<Mapper> Create(MapperType mapperType, PCartridge &&cart);
};

}; // namespace nes
