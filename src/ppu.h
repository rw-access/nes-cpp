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
        bool tallSprites                   : 1;
        bool ppuMasterSelect               : 1;
        bool enableNMI                     : 1;
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

union PPUSTATUS {
    struct {
        Byte PPUOpenBus     : 5;
        bool spriteOverflow : 1;
        bool spriteZeroHit  : 1;
        bool nmiOccurred    : 1;
    };
    Byte raw;
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

union VRAMAddress {
    struct {
        Address coarseX         : 5;
        Address coarseY         : 5;
        Address nameTableSelect : 2;
        Address fineY           : 3;
    };
    Address raw : 15;

    void incrementX();
    void incrementY();

    void copyX(const VRAMAddress &other);
    void copyY(const VRAMAddress &other);
};

struct Sprite {
    Byte yPosTop; // byte 0
    union {
        struct {
            Byte bank          : 1;
            Byte tileNumberTop : 7;
        };
        Byte raw;
    } tileIndex; // byte 1
    struct {
        Byte palette                  : 2;
        Byte                          : 3;
        bool priorityBehindBackground : 1;
        bool flipHorizontal           : 1;
        bool flipVertical             : 1;
    } attributes;  // byte 2
    Byte xPosLeft; // byte 3

    bool empty() const;
};

struct TileData {
    Byte nameTableIndex;
    Byte palette;
    Byte patternLow;
    Byte patternHigh;

    Byte color(uint8_t x) const;
};

struct ProcessedSprite {
    Sprite sprite;
    TileData tile;

    Byte color(uint8_t x) const;
};

static_assert(sizeof(PPUCTRL) == 1);
static_assert(sizeof(PPUMASK) == 1);
static_assert(sizeof(PPUSTATUS) == 1);
static_assert(sizeof(PPUAddress) == 2);
static_assert(sizeof(VRAMAddress) == 2);
static_assert(sizeof(Sprite) == 4);

class PPU {
public:
    static const int SCREEN_WIDTH  = 256;
    static const int SCREEN_HEIGHT = 240;

    // Note that this is indexed by screen[y][x]
    using Screen = uint32_t[SCREEN_HEIGHT][SCREEN_WIDTH];

private:
    Console &console;
    PPUCTRL ppuCtrl                   = {.raw = 0};
    PPUSTATUS status                  = {.raw = 0};
    PPUMASK ppuMask                   = {.raw = 0};
    uint16_t scanLine                 = 261; // range [0, 261]
    uint16_t cycleInScanLine          = 0;   // range [0, 340]
    uint64_t frame                    = 0;
    std::array<Byte, 256> oam         = {0}; // 64 sprites
    std::array<Byte, 32> secondaryOam = {0}; // up to 8 sprites on the current line
    std::array<Byte, 32> paletteRam   = {0};
    std::array<Byte, 2048> nametables = {0};
    std::array<ProcessedSprite, 8> processedSprites; // after secondary is populated and tiles are fetched
    TileData pendingTile;
    std::array<TileData, 2> processedTiles;
    bool spriteZeroInLine = false;
    Screen screenBuffers[2];

    Byte oamAddr             = 0;
    Byte bufferedData        = 0;
    VRAMAddress vramAddr     = {.raw = 0};
    VRAMAddress tempVramAddr = {.raw = 0};
    Byte fineXScroll : 3     = 0;
    bool writeToggle         = 0;
    bool inVBlank            = false;

    Byte read(Address addr) const;
    void write(Address addr, Byte data);


    void stepVisible();
    void stepPreRender();
    void stepPostRender();
    void stepVBlank();

public:
    PPU(Console &c);

    std::array<Sprite, 64> &primarySprites();
    std::array<Sprite, 8> &secondarySprites();

    uint64_t currentFrame() const;
    const Screen &completedScreen() const;

    Byte readRegister(Address addr);
    void writeRegister(Address addr, Byte data);
    void writeDMA(const Byte *page);
    void step();
    void updateCycle();
    void updateVRAMAddr();
    void fetchBackgroundTile();
    void reset();
};

} // namespace nes