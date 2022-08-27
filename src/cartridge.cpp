#include "cartridge.h"

namespace nes {

Mapper::Mapper(nes::PCartridge &&c) :
    cartridge(std::move(c)) {
}

Byte Mapper::Read(nes::Address addr) const {
    auto wordPtr = this->decodeAddress(addr);
    if (nullptr == wordPtr)
        return 0;

    return *wordPtr;
}

void Mapper::Write(nes::Address addr, nes::Byte data) {
    auto wordPtr = const_cast<Byte *>(this->decodeAddress(addr));
    if (nullptr != wordPtr)
        *wordPtr = data;
}

class INESMapper0 : public Mapper {
    using Mapper::Mapper;

protected:
    const Byte *decodeAddress(Address addr) const {
        // TODO: add bounds checking or guarantee minimum sizes
        // Mapper specific details at https://www.nesdev.org/wiki/NROM

        // PPU reads only 0x0000 - 0x3EFF
        // https://www.nesdev.org/wiki/PPU_memory_map
        if (addr < 0x2000) {
            return &this->cartridge->chrROM[addr];
        } else if (addr < 0x4000) {
            // $0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
            // $2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring configuration controlled by the cartridge, but it can be partly or fully remapped to RAM on the cartridge, allowing up to 4 simultaneous nametables.
            // $3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this address range, so this space has negligible utility.
            // $3F00-3FFF is not configurable, always mapped to the internal palette control.
            return nullptr;
        }

        // CPU reads only 0x4020 - 0xFFFF from the cartridge
        // https://www.nesdev.org/wiki/CPU_memory_map
        if (addr < 0x6000) {
            return nullptr;
        } else if (addr < 0x8000) {
            // CPU $6000-$7FFF: Family Basic only: PRG RAM, mirrored as necessary to fill entire 8 KiB window, write protectable with an external switch
            auto offset = addr - 0x6000;
            if (!this->cartridge->prgRAM.empty())
                return &this->cartridge->prgRAM[offset % this->cartridge->prgRAM.size()];
        } else if (addr < 0xC000) {
            // CPU $8000-$BFFF: First 16 KB of ROM.
            auto offset = addr - 0x8000;
            if (offset < this->cartridge->prgROM.size())
                return &this->cartridge->prgROM[offset];
        } else {
            // CPU $C000-$FFFF: Last 16 KB of ROM (NROM-256) or mirror of $8000-$BFFF (NROM-128).
            auto offset = addr - 0xC000;

            if (offset < this->cartridge->prgROM.size()) {
                auto last16KBStart = (this->cartridge->prgROM.size() - 1) & ~0x3fff;
                auto prgAddr       = last16KBStart | (offset & 0x3fff);
                return &this->cartridge->prgROM[prgAddr];
            }
        }

        return nullptr;
    }
};

std::unique_ptr<Mapper> Mapper::Create(nes::MapperType mapperType, nes::PCartridge &&cart) {
    switch (mapperType) {
        case MapperType::INESMapper000:
            return std::make_unique<INESMapper0>(std::move(cart));
    }

    throw std::runtime_error("Unknown mapper type!");
}

} // namespace nes