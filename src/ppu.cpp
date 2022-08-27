#include "ppu.h"
#include "cpu.h"

namespace nes {
PPU::PPU(Console &c) :
    registers{0},
    console(c){};

const Byte *PPU::getRegister(nes::PPURegister reg) const {
    return &this->registers[uint8_t(reg) % 8];
}

Byte PPU::readRegister(nes::PPURegister reg) const {
    return *this->getRegister(reg);
}

void PPU::writeRegister(nes::PPURegister reg, Byte data) {
    auto pReg = const_cast<Byte *>(this->getRegister(reg));
    *pReg     = data;
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

} // namespace nes