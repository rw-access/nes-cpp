#include "cartridge.h"

namespace nes {

Mapper::Mapper(nes::PCartridge &&c) :
    cartridge(std::move(c)) {
}

class UxROM : public Mapper {
private:
    Address firstBankStart  = 0x0000;
    Address secondBankStart = 0x0000;

public:
    UxROM(PCartridge &&c) :
        Mapper(std::move(c)) {
        this->secondBankStart = (this->cartridge->prgROM.size() - 1) & ~0x3fff;
    }

    Byte Read(nes::Address addr) const override {
        if (addr < 0x2000)
            return this->cartridge->chrROM[addr % 0x2000];
        else if (addr < 0x8000)
            return 0;
        else if (addr < 0xC000)
            // CPU $8000-$BFFF: 16 KB switchable PRG ROM bank
            return this->cartridge->prgROM[this->secondBankStart | (addr % 0x4000)];
        else
            // CPU $C000-$FFFF: 16 KB PRG ROM bank, fixed to the last bank
            return this->cartridge->prgROM[this->firstBankStart | (addr % 0x4000)];
    }

    void Write(nes::Address addr, Byte data) override {
        if (addr < 0x2000)
            this->cartridge->chrROM[addr] = data;
        else if (addr < 0x8000)
            // CPU $8000-$BFFF: 16 KB switchable PRG ROM bank
            return;
        else
            this->firstBankStart = (Address(data & 0x0f) << 16) % this->cartridge->prgROM.size();
    }
};


class NROM : public UxROM {
    using UxROM::UxROM;

public:
    void Write(nes::Address addr, nes::Byte data) override {
        if (addr < 0x2000)
            this->cartridge->chrROM[addr % 0x2000] = data;
    }
};


std::unique_ptr<Mapper> Mapper::Create(nes::MapperType mapperType, nes::PCartridge &&cart) {
    switch (mapperType) {
        case MapperType::INESMapper000:
            return std::make_unique<NROM>(std::move(cart));
        case MapperType::INESMapper002:
            return std::make_unique<UxROM>(std::move(cart));
    }

    throw std::runtime_error(std::string("Unknown mapper type: ").append(std::to_string(uint8_t(mapperType))));
}

} // namespace nes