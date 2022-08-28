#pragma once
#include "console.h"
#include "nes.h"

namespace nes {


enum class PPURegister : Byte {
    PPUCTRL   = 0, // CPU @ 0x2000
    PPUMASK   = 1, // CPU @ 0x2001
    PPUSTATUS = 2, // CPU @ 0x2002
    OAMADDR   = 3, // CPU @ 0x2003
    OAMDATA   = 4, // CPU @ 0x2004
    PPUSCROLL = 5, // CPU @ 0x2005
    PPUADDR   = 6, // CPU @ 0x2006
    PPUDATA   = 7, // CPU @ 0x2007
};


union PPUCTRL {
    struct {
        Byte baseNameTable                 : 2;
        Byte vramAddressIncrement          : 1;
        Byte spritePatternTableAddress     : 1;
        Byte backgroundPatternTableAddress : 1;
        Byte spriteSize                    : 1;
        bool ppuMasterSelect               : 1;
        bool nmiOn                         : 1;
    };
    struct {
        Byte add256X : 1;
        Byte add240Y : 1;
    };
    Byte raw;
};

union PPUMASK {
    struct {
        bool greyscale           : 1;
        bool showBackgroundLeft8 : 1;
        bool showSpritesLeft8    : 1;
        bool showBackground      : 1;
        bool showSprites         : 1;
        bool boostRed            : 1; // green on PAL/Dendy
        bool boostGreen          : 1; // red on PAL/Dendy
        bool boostBlue           : 1;
    };
    Byte raw;
};

struct PPUSTATUS {
    Byte PPUOpenBus     : 5;
    bool spriteOverflow : 1;
    bool spriteZeroHit  : 1;
    bool inVBlank       : 1;
};

struct PPUAddress {
    Address fineY                  : 3;
    Address bitPlane               : 1;
    Address tileColumn             : 4;
    Address tileRow                : 4;
    Address patternTableLR         : 1;
    Address patternTableInLower8KB : 1;
    Address                        : 2;
};

struct Sprite {
    Byte yPosTop; // byte 0
    struct {
        Byte bank          : 1;
        Byte tileNumberTop : 7;
    } TileIndex; // byte 1
    struct {
        Byte palette                  : 2;
        Byte                          : 3;
        bool priorityBehindBackground : 1;
        bool flipHorizontal           : 1;
        bool flipVertical             : 1;
    } Attributes;  // byte 2
    Byte xPosLeft; // byte 3
};

struct TileData {
    Byte NameTableByte;
    Byte AttributeTableByte;
    Byte PatternTableLow;
    Byte PatternTableHigh;
};

static_assert(sizeof(PPUCTRL) == 1);
static_assert(sizeof(PPUMASK) == 1);
static_assert(sizeof(PPUSTATUS) == 1);
static_assert(sizeof(PPUAddress) == 2);
static_assert(sizeof(Sprite) == 4);

class PPU {
private:
    Console &console;
    PPUCTRL ctrlReg                   = {.raw = 0};
    Byte status                       = {0};
    PPUMASK ppuMask                   = {.raw = 0};
    uint16_t scanLine                 = 261; // range [0, 261]
    uint16_t cycleInScanLine          = 0;   // range [0, 340]
    uint64_t frame                    = 0;
    std::array<Byte, 256> oam         = {0}; // 64 sprites
    std::array<Byte, 8> secondaryOam  = {0}; // up to 8 sprites on the current line
    std::array<Byte, 32> paletteRam   = {0};
    std::array<Byte, 2048> nametables = {0};
    TileData pendingTile;

    uint8_t oamAddr           = 0;
    Address vramAddr     : 15 = 0;
    Address tempVramAddr : 15 = 0;
    uint8_t fineXScroll  : 3  = 0;
    bool writeToggle     : 1  = 0;

    const Byte *decodeAddress(Address addr) const;
    Byte read(Address addr) const;
    void write(Address addr, Byte data);


    void stepVisible();
    void stepPreRender();
    void stepPostRender();
    void stepVBlank();

public:
    PPU(Console &console);

    Byte readRegister(Address addr) const;
    void writeRegister(Address addr, Byte data);
    void step();
};

} // namespace nes