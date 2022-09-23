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

    std::vector<Byte> prgROM; // multiple of 16 KiB / 0x4000
    std::vector<Byte> chrROM; // multiple of 8 KiB  / 0x2000
    std::vector<Byte> prgRAM; // multiple of 8 KiB  / 0x2000
    std::vector<Byte> sRAM;
    MirroringMode mirroringMode;
};

using PCartridge = std::unique_ptr<Cartridge>;

enum class MapperType : uint16_t {
    INESMapper000 = 0,
    INESMapper001 = 1,
    INESMapper002 = 2,
    INESMapper004 = 4,
};

class Mapper {
private:
    bool pendingIRQ = false;

protected:
    void triggerIRQ();

public:
    PCartridge cartridge;

    Mapper(PCartridge &&c);
    virtual ~Mapper() = default;
    bool CheckIRQ();

    virtual Byte Read(Address addr) const            = 0;
    virtual const Byte *DMAStart(Address addr) const = 0;
    virtual void Write(Address addr, Byte data)      = 0;
    virtual void OnScanline();

    static std::unique_ptr<Mapper> Create(MapperType mapperType, PCartridge &&cart);
};

}; // namespace nes
