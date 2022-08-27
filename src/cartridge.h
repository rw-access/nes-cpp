#pragma once
#include "nes.h"

namespace nes {

struct Cartridge {
    enum class MirroringMode {
        Horizontal   = 0,
        Vertical     = 1,
        SingleScreen = 2,
        FourScreen   = 3,
    };

    std::vector<Byte> prgROM; // multiple of 16 KiB
    std::vector<Byte> chrROM; // multiple of 8 KiB
    std::vector<Byte> instROM;
    std::vector<Byte> prgRAM; // multiple of 8 KiB
    MirroringMode mirroringMode;
};

using PCartridge = std::unique_ptr<Cartridge>;

enum class MapperType : uint16_t {
    INESMapper000 = 0,
};

class Mapper {
public:
    PCartridge cartridge;

public:
    Mapper(PCartridge &&c);
    virtual ~Mapper() = default;

    Byte Read(Address addr) const;
    void Write(Address addr, Byte data);

    static std::unique_ptr<Mapper> Create(MapperType mapperType, PCartridge &&cart);
    // Subclasses all need to implement decodeAddress to decode an address to the target location
    virtual const Byte *decodeAddress(Address addr) const = 0;
};

}; // namespace nes
