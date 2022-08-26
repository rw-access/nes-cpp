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
        if (addr < 0x6000) {
            return nullptr;
        } else if (addr < 0x8000) {
            // CPU $6000-$7FFF: Family Basic only: PRG RAM, mirrored as necessary to fill entire 8 KiB window, write protectable with an external switch
            auto offset = addr - 0x6000;
            if (!this->cartridge->prgRam.empty())
                return &this->cartridge->prgRam[offset % this->cartridge->prgRom.size()];
        } else if (addr < 0xC000) {
            // CPU $8000-$BFFF: First 16 KB of ROM.
            auto offset = addr - 0x8000;
            if (offset < this->cartridge->prgRom.size())
                return &this->cartridge->prgRom[offset];
        } else {
            // CPU $C000-$FFFF: Last 16 KB of ROM (NROM-256) or mirror of $8000-$BFFF (NROM-128).
            auto offset = addr - 0xC000;

            if (offset < this->cartridge->prgRom.size()) {
                auto last16KBStart = (this->cartridge->prgRom.size() - 1) & ~0x3fff;
                auto prgAddr = last16KBStart | (offset & 0x3fff);
                return &this->cartridge->prgRom[prgAddr];
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