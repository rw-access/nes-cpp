#pragma once
#include "console.h"
#include "nes.h"

namespace nes {

enum class PPURegister : uint8_t {
    PPUCTRL = 0,   // CPU @ 0x2000
    PPUMASK = 1,   // CPU @ 0x2001
    PPUSTATUS = 2, // CPU @ 0x2002
    OAMADDR = 3,   // CPU @ 0x2003
    OAMDATA = 4,   // CPU @ 0x2004
    PPUSCROLL = 5, // CPU @ 0x2005
    PPUADDR = 6,   // CPU @ 0x2006
    PPUDATA = 7,   // CPU @ 0x2007
};

union PPUCTRL {
    struct {
        uint8_t baseNameTable : 2;
        uint8_t vramAddressIncrement : 1;
        uint8_t spritePatternTableAddress : 1;
        uint8_t backgroundPatternTableAddress : 1;
        uint8_t spriteSize : 1;
        bool ppuMasterSelect : 1;
        bool nmiOn : 1;
    };
    struct {
        uint8_t add256X : 1;
        uint8_t add240Y : 1;
    };
};

struct PPUMASK {
    bool greyscale : 1;
    bool showBackgroundLeft8 : 1;
    bool showSpritesLeft8 : 1;
    bool showBackground : 1;
    bool showSprites : 1;
    bool boostRed : 1;   // green on PAL/Dendy
    bool boostGreen : 1; // red on PAL/Dendy
    bool boostBlue : 1;
};

struct PPUSTATUS {
    uint8_t PPUOpenBus : 5;
    bool spriteOverflow : 1;
    bool spriteZeroHit : 1;
    bool inVBlank : 1;
};

struct PPUAddress {
    Address fineY : 3;
    Address bitPlane : 1;
    Address tileColumn : 4;
    Address tileRow : 4;
    Address patternTableLR : 1;
    Address patternTableInLower8KB : 1;
    Address : 2;
};

struct Sprite {
    uint8_t yPosTop; // byte 0
    struct {
        uint8_t bank : 1;
        uint8_t tileNumberTop : 7;
    } TileIndex; // byte 1
    struct {
        uint8_t palette : 2;
        uint8_t : 3;
        bool priorityBehindBackground : 1;
        bool flipHorizontal : 1;
        bool flipVertical : 1;
    } Attributes;     // byte 2
    uint8_t xPosLeft; // byte 3
};

static_assert(sizeof(PPUCTRL) == 1);
static_assert(sizeof(PPUMASK) == 1);
static_assert(sizeof(PPUSTATUS) == 1);
static_assert(sizeof(PPUAddress) == 2);
static_assert(sizeof(Sprite) == 4);

class PPU {
private:
    std::array<Byte, 8> registers;
    Console &console;

    const Byte *decodeAddress(Address addr) const;

public:
    PPU(Console &console);

    Byte readRegister(PPURegister reg) const;
    Byte writeRegister(Address addr);
};

} // namespace nes