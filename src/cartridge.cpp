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

    const Byte *DMAStart(nes::Address addr) const override {
        if (addr < 0x2000)
            return &this->cartridge->chrROM[addr % 0x2000];
        else if (addr < 0x8000)
            return nullptr;
        else if (addr < 0xC000)
            // CPU $8000-$BFFF: 16 KB switchable PRG ROM bank
            return &this->cartridge->prgROM[this->secondBankStart | (addr % 0x4000)];
        else
            // CPU $C000-$FFFF: 16 KB PRG ROM bank, fixed to the last bank
            return &this->cartridge->prgROM[this->firstBankStart | (addr % 0x4000)];
    }

    Byte Read(nes::Address addr) const override {
        if (addr < 0x2000)
            return this->cartridge->chrROM[addr % 0x2000];
        else if (addr < 0x8000)
            return 0;
        else if (addr < 0xC000)
            // CPU $8000-$BFFF: 16 KB switchable PRG ROM bank
            return this->cartridge->prgROM[this->firstBankStart | (addr % 0x4000)];
        else
            // CPU $C000-$FFFF: 16 KB PRG ROM bank, fixed to the last bank
            return this->cartridge->prgROM[this->secondBankStart | (addr % 0x4000)];
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

    const Byte *DMAStart(nes::Address addr) const override {
        if (addr < 0x1000)
            return &this->cartridge->chrROM[this->firstCHROffset | (addr & 0xfff)];
        else if (addr < 0x2000)
            return &this->cartridge->chrROM[this->secondProgOffset | (addr & 0xfff)];
        else if (addr < 0x6000)
            return nullptr;
        else if (addr < 0x8000)
            // CPU $6000-$7FFF: 8 KB PRG RAM bank, (optional)
            if (!this->cartridge->prgRAM.empty())
                return &this->cartridge->prgRAM[addr];
            else
                return nullptr;
        else if (addr < 0xC000)
            // CPU $8000-$BFFF: 16 KB PRG ROM bank, either switchable or fixed to the first bank
            return &this->cartridge->prgROM[this->firstProgOffset | (addr % 0x4000)];
        else
            // CPU $C000-$FFFF: 16 KB PRG ROM bank, either fixed to the last bank or switchable
            return &this->cartridge->prgROM[this->secondProgOffset | (addr % 0x4000)];
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


// https://www.nesdev.org/wiki/MMC3
class MMC3 : public Mapper {

private:
    std::array<Byte, 8> registers   = {0};
    Byte registerSelect             = 0;
    bool enableRAM                  = false;
    bool enableWrites               = false;
    bool enableIRQ                  = false;
    bool invertCHR                  = false;
    bool fixLowPRG                  = false;
    Byte irqCounter                 = 0;
    std::array<size_t, 8> chrBanks  = {0};
    std::array<size_t, 4> prgBanks  = {0};

    static const size_t prgBankSize = 0x2000;
    static const size_t chrBankSize = 0x0400;

    void updateBanks() {
        // alignments in Cartridge already guaranteed

        const size_t totalPRG = this->cartridge->prgROM.size() / prgBankSize;
        const size_t totalCHR = this->cartridge->chrROM.size() / chrBankSize;

        this->prgBanks[0]     = this->registers[6];
        this->prgBanks[1]     = this->registers[7];
        this->prgBanks[2]     = (totalPRG - 2);
        this->prgBanks[3]     = (totalPRG - 1);

        if (this->fixLowPRG)
            std::swap(this->prgBanks[0], this->prgBanks[2]);

        this->chrBanks[0] = this->invertCHR ? this->registers[2] : this->registers[0] & ~0x1;
        this->chrBanks[1] = this->invertCHR ? this->registers[3] : this->registers[0] | 0x1;
        this->chrBanks[2] = this->invertCHR ? this->registers[4] : this->registers[1] & ~0x1;
        this->chrBanks[3] = this->invertCHR ? this->registers[5] : this->registers[1] | 0x1;
        this->chrBanks[4] = this->invertCHR ? this->registers[0] & ~0x1 : this->registers[2];
        this->chrBanks[5] = this->invertCHR ? this->registers[0] | 0x1 : this->registers[3];
        this->chrBanks[6] = this->invertCHR ? this->registers[1] & ~0x1 : this->registers[4];
        this->chrBanks[7] = this->invertCHR ? this->registers[1] | 0x1 : this->registers[5];

        for (auto &bank: this->prgBanks)
            bank *= prgBankSize;

        for (auto &bank: this->chrBanks)
            bank *= chrBankSize;
    }

public:
    MMC3(PCartridge &&c) :
        Mapper(std::move(c)) {
        updateBanks();
    }


    Byte Read(Address addr) const override {
        // https://www.nesdev.org/wiki/MMC3#Banks

        // PPU reads
        if (addr < 0x2000) {
            // PPU $0000-$07FF (or $1000-$17FF): 2 KB switchable CHR bank
            // PPU $0800-$0FFF (or $1800-$1FFF): 2 KB switchable CHR bank
            // PPU $1000-$13FF (or $0000-$03FF): 1 KB switchable CHR bank
            // PPU $1400-$17FF (or $0400-$07FF): 1 KB switchable CHR bank
            // PPU $1800-$1BFF (or $0800-$0BFF): 1 KB switchable CHR bank
            // PPU $1C00-$1FFF (or $0C00-$0FFF): 1 KB switchable CHR bank
            const size_t bank      = addr / chrBankSize;
            const size_t offset    = addr % chrBankSize;
            const size_t chrOffset = this->chrBanks[bank] | offset;

            if (chrOffset < this->cartridge->chrROM.size())
                return this->cartridge->chrROM[chrOffset];
        } else if (addr < 0x6000) {
            // not mapped to PPU or CPU
            return 0;
        } else if (addr < 0x8000) {
            // CPU $6000-$7FFF: 8 KB PRG RAM bank (optional)
            addr %= 0x2000;
            if (addr < this->cartridge->prgRAM.size())
                return this->cartridge->prgRAM[addr];
        } else {
            // CPU $8000-$9FFF (or $C000-$DFFF): 8 KB switchable PRG ROM bank
            // CPU $A000-$BFFF: 8 KB switchable PRG ROM bank
            // CPU $C000-$DFFF (or $8000-$9FFF): 8 KB PRG ROM bank, fixed to the second-last bank
            // CPU $E000-$FFFF: 8 KB PRG ROM bank, fixed to the last bank
            const size_t bank      = (addr & ~0x8000) / prgBankSize;
            const size_t offset    = addr % prgBankSize;
            const size_t prgOffset = this->prgBanks[bank] | offset;

            if (prgOffset < this->cartridge->prgROM.size())
                return this->cartridge->prgROM[prgOffset];
        }

        return 0;
    }

    const Byte *DMAStart(Address addr) const override {
        // PPU reads
        if (addr < 0x2000) {
            // PPU $0000-$07FF (or $1000-$17FF): 2 KB switchable CHR bank
            // PPU $0800-$0FFF (or $1800-$1FFF): 2 KB switchable CHR bank
            // PPU $1000-$13FF (or $0000-$03FF): 1 KB switchable CHR bank
            // PPU $1400-$17FF (or $0400-$07FF): 1 KB switchable CHR bank
            // PPU $1800-$1BFF (or $0800-$0BFF): 1 KB switchable CHR bank
            // PPU $1C00-$1FFF (or $0C00-$0FFF): 1 KB switchable CHR bank
            const size_t bank      = addr / chrBankSize;
            const size_t offset    = addr % chrBankSize;
            const size_t chrOffset = this->chrBanks[bank] | offset;

            if (chrOffset + 256 <= this->cartridge->chrROM.size())
                return &this->cartridge->chrROM[chrOffset];
        } else if (addr < 0x6000) {
            // not mapped to PPU or CPU
            return 0;
        } else if (addr < 0x8000) {
            // CPU $6000-$7FFF: 8 KB PRG RAM bank (optional)
            addr %= 0x2000;
            if (addr + 256 < this->cartridge->prgRAM.size())
                return &this->cartridge->prgRAM[addr];
        } else {
            // CPU $8000-$9FFF (or $C000-$DFFF): 8 KB switchable PRG ROM bank
            // CPU $A000-$BFFF: 8 KB switchable PRG ROM bank
            // CPU $C000-$DFFF (or $8000-$9FFF): 8 KB PRG ROM bank, fixed to the second-last bank
            // CPU $E000-$FFFF: 8 KB PRG ROM bank, fixed to the last bank
            const size_t bank      = (addr & ~0x8000) / prgBankSize;
            const size_t offset    = addr % prgBankSize;
            const size_t prgOffset = this->prgBanks[bank] | offset;

            if (prgOffset + 256 <= this->cartridge->prgROM.size())
                return &this->cartridge->prgROM[prgOffset];
        }

        return nullptr;
    }

    void Write(Address addr, Byte data) override {
        // PPU reads
        if (addr < 0x2000) {
            // PPU $0000-$07FF (or $1000-$17FF): 2 KB switchable CHR bank
            // PPU $0800-$0FFF (or $1800-$1FFF): 2 KB switchable CHR bank
            // PPU $1000-$13FF (or $0000-$03FF): 1 KB switchable CHR bank
            // PPU $1400-$17FF (or $0400-$07FF): 1 KB switchable CHR bank
            // PPU $1800-$1BFF (or $0800-$0BFF): 1 KB switchable CHR bank
            // PPU $1C00-$1FFF (or $0C00-$0FFF): 1 KB switchable CHR bank
            const size_t bank      = addr / chrBankSize;
            const size_t offset    = addr % chrBankSize;
            const size_t chrOffset = this->chrBanks[bank] | offset;

            if (chrOffset < this->cartridge->chrROM.size())
                this->cartridge->chrROM[chrOffset] = data;
        } else if (addr < 0x6000) {
            // probably unmapped?
            return;
        } else if (addr < 0x8000) {
            // CPU $6000-$7FFF: 8 KB PRG RAM bank (optional)
            addr %= 0x2000;
            if (addr < this->cartridge->prgRAM.size())
                this->cartridge->prgRAM[addr] = data;
        } else if (addr < 0xa000 && addr & 1) {
            // Bank data ($8001-$9FFF, odd)
            this->registers[this->registerSelect] = data;

            this->updateBanks();
        } else if (addr < 0xa000 && ~addr & 1) {
            // Bank select ($8000-$9FFE, even)
            this->registerSelect = data & 0x7;
            this->fixLowPRG      = data & 0x40;
            this->invertCHR      = data & 0x80;

            this->updateBanks();
        } else if (addr < 0xc000 && addr & 1) {
            // PRG RAM protect ($A001-$BFFF, odd)
            this->enableRAM    = data & 0x40;
            this->enableWrites = data & 0x80;
        } else if (addr < 0xc000 && ~addr & 1) {
            // Mirroring ($A000-$BFFE, even)
            this->cartridge->mirroringMode =
                    (data & 1) ? Cartridge::MirroringMode::Horizontal : Cartridge::MirroringMode::Vertical;
        } else if (addr < 0xe000 && addr & 1) {
            // IRQ reload ($C001-$DFFF, odd)
            this->irqCounter = 0;
        } else if (addr < 0xe000 && ~addr & 1) {
            // IRQ latch ($C000-$DFFE, even)
        } else if (addr & 1) {
            // IRQ enable ($E001-$FFFF, odd)
            // IRQ disable ($E000-$FFFE, even)
            this->enableIRQ = addr & 1;
        }
    }
};

std::unique_ptr<Mapper> Mapper::Create(MapperType mapperType, PCartridge &&cart) {
    switch (mapperType) {
        case MapperType::INESMapper000:
            return std::make_unique<NROM>(std::move(cart));
            //        case MapperType::INESMapper001:
            //            return std::make_unique<MMC1>(std::move(cart));
        case MapperType::INESMapper002:
            return std::make_unique<UxROM>(std::move(cart));
        case MapperType::INESMapper004:
            return std::make_unique<MMC3>(std::move(cart));
        default:
            break;
    }

    throw std::runtime_error(std::string("Unknown mapper type: ").append(std::to_string(uint8_t(mapperType))));
}

} // namespace nes