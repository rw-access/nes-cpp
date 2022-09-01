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


// https://www.nesdev.org/wiki/MMC1
class MMC1 : public Mapper {
private:
    using PrgBankMode = enum : uint8_t {
        switch32KBBank          = 0,
        switch32KBBankAlt       = 1,
        fixFirstAndSwitchSecond = 2,
        fixSecondAndSwitchFirst = 3,
    };

    Byte chrBanks[2];
    Byte prgBank            = 0;

    size_t firstProgOffset  = 0x0000;
    size_t secondProgOffset = 0x0000;
    size_t firstCHROffset   = 0x0000;
    size_t secondCHROffset  = 0x0000;

    Byte shiftReg           = 0x20;
    PrgBankMode prgBankMode = switch32KBBank;
    bool chrSplitBanks      = false;

private:
    void updateRegister(Address addr, Byte data) {
        if (addr < 0xA000) {
            // https://www.nesdev.org/wiki/MMC1#Control_(internal,_$8000-$9FFF)
            this->chrSplitBanks = data & 0x10;
            this->prgBankMode   = PrgBankMode((data >> 2) & 3);

            switch (data & 0x3) {
                case 0:
                    this->cartridge->mirroringMode = Cartridge::MirroringMode::SingleScreenLowerBank;
                    break;
                case 1:
                    this->cartridge->mirroringMode = Cartridge::MirroringMode::SingleScreenUpperBank;
                    break;
                case 2:
                    this->cartridge->mirroringMode = Cartridge::MirroringMode::Vertical;
                    break;
                case 3:
                    this->cartridge->mirroringMode = Cartridge::MirroringMode::Horizontal;
                    break;
            }
        } else if (addr < 0xC000) {
            // https://www.nesdev.org/wiki/MMC1#CHR_bank_0_(internal,_$A000-$BFFF)
            this->chrBanks[0] = data;
        } else if (addr < 0xF000) {
            // https://www.nesdev.org/wiki/MMC1#CHR_bank_1_(internal,_$C000-$DFFF)
            this->chrBanks[1] = data;
        } else {
            // https://www.nesdev.org/wiki/MMC1#PRG_bank_(internal,_$E000-$FFFF)
            this->prgBank = data & 0xF;
        }

        // compute the new offsets so read/write ops are faster
        switch (this->prgBankMode) {
            case switch32KBBank:
            case switch32KBBankAlt:
                // switch 32 KB at $8000, ignoring low bit of bank number
                this->firstProgOffset  = size_t(this->prgBank & 0xE) << 15;
                this->secondProgOffset = this->firstProgOffset + 0x4000;
                break;
            case fixFirstAndSwitchSecond:
                // fix first bank at $8000 and switch 16 KB bank at $C000
                this->firstProgOffset  = 0;
                this->secondProgOffset = size_t(this->prgBank & 0xF) << 14;
                break;
            case fixSecondAndSwitchFirst:
                // fix last bank at $C000 and switch 16 KB bank at $8000
                this->firstProgOffset  = size_t(this->prgBank & 0xF) << 14;
                this->secondProgOffset = (this->cartridge->prgROM.size() - 1) & ~(0x3fff);
                break;
        }


        if (this->chrSplitBanks) {
            this->firstCHROffset  = size_t(this->chrBanks[0]) << 12;
            this->secondCHROffset = size_t(this->chrBanks[1]) << 12;
        } else {
            this->firstCHROffset  = size_t(this->chrBanks[0]) << 12;
            this->secondCHROffset = this->firstCHROffset + 0x1000;
        }
    }

public:
    MMC1(PCartridge &&c) :
        Mapper(std::move(c)) {
        this->firstProgOffset  = 0;
        this->secondProgOffset = (this->cartridge->prgROM.size() - 1) & ~0x3fff;
    }

    Byte Read(nes::Address addr) const override {
        if (addr < 0x1000)
            return this->cartridge->chrROM[this->firstCHROffset | (addr & 0xfff)];
        else if (addr < 0x2000)
            return this->cartridge->chrROM[this->secondProgOffset | (addr & 0xfff)];
        else if (addr < 0x6000)
            return 0;
        else if (addr < 0x8000)
            // CPU $6000-$7FFF: 8 KB PRG RAM bank, (optional)
            if (!this->cartridge->prgRAM.empty())
                return this->cartridge->prgRAM[addr];
            else
                return 0;
        else if (addr < 0xC000)
            // CPU $8000-$BFFF: 16 KB PRG ROM bank, either switchable or fixed to the first bank
            return this->cartridge->prgROM[this->firstProgOffset | (addr % 0x4000)];
        else
            // CPU $C000-$FFFF: 16 KB PRG ROM bank, either fixed to the last bank or switchable
            return this->cartridge->prgROM[this->secondProgOffset | (addr % 0x4000)];
    }

    void Write(nes::Address addr, Byte data) override {
        if (addr < 0x1000)
            this->cartridge->chrROM[this->firstCHROffset | (addr & 0xfff)] = data;
        else if (addr < 0x2000)
            this->cartridge->chrROM[this->secondCHROffset | (addr & 0xfff)] = data;
        else if (addr < 0x6000)
            return;
        else if (addr < 0x8000) {
            // CPU $6000-$7FFF: 8 KB PRG RAM bank, (optional)
            if (!this->cartridge->prgRAM.empty())
                this->cartridge->prgRAM[addr] = data;
        } else {
            if (data & 0x80) {
                //  7  bit  0
                // ---- ----
                // Rxxx xxxD
                // |       |
                // |       +- Data bit to be shifted into shift register, LSB first
                // +--------- 1: Reset shift register and write Control with (Control OR $0C),
                //               locking PRG ROM at $C000-$FFFF to the last bank.

                this->shiftReg = 0x20; // after five shifts will be in bit 1
            } else {
                // bits are shifted to the *right* because the LSB is added first
                this->shiftReg = (this->shiftReg >> 1) | ((data & 0x1) << 5);

                if (this->shiftReg & 0x1) {
                    this->updateRegister(addr, this->shiftReg >> 1);

                    // reset it back to the initial state, waiting for 5 shifts
                    this->shiftReg = 0x02;
                }
            }
        }
    }
};


std::unique_ptr<Mapper> Mapper::Create(nes::MapperType mapperType, nes::PCartridge &&cart) {
    switch (mapperType) {
        case MapperType::INESMapper000:
            return std::make_unique<NROM>(std::move(cart));
        case MapperType::INESMapper001:
            return std::make_unique<MMC1>(std::move(cart));
        case MapperType::INESMapper002:
            return std::make_unique<UxROM>(std::move(cart));
    }

    throw std::runtime_error(std::string("Unknown mapper type: ").append(std::to_string(uint8_t(mapperType))));
}

} // namespace nes