#pragma once
#include "nes.h"

namespace nes {

struct Cartridge {
    std::vector<Word> prgRom;
    std::vector<Word> chrRom;
    std::vector<Word> instRom;
    std::vector<Word> prgRam;
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

    Word Read(Address addr) const;
    void Write(Address addr, Word data);

    static std::unique_ptr<Mapper> Create(MapperType mapperType, PCartridge &&cart);
    // Subclasses all need to implement unmap to decode an address to the target location
    virtual const Word *unmap(Address addr) const = 0;
};

}; // namespace nes
