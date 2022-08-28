#include "ppu.h"
#include "cpu.h"

namespace nes {
PPU::PPU(Console &c) :
    console(c){};

Byte PPU::readRegister(Address addr) const {
    switch (addr % 8) {
        case 1: // PPUCTRL: $2001
            return this->status;
        case 4: // OAMDATA: $2004
            return this->oam[this->oamAddr];
        case 7: // PPUDATA: $2007
            break;
    }

    return 0;
}

void PPU::writeRegister(Address addr, Byte data) {
    switch (addr % 8) {
        case 0: // PPUCTRL: $2000
            this->ctrlReg.raw = data;
            break;
        case 2: // PPUMASK: $2001
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
            // TODO
            break;
        case 6: // PPUADDR: $2006
            // TODO
            break;
        case 7: // PPUDATA: 2007
            // TODO
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


void PPU::stepPreRender() {
    // Pre-render scanline (-1 or 261)
}

void PPU::stepVisible() {
    if (!this->ppuMask.showBackground && !this->ppuMask.showSprites)
        return;

    if (this->cycleInScanLine == 0) {
        // idle
    } else if (this->cycleInScanLine <= 256) {
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
        // this way, it averages 1 pixel / cycle
        auto x = this->cycleInScanLine - 1;
        switch (x % 8) {
            case 2:
                break;
            case 4:
                break;
            case 6:
                break;
            case 7:
                break;
        }
    }
}

void PPU::stepPostRender() {
}


void PPU::stepVBlank() {
}

void PPU::step() {
    // https://www.nesdev.org/wiki/PPU_rendering#Line-by-line_timing
    if (this->scanLine <= 239) {
        this->stepVisible();
    } else if (this->scanLine == 240) {
        this->stepPreRender();
    } else if (this->scanLine <= 260) {
        this->stepVBlank();
    } else {
        this->stepPreRender();
    }

    if (this->cycleInScanLine == 340) {
        this->cycleInScanLine = 0;

        if (this->scanLine == 261) {
            this->scanLine = 0;
            this->frame++;
            this->cycleInScanLine += (this->frame & 1); // skip the first frame for odd frames
        } else {
            this->scanLine++;
        }

    } else {
        this->cycleInScanLine++;
    }
}

} // namespace nes