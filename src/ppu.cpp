#include "ppu.h"
#include "cpu.h"

namespace nes {
uint32_t colorPaletteRGBA[] = {
        0x666666, 0x002A88, 0x1412A7, 0x3B00A4, 0x5C007E, 0x6E0040, 0x6C0600, 0x561D00, 0x333500, 0x0B4800, 0x005200,
        0x004F08, 0x00404D, 0x000000, 0x000000, 0x000000, 0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE, 0xA01ACC, 0xB71E7B,
        0xB53120, 0x994E00, 0x6B6D00, 0x388700, 0x0C9300, 0x008F32, 0x007C8D, 0x000000, 0x000000, 0x000000, 0xFFFEFF,
        0x64B0FF, 0x9290FF, 0xC676FF, 0xF36AFF, 0xFE6ECC, 0xFE8170, 0xEA9E22, 0xBCBE00, 0x88D800, 0x5CE430, 0x45E082,
        0x48CDDE, 0x4F4F4F, 0x000000, 0x000000, 0xFFFEFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF, 0xFBC2FF, 0xFEC4EA, 0xFECCC5,
        0xF7D8A5, 0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC, 0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000,
};

uint8_t TileData::backgroundPixel(uint8_t x) const {
    auto loBit = (this->PatternTableLow >> (7 - x)) & 1;
    auto hiBit = (this->PatternTableHigh >> (7 - x)) & 1;
    return (hiBit << 1) | loBit;
}

void VRAMAddress::incrementX() {
    // https://www.nesdev.org/wiki/PPU_scrolling#X_increment
    this->coarseX++;

    if (this->coarseX == 0)
        this->nameTableSelect ^= 0x1;
}

void VRAMAddress::incrementY() {
    // https://www.nesdev.org/wiki/PPU_scrolling#Y_increment
    this->fineY++;

    if (this->fineY == 0) {
        // bump coarse Y on wraparound
        if (this->coarseY == 29) {
            this->coarseY = 0;
            this->nameTableSelect ^= 0x1;
        } else if (this->coarseY == 31) {
            this->coarseY = 0;
        } else {
            this->coarseY++;
        }
    }
}

void VRAMAddress::copyX(const VRAMAddress &other) {
    this->coarseX         = other.coarseX;
    this->nameTableSelect = this->nameTableSelect & 0x2 | other.nameTableSelect & 0x1;
}

void VRAMAddress::copyY(const nes::VRAMAddress &other) {
    this->coarseY         = other.coarseY;
    this->nameTableSelect = other.nameTableSelect & 0x2 | this->nameTableSelect & 0x1;
    this->fineY           = other.fineY;
}

PPU::PPU(nes::Console &c) :
    console(c) {
    this->reset();
}

void PPU::reset() {
    this->cycleInScanLine = 340;
    this->scanLine        = 240;
    this->frame           = 0;
    this->ppuCtrl.raw     = 0;
    this->ppuMask.raw     = 0;
    this->oamAddr         = 0;
}

Byte PPU::readRegister(Address addr) {
    // https://www.nesdev.org/wiki/PPU_scrolling#Register_controls
    // mirrored every 8 bytes
    Byte contents = 0;
    switch (addr % 8) {
        case 1: // PPUCTRL: $2001
            return this->status.raw;
        case 2: // PPUSTATUS: $2002
            contents              = this->status.raw;
            this->writeToggle     = 0;
            this->status.inVBlank = false;
            return contents;
        case 4: // OAMDATA: $2004
            return this->oam[this->oamAddr];
        case 7: // PPUDATA: $2007
            this->vramAddr.coarseX += (this->ppuCtrl.vramAddressIncrement ^ 0x1);
            this->vramAddr.coarseY += this->ppuCtrl.vramAddressIncrement;
            break;
    }

    return contents;
}

void PPU::writeRegister(Address addr, Byte data) {
    // https://www.nesdev.org/wiki/PPU_scrolling#Register_controls
    switch (addr % 8) {
        case 0: // PPUCTRL: $2000
            // t: ...GH.. ........ <- d: ......GH
            //    <used elsewhere> <- d: ABCDEF..
            this->ppuCtrl.raw                  = data & ~0x3;
            this->tempVramAddr.nameTableSelect = data & 0x3;
            break;
        case 1: // PPUMASK: $2001
            this->ppuMask.raw = data;
            break;
        case 3: // OAMADDR: $2003
            if (this->writeToggle == 0)
                this->oamAddr = (this->oamAddr & 0xff00) | data;
            else
                this->oamAddr = data << 8 | (this->oamAddr & 0x00ff);
            break;
        case 4: // OAMDATA: $2004
            this->oam[this->oamAddr] = data;
            this->oamAddr++;
            break;
        case 5: // PPUSCROLL: $2005
            if (!this->writeToggle) {
                // t: ....... ...ABCDE <- d: ABCDE...
                // x:              FGH <- d: .....FGH
                this->tempVramAddr.fineY = data >> 3;
                this->fineXScroll        = data & 0x7;
            } else {
                // t: FGH..AB CDE..... <- d: ABCDEFGH
                this->tempVramAddr.fineY   = data << 5;
                this->tempVramAddr.coarseY = data >> 3;
            }

            this->writeToggle = !this->writeToggle;
            break;
        case 6: // PPUADDR: $2006
            if (!this->writeToggle) {
                // t: .CDEFGH ........ <- d: ..CDEFGH
                //        <unused>     <- d: AB......
                // t: Z...... ........ <- 0 (bit Z is cleared)
                data &= 0x3f;
                this->tempVramAddr.raw = (this->tempVramAddr.raw & 0x80ff) | Address(data) << 8;
            } else {
                // t: ....... ABCDEFGH <- d: ABCDEFGH
                // v: <...all bits...> <- t: <...all bits...>
                this->tempVramAddr.raw = (this->tempVramAddr.raw & 0xff00) | data;
                this->vramAddr         = this->tempVramAddr;
            }

            this->writeToggle = !this->writeToggle;
            break;
        case 7: // PPUDATA: $2007
            this->write(this->vramAddr.raw, data);
            if (this->ppuCtrl.vramAddressIncrement == 0) {
                this->vramAddr.raw++;
            } else {
                this->vramAddr.raw += 32;
            }
            break;
    }
}

// https://www.nesdev.org/wiki/Mirroring#Nametable_Mirroring
static const uint8_t nameTableMirrorings[][4] = {
        {0, 0, 1, 1}, // Cartridge::MirroringMode::Horizontal   = 0
        {0, 0, 1, 1}, // Cartridge::MirroringMode::Vertical     = 1
        {0, 0, 0, 0}, // Cartridge::MirroringMode::SingleScreen = 3
        {0, 1, 2, 3}, // Cartridge::MirroringMode::FourScreen   = 4
};

const Byte *PPU::decodeAddress(nes::Address addr) const {
    // $0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
    // $2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring
    //            configuration controlled by the cartridge, but it can be partly or fully remapped to RAM
    //            on the cartridge, allowing up to 4 simultaneous nametables.
    // $3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this
    //            address range, so this space has negligible utility.
    // $3F00-3FFF is not configurable, always mapped to the internal palette control.
    if (addr < 0x2000) {
        return this->console.mapper->decodeAddress(addr);
    } else if (addr < 0x3f00) {
        // TODO: figure out how this is "partly or fully remapped to RAM on the cartridge"
        // $2000-$23FF  $0400  Nametable 0
        // $2400-$27FF  $0400  Nametable 1
        // $2800-$2BFF  $0400  Nametable 2
        // $2C00-$2FFF  $0400  Nametable 3
        // $3000-$3EFF  $0F00  Mirrors of $2000-$2EFF
        auto nameTableOffset = addr % 0x400;         // $3xxx / $2xxx => $0xxx
        auto nameTable       = (addr & 0xC00) >> 10; // 0, 1, 2, 3

        // unmirror the nametable from 0, 1, 2, 3 to its actual address
        auto nameTableBank = nameTableMirrorings[uint8_t(this->console.mapper->cartridge->mirroringMode)][nameTable];
        return &this->nametables[nameTableBank << 10 | nameTableOffset];
    } else {
        return &this->paletteRam[addr % 0x20];
    }
}

Byte PPU::read(Address addr) const {
    auto pByte = this->decodeAddress(addr);
    if (pByte != nullptr) {
        return *pByte;
    }
    return 0;
}


void PPU::write(Address addr, Byte data) {
    auto pByte = const_cast<Byte *>(this->decodeAddress(addr));
    if (pByte != nullptr) {
        *pByte = data;
    }
}


void PPU::fetchTile() {
    // The data for each tile is fetched during this phase. Each memory access takes 2 PPU cycles to complete,
    // and 4 must be performed per tile:
    //
    // * Nametable byte
    // * Attribute table byte
    // * Pattern table tile low
    // * Pattern table tile high (+8 bytes from pattern table tile low)
    //
    // The data fetched from these accesses is placed into internal latches, and then fed to the appropriate
    // shift registers when it's time to do so (every 8 cycles). Because the PPU can only fetch an attribute
    // byte every 8 cycles, each sequential string of 8 pixels is forced to have the same palette attribute.
    //
    // Sprite zero hits act as if the image starts at cycle 2 (which is the same cycle that the shifters shift
    // for the first time), so the sprite zero flag will be raised at this point at the earliest. Actual pixel
    // output is delayed further due to internal render pipelining, and the first pixel is output during cycle 4.
    //
    // The shifters are reloaded during ticks 9, 17, 25, ..., 257.
    //
    // Note: At the beginning of each scanline, the data for the first two tiles is already loaded into the shift
    // registers (and ready to be rendered), so the first tile that gets fetched is Tile 3.
    //
    // While all of this is going on, sprite evaluation for the next scanline is taking place as a separate process,
    // independent to what's happening here.

    // ultimately, we're retrieving and rendering a strip of 8 pixels long as a single unit
    // this way, it averages 1 pixel / cycle.

    auto x = this->cycleInScanLine - 1;

    // https://www.nesdev.org/wiki/PPU_scrolling#Tile_and_attribute_fetching
    switch (x % 8) {
        case 1:
            // everything but fine Y
            this->pendingTile.NameTableByte = this->read(0x2000 | this->vramAddr.raw & 0x0FFF);
            break;
        case 3:
            // TODO: switch to use bitfields
            this->pendingTile.AttributeTableByte =
                    this->read(0x23C0 | (this->vramAddr.raw & 0x0C00) | ((this->vramAddr.raw >> 4) & 0x38) |
                               ((this->vramAddr.raw >> 2) & 0x07));
            break;
        case 5:
            // two pattern tables: 0x0000 and 0x1000
            // xxxx xxxx xxxx xxxx
            //                 ^^^--- fine Y
            //      ^^^^ ^^^^ ------- tile
            //                0------ low byte
            //    ^ ---- ---- ------- foreground/background
            this->pendingTile.PatternTableLow =
                    this->read(this->ppuCtrl.backgroundPatternTableAddress << 12 |
                               this->pendingTile.NameTableByte << 4 | 0 << 3 | this->vramAddr.fineY);
            break;
        case 7:
            // two pattern tables: 0x0000 and 0x1000
            // xxxx xxxx xxxx xxxx
            //                 ^^^--- fine Y
            //      ^^^^ ^^^^ ------- tile
            //                1------ high byte
            //    ^ ---- ---- ------- foreground/background
            this->pendingTile.PatternTableHigh =
                    this->read(this->ppuCtrl.backgroundPatternTableAddress << 12 |
                               this->pendingTile.NameTableByte << 4 | 1 << 3 | this->vramAddr.fineY);

            this->processedTile = this->pendingTile;
            break;
    }
}


void PPU::stepPreRender() {
    if (!this->ppuMask.showBackground && !this->ppuMask.showSprites)
        return;

    // Pre-render scanline (-1 or 261)
    if (this->cycleInScanLine == 1) {
        this->status.spriteZeroHit = false;
        this->status.spriteZeroHit = false;
        this->status.inVBlank      = false;
    }

    if (this->cycleInScanLine == 0) {
        // idle
    } else if (this->cycleInScanLine <= 256) {
        this->fetchTile();
    } else if (this->cycleInScanLine >= 321 && this->cycleInScanLine <= 336) {
        // prefetch for the next line
        this->fetchTile();
    }

    this->updateVRAMAddr();
}

void PPU::stepVisible() {
    if (!this->ppuMask.showBackground && !this->ppuMask.showSprites)
        return;


    if (this->cycleInScanLine == 0) {
        // idle
    } else if (this->cycleInScanLine <= 256) {
        // visible cycle: draw a pixel
        Byte x          = this->cycleInScanLine - 1;
        Byte y          = this->scanLine;
        Byte background = this->processedTile.backgroundPixel(x % 8);

        // TODO: Add sprites!
        auto color                                 = this->paletteRam[background];
        this->screenBuffers[this->frame & 1][y][x] = colorPaletteRGBA[color];

        this->fetchTile();
    } else if (this->cycleInScanLine >= 321 && this->cycleInScanLine <= 336) {
        // prefetch for the next line
        this->fetchTile();
    }

    this->updateVRAMAddr();
}

void PPU::stepPostRender() {
}


void PPU::stepVBlank() {
    if (this->scanLine == 241 && this->cycleInScanLine == 1) {
        this->status.inVBlank = true;

        if (this->ppuCtrl.nmiOn)
            this->console.cpu->interrupt(Interrupt::NMI);
    }
}

void PPU::updateVRAMAddr() {
    if (!this->ppuMask.showBackground && !this->ppuMask.showSprites)
        return;

    if (this->cycleInScanLine == 256) {
        // https://www.nesdev.org/wiki/PPU_scrolling#At_dot_256_of_each_scanline
        this->vramAddr.incrementY();
    } else if (this->cycleInScanLine == 257) {
        // https://www.nesdev.org/wiki/PPU_scrolling#At_dot_257_of_each_scanline
        // If rendering is enabled, the PPU copies all bits related to horizontal position from t to v:
        // v: ....A.. ...BCDEF <- t: ....A.. ...BCDEF
        this->vramAddr.copyX(this->tempVramAddr);
    } else if (this->scanLine == 261 && (this->cycleInScanLine >= 280 && this->cycleInScanLine <= 304)) {
        // If rendering is enabled, at the end of vblank, shortly after the horizontal bits are copied from
        // t to v at dot 257, the PPU will repeatedly copy the vertical bits from t to v from dots 280 to 304,
        // completing the full initialization of v from t:
        // v: GHIA.BC DEF..... <- t: GHIA.BC DEF.....
        this->vramAddr.copyY(this->tempVramAddr);
    } else if (this->cycleInScanLine > 0 && (this->cycleInScanLine <= 256 | this->cycleInScanLine >= 328) &&
               this->cycleInScanLine % 8 == 0) {
        // https://www.nesdev.org/wiki/PPU_scrolling#Between_dot_328_of_a_scanline,_and_256_of_the_next_scanline
        // If rendering is enabled, the PPU increments the horizontal position in v many times across the scanline,
        // it begins at dots 328 and 336, and will continue through the next scanline at 8, 16, 24... 240, 248, 256
        // (every 8 dots across the scanline until 256). Across the scanline the effective coarse X scroll coordinate
        // is incremented repeatedly, which will also wrap to the next nametable appropriately
        this->vramAddr.incrementX();
    }
}

void PPU::updateCycle() {
    if (this->cycleInScanLine == 340) {
        this->cycleInScanLine = 0;

        if (this->scanLine == 261) {
            this->scanLine = 0;
            this->frame++;
            //            this->cycleInScanLine += (this->frame & 1); // skip the first cycle for odd frames
        } else {
            this->scanLine++;
        }

    } else {
        this->cycleInScanLine++;
    }
}

void PPU::step() {
    this->updateCycle();

    // https://www.nesdev.org/wiki/PPU_rendering#Line-by-line_timing
    if (this->scanLine <= 239) {
        this->stepVisible();
    } else if (this->scanLine == 240) {
        this->stepPostRender();
    } else if (this->scanLine <= 260) {
        this->stepVBlank();
    } else {
        this->stepPreRender();
    }
}

uint64_t PPU::currentFrame() const {
    return this->frame;
}

const PPU::Screen &PPU::completedScreen() const {
    return this->screenBuffers[this->frame & 0x1];
}

} // namespace nes