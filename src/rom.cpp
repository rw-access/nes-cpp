#include "nes.h"
#include <errno.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

namespace nes {

// https://www.nesdev.org/wiki/INES
struct inesHeader {
    std::array<uint8_t, 4> magic;   // NES \x1a
    uint8_t prgRomSize;             // 4: Size of PRG ROM in 16 KB units
    uint8_t chrRomSize;             // 5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
    bool mirrorMode : 1;            // 6 0
    bool hasBattery : 1;            // 6 1
    bool hasTrainer : 1;            // 6 2
    bool ignoreMirroring : 1;       // 6 3
    uint8_t mapperLo : 4;           // 6 4..7
    bool vsUnisystem : 1;           // 7 0
    bool playChoice10 : 1;          // 7 1
    uint8_t nes2_0 : 2;             // 7 2..3
    uint8_t mapperHi : 4;           // 7 4..6
    uint8_t prgRamSize;             // 8
    bool pal : 1;                   // 9 1
    uint8_t : 7;                    // 9 2..7
    uint8_t tvSystemPrgRamPresence; // 10
    uint8_t padding[5];             // 11-15
};

// https://www.nesdev.org/wiki/NES_2.0
struct nes2Header {
    std::array<uint8_t, 4> magic; // 0-3: NES \x1a
    uint8_t prgRomSizeLo;         // 4: Size of PRG ROM in 16 KB units
    uint8_t chrSizeLo;            // 5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
    bool mirrorMode : 1;          // 6 0
    bool hasBattery : 1;          // 6 1
    bool hasTrainer : 1;          // 6 2
    bool ignoreMirroring : 1;     // 6 3
    uint8_t mapperLo : 4;         // 6 4..7
    enum {
        Famicom,
        VsSystem,
        Playchoice10,
        ExtendedConsole,
    } consoleType : 2;        // 7 0..1
    uint8_t nes2_0 : 2;       // 7 2..3
    uint8_t mapperMid : 4;    // 7 4..7
    uint8_t mapperHi : 4;     // 8 0..3
    uint8_t submapper : 4;    // 8 4..7
    uint8_t prgRomSizeHi : 4; // 9 0..3
    uint8_t chrRomSizeHi : 4; // 9 4..7

    // shift counts mean 64 << N bytes in size
    uint8_t prgRamShiftCount : 4;    // 10 0..3
    uint8_t prgNVRAMShiftCount : 4;  // 10 4..7
    uint8_t chrRamShiftCount : 4;    // 11 0..3
    uint8_t chrEEPROMShiftCount : 4; // 11 4..7

    enum {
        RP2C02,
        RP2C07,
        MultipleRegion,
        UMC6527P,
    } cpuTiming : 2; // 12 0..1
    uint8_t : 6;     // 12 2..7

    union {
        struct {
            uint8_t ppuType : 4;      // 13 0..3
            uint8_t hardwareType : 4; // 13 4..7
        };                            // when consoleType == VsSystem

        struct {
            uint8_t extendedConsoleType : 4; // 13 0..3
            uint8_t : 4;                     // 13 4..7
        };                                   // when consoleType == ExtendedConsole
    } tvSystemPrgRamPresence;                // 13

    uint8_t miscellaneousRoms : 2;      // 14 0..1
    uint8_t : 6;                        // 14 2..7
    uint8_t defaultExpansionDevice : 6; // 15 0..5
    uint8_t : 2;                        // 15 6..7
};


struct rom {
    std::vector<uint8_t> prg;
    std::vector<uint8_t> chr;
    uint8_t mapperNum;
};

const std::array<uint8_t, 4> expectedMagic{'N', 'E', 'S', '\x1a'};

union anyHeader {
    inesHeader ines;
    nes2Header nes2;
};


static_assert(sizeof(inesHeader) == 16, "unexpected inesHeader size");
static_assert(sizeof(nes2Header) == 16, "unexpected nes2Header size");
static_assert(sizeof(anyHeader) == 16, "unexpected anyHeader size");

class PlainCartridge : public mem::Cartridge {
private:
    std::vector<uint8_t> prgRom;
    std::vector<uint8_t> chrRom;
    // std::vector<uint8_t> instRom;
    //    std::vector<uint8_t> prgRam;

public:
    PlainCartridge(std::vector<uint8_t> &&prgRom, std::vector<uint8_t> &&chrRom
                   //                   std::vector<uint8_t> &&prgRam)
                   ) :
        prgRom(std::move(prgRom)),
        chrRom(std::move(chrRom))
        //        prgRam(std::move(prgRam))
        {};

    uint8_t Read(mem::Address addr) const {
        if (addr < 0x6000)
            return 0;

        if (addr < 0x8000)
            // no prg RAM attached
            return 0;

        if (addr < 0xC000)
            return 0;

        return 0;
    };

    void Write(mem::Address addr, uint8_t data) const;
};

std::unique_ptr<mem::Cartridge> loadAndValidateROM(const std::string &path) {
    std::ifstream romFile(path, std::ios_base::binary | std::ios_base::in);
    anyHeader hdr;

    // read directly into the inesHeader
    romFile.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    if (hdr.ines.magic != expectedMagic)
        throw std::runtime_error("Not a valid NES file");

    bool isNES2_0 = hdr.nes2.nes2_0 == 0x2;
    if (isNES2_0)
        throw std::runtime_error("TODO: support NES 2.0");

    // skip over the trainer for now
    if (hdr.ines.hasTrainer)
        romFile.seekg(romFile.tellg() * 512);

    // read PRG ROM
    std::vector<uint8_t> prgRom;
    prgRom.resize(hdr.ines.prgRomSize * 16536);
    romFile.read(reinterpret_cast<char *>(prgRom.data()), prgRom.size());

    // read CHR ROM
    std::vector<uint8_t> chrROM;
    chrROM.resize(hdr.ines.chrRomSize * 8192);
    romFile.read(reinterpret_cast<char *>(chrROM.data()), chrROM.size());

    // TODO: read INST ROM

    // TODO: read PROM

    // https://www.nesdev.org/wiki/CPU_memory_map
    // $4020-$FFFF      $BFE0   Cartridge space: PRG ROM, PRG RAM, and mapper registers (See Note)
    return std::make_unique<PlainCartridge>(std::move(prgRom), std::move(chrROM));
}
} // namespace nes