#pragma once
#include "nes.h"

namespace nes {

struct Cartridge {
    std::vector<Byte> prgRom;
    std::vector<Byte> chrRom;
    std::vector<Byte> instRom;
    std::vector<Byte> prgRam;
};

using PCartridge = std::unique_ptr<Cartridge>;

enum class MapperType : uint16_t {
    INESMapper000 = 0,
};

class Mapper {
protected:
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
