#include "cpu.h"
#include "apu.h"
#include "opcodes.def"
#include "ppu.h"
#include <iostream>
#include <string_view>


namespace nes {

static bool DBG_PRINT = false;

// clang-format off
static const DecodedInstruction decodeTable[256]{
        {Opcode::BRK, AddressingMode::Implied, 7, false},
        {Opcode::ORA, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::SLO, AddressingMode::IndexedIndirect, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPage, 3, false},
        {Opcode::ORA, AddressingMode::ZeroPage, 3, false},
        {Opcode::ASL, AddressingMode::ZeroPage, 5, false},
        {Opcode::SLO, AddressingMode::ZeroPage, 5, false},
        {Opcode::PHP, AddressingMode::Implied, 3, false},
        {Opcode::ORA, AddressingMode::Immediate, 2, false},
        {Opcode::ASL, AddressingMode::Accumulator, 2, false},
        {Opcode::ANC, AddressingMode::Immediate, 2, false},
        {Opcode::NOP, AddressingMode::Absolute, 4, false},
        {Opcode::ORA, AddressingMode::Absolute, 4, false},
        {Opcode::ASL, AddressingMode::Absolute, 6, false},
        {Opcode::SLO, AddressingMode::Absolute, 6, false},
        {Opcode::BPL, AddressingMode::Relative, 2, true},
        {Opcode::ORA, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::SLO, AddressingMode::IndirectIndexed, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::ORA, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::ASL, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::SLO, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::CLC, AddressingMode::Implied, 2, false},
        {Opcode::ORA, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::NOP, AddressingMode::Implied, 2, false},
        {Opcode::SLO, AddressingMode::AbsoluteIndexedY, 7, false},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::ORA, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::ASL, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::SLO, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::JSR, AddressingMode::Absolute, 6, false},
        {Opcode::AND, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::RLA, AddressingMode::IndexedIndirect, 8, false},
        {Opcode::BIT, AddressingMode::ZeroPage, 3, false},
        {Opcode::AND, AddressingMode::ZeroPage, 3, false},
        {Opcode::ROL, AddressingMode::ZeroPage, 5, false},
        {Opcode::RLA, AddressingMode::ZeroPage, 5, false},
        {Opcode::PLP, AddressingMode::Implied, 4, false},
        {Opcode::AND, AddressingMode::Immediate, 2, false},
        {Opcode::ROL, AddressingMode::Accumulator, 2, false},
        {Opcode::ANC, AddressingMode::Immediate, 2, false},
        {Opcode::BIT, AddressingMode::Absolute, 4, false},
        {Opcode::AND, AddressingMode::Absolute, 4, false},
        {Opcode::ROL, AddressingMode::Absolute, 6, false},
        {Opcode::RLA, AddressingMode::Absolute, 6, false},
        {Opcode::BMI, AddressingMode::Relative, 2, true},
        {Opcode::AND, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::RLA, AddressingMode::IndirectIndexed, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::AND, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::ROL, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::RLA, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::SEC, AddressingMode::Implied, 2, false},
        {Opcode::AND, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::NOP, AddressingMode::Implied, 2, false},
        {Opcode::RLA, AddressingMode::AbsoluteIndexedY, 7, false},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::AND, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::ROL, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::RLA, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::RTI, AddressingMode::Implied, 6, false},
        {Opcode::EOR, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::SRE, AddressingMode::IndexedIndirect, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPage, 3, false},
        {Opcode::EOR, AddressingMode::ZeroPage, 3, false},
        {Opcode::LSR, AddressingMode::ZeroPage, 5, false},
        {Opcode::SRE, AddressingMode::ZeroPage, 5, false},
        {Opcode::PHA, AddressingMode::Implied, 3, false},
        {Opcode::EOR, AddressingMode::Immediate, 2, false},
        {Opcode::LSR, AddressingMode::Accumulator, 2, false},
        {Opcode::ALR, AddressingMode::Immediate, 2, false},
        {Opcode::JMP, AddressingMode::Absolute, 3, false},
        {Opcode::EOR, AddressingMode::Absolute, 4, false},
        {Opcode::LSR, AddressingMode::Absolute, 6, false},
        {Opcode::SRE, AddressingMode::Absolute, 6, false},
        {Opcode::BVC, AddressingMode::Relative, 2, true},
        {Opcode::EOR, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::SRE, AddressingMode::IndirectIndexed, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::EOR, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::LSR, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::SRE, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::CLI, AddressingMode::Implied, 2, false},
        {Opcode::EOR, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::NOP, AddressingMode::Implied, 2, false},
        {Opcode::SRE, AddressingMode::AbsoluteIndexedY, 7, false},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::EOR, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::LSR, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::SRE, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::RTS, AddressingMode::Implied, 6, false},
        {Opcode::ADC, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::RRA, AddressingMode::IndexedIndirect, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPage, 3, false},
        {Opcode::ADC, AddressingMode::ZeroPage, 3, false},
        {Opcode::ROR, AddressingMode::ZeroPage, 5, false},
        {Opcode::RRA, AddressingMode::ZeroPage, 5, false},
        {Opcode::PLA, AddressingMode::Implied, 4, false},
        {Opcode::ADC, AddressingMode::Immediate, 2, false},
        {Opcode::ROR, AddressingMode::Accumulator, 2, false},
        {Opcode::ARR, AddressingMode::Immediate, 2, false},
        {Opcode::JMP, AddressingMode::Indirect, 5, false},
        {Opcode::ADC, AddressingMode::Absolute, 4, false},
        {Opcode::ROR, AddressingMode::Absolute, 6, false},
        {Opcode::RRA, AddressingMode::Absolute, 6, false},
        {Opcode::BVS, AddressingMode::Relative, 2, true},
        {Opcode::ADC, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::RRA, AddressingMode::IndirectIndexed, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::ADC, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::ROR, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::RRA, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::SEI, AddressingMode::Implied, 2, false},
        {Opcode::ADC, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::NOP, AddressingMode::Implied, 2, false},
        {Opcode::RRA, AddressingMode::AbsoluteIndexedY, 7, false},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::ADC, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::ROR, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::RRA, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::NOP, AddressingMode::Immediate, 2, false},
        {Opcode::STA, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::NOP, AddressingMode::Immediate, 2, false},
        {Opcode::SAX, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::STY, AddressingMode::ZeroPage, 3, false},
        {Opcode::STA, AddressingMode::ZeroPage, 3, false},
        {Opcode::STX, AddressingMode::ZeroPage, 3, false},
        {Opcode::SAX, AddressingMode::ZeroPage, 3, false},
        {Opcode::DEY, AddressingMode::Implied, 2, false},
        {Opcode::NOP, AddressingMode::Immediate, 2, false},
        {Opcode::TXA, AddressingMode::Implied, 2, false},
        {Opcode::XAA, AddressingMode::Immediate, 2, false},
        {Opcode::STY, AddressingMode::Absolute, 4, false},
        {Opcode::STA, AddressingMode::Absolute, 4, false},
        {Opcode::STX, AddressingMode::Absolute, 4, false},
        {Opcode::SAX, AddressingMode::Absolute, 4, false},
        {Opcode::BCC, AddressingMode::Relative, 2, true},
        {Opcode::STA, AddressingMode::IndirectIndexed, 6, false},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::AHX, AddressingMode::IndirectIndexed, 6, false},
        {Opcode::STY, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::STA, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::STX, AddressingMode::ZeroPageIndexedY, 4, false},
        {Opcode::SAX, AddressingMode::ZeroPageIndexedY, 4, false},
        {Opcode::TYA, AddressingMode::Implied, 2, false},
        {Opcode::STA, AddressingMode::AbsoluteIndexedY, 5, false},
        {Opcode::TXS, AddressingMode::Implied, 2, false},
        {Opcode::TAS, AddressingMode::AbsoluteIndexedY, 5, false},
        {Opcode::SHY, AddressingMode::AbsoluteIndexedX, 5, false},
        {Opcode::STA, AddressingMode::AbsoluteIndexedX, 5, false},
        {Opcode::SHX, AddressingMode::AbsoluteIndexedY, 5, false},
        {Opcode::AHX, AddressingMode::AbsoluteIndexedY, 5, false},
        {Opcode::LDY, AddressingMode::Immediate, 2, false},
        {Opcode::LDA, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::LDX, AddressingMode::Immediate, 2, false},
        {Opcode::LAX, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::LDY, AddressingMode::ZeroPage, 3, false},
        {Opcode::LDA, AddressingMode::ZeroPage, 3, false},
        {Opcode::LDX, AddressingMode::ZeroPage, 3, false},
        {Opcode::LAX, AddressingMode::ZeroPage, 3, false},
        {Opcode::TAY, AddressingMode::Implied, 2, false},
        {Opcode::LDA, AddressingMode::Immediate, 2, false},
        {Opcode::TAX, AddressingMode::Implied, 2, false},
        {Opcode::LAX, AddressingMode::Immediate, 2, false},
        {Opcode::LDY, AddressingMode::Absolute, 4, false},
        {Opcode::LDA, AddressingMode::Absolute, 4, false},
        {Opcode::LDX, AddressingMode::Absolute, 4, false},
        {Opcode::LAX, AddressingMode::Absolute, 4, false},
        {Opcode::BCS, AddressingMode::Relative, 2, true},
        {Opcode::LDA, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::LAX, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::LDY, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::LDA, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::LDX, AddressingMode::ZeroPageIndexedY, 4, false},
        {Opcode::LAX, AddressingMode::ZeroPageIndexedY, 4, false},
        {Opcode::CLV, AddressingMode::Implied, 2, false},
        {Opcode::LDA, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::TSX, AddressingMode::Implied, 2, false},
        {Opcode::LAS, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::LDY, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::LDA, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::LDX, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::LAX, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::CPY, AddressingMode::Immediate, 2, false},
        {Opcode::CMP, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::NOP, AddressingMode::Immediate, 2, false},
        {Opcode::DCP, AddressingMode::IndexedIndirect, 8, false},
        {Opcode::CPY, AddressingMode::ZeroPage, 3, false},
        {Opcode::CMP, AddressingMode::ZeroPage, 3, false},
        {Opcode::DEC, AddressingMode::ZeroPage, 5, false},
        {Opcode::DCP, AddressingMode::ZeroPage, 5, false},
        {Opcode::INY, AddressingMode::Implied, 2, false},
        {Opcode::CMP, AddressingMode::Immediate, 2, false},
        {Opcode::DEX, AddressingMode::Implied, 2, false},
        {Opcode::AXS, AddressingMode::Immediate, 2, false},
        {Opcode::CPY, AddressingMode::Absolute, 4, false},
        {Opcode::CMP, AddressingMode::Absolute, 4, false},
        {Opcode::DEC, AddressingMode::Absolute, 6, false},
        {Opcode::DCP, AddressingMode::Absolute, 6, false},
        {Opcode::BNE, AddressingMode::Relative, 2, true},
        {Opcode::CMP, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::DCP, AddressingMode::IndirectIndexed, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::CMP, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::DEC, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::DCP, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::CLD, AddressingMode::Implied, 2, false},
        {Opcode::CMP, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::NOP, AddressingMode::Implied, 2, false},
        {Opcode::DCP, AddressingMode::AbsoluteIndexedY, 7, false},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::CMP, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::DEC, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::DCP, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::CPX, AddressingMode::Immediate, 2, false},
        {Opcode::SBC, AddressingMode::IndexedIndirect, 6, false},
        {Opcode::NOP, AddressingMode::Immediate, 2, false},
        {Opcode::ISB, AddressingMode::IndexedIndirect, 8, false},
        {Opcode::CPX, AddressingMode::ZeroPage, 3, false},
        {Opcode::SBC, AddressingMode::ZeroPage, 3, false},
        {Opcode::INC, AddressingMode::ZeroPage, 5, false},
        {Opcode::ISB, AddressingMode::ZeroPage, 5, false},
        {Opcode::INX, AddressingMode::Implied, 2, false},
        {Opcode::SBC, AddressingMode::Immediate, 2, false},
        {Opcode::NOP, AddressingMode::Implied, 2, false},
        {Opcode::SBC, AddressingMode::Immediate, 2, false},
        {Opcode::CPX, AddressingMode::Absolute, 4, false},
        {Opcode::SBC, AddressingMode::Absolute, 4, false},
        {Opcode::INC, AddressingMode::Absolute, 6, false},
        {Opcode::ISB, AddressingMode::Absolute, 6, false},
        {Opcode::BEQ, AddressingMode::Relative, 2, true},
        {Opcode::SBC, AddressingMode::IndirectIndexed, 5, true},
        {Opcode::STP, AddressingMode::Implied, 2, false},
        {Opcode::ISB, AddressingMode::IndirectIndexed, 8, false},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::SBC, AddressingMode::ZeroPageIndexedX, 4, false},
        {Opcode::INC, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::ISB, AddressingMode::ZeroPageIndexedX, 6, false},
        {Opcode::SED, AddressingMode::Implied, 2, false},
        {Opcode::SBC, AddressingMode::AbsoluteIndexedY, 4, true},
        {Opcode::NOP, AddressingMode::Implied, 2, false},
        {Opcode::ISB, AddressingMode::AbsoluteIndexedY, 7, false},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::SBC, AddressingMode::AbsoluteIndexedX, 4, true},
        {Opcode::INC, AddressingMode::AbsoluteIndexedX, 7, false},
        {Opcode::ISB, AddressingMode::AbsoluteIndexedX, 7, false},
};
// clang-format on

static bool crossesPageBoundary(Address a, Address b) {
    return (a & 0xff00) != (b & 0xff00);
}

uint16_t CPU::step() {
    auto prevCycle = this->cycle;

    if (this->handleInterrupt())
        return this->cycle - prevCycle;

    auto prePC         = this->pc;
    auto instFirstByte = this->read(this->pc);
    auto decoded       = decodeTable[instFirstByte];

    Address address    = 0;
    Address indirect   = 0;
    Byte offset        = 0;

    this->cycle += decoded.MinCycles;
    this->pc++;

    switch (decoded.addressingMode) {
        case AddressingMode::Implied:
        case AddressingMode::Accumulator:
            // read + alu cycles tracked in instFirstByte
            break;
        case AddressingMode::Absolute:
            address = this->readAddress(this->pc);
            this->pc += 2;
            break;
        case AddressingMode::Immediate:
            address = this->pc;
            this->pc++;
            break;
        case AddressingMode::AbsoluteIndexedX:
            indirect = this->readAddress(this->pc);
            address  = indirect + this->regX;
            this->pc += 2;
            this->cycle += decoded.PageBoundaryHit && crossesPageBoundary(indirect, address);
            break;
        case AddressingMode::AbsoluteIndexedY:
            indirect = this->readAddress(this->pc);
            address  = indirect + this->regY;
            this->pc += 2;
            this->cycle += decoded.PageBoundaryHit && crossesPageBoundary(indirect, address);
            break;
        case AddressingMode::IndexedIndirect:
            // val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256)
            // val = PEEK(
            // zero page wrap around
            offset   = this->read(this->pc);
            indirect = Byte(offset + regX);
            address  = this->readAddressIndirectWraparound(indirect);
            this->pc += 1;
            break;
        case AddressingMode::Indirect:
            indirect = this->readAddress(this->pc);
            address  = this->readAddressIndirectWraparound(indirect);
            this->pc += 2;
            break;
        case AddressingMode::IndirectIndexed:
            // val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)
            offset   = this->read(this->pc);
            indirect = this->readAddressIndirectWraparound(offset);
            address  = indirect + this->regY;
            this->pc += 1;
            this->cycle += decoded.PageBoundaryHit && crossesPageBoundary(indirect, address);
            break;
        case AddressingMode::Relative:
            // TODO: check that this handles negative offsets correctly
            offset = this->read(this->pc);
            this->pc++;

            if (offset & 0x80)
                address = this->pc - Address(0x100 - offset);
            else
                address = this->pc + Address(offset);
            break;
        case AddressingMode::ZeroPage:
            offset  = this->read(this->pc);
            address = offset;
            this->pc++;
            break;
        case AddressingMode::ZeroPageIndexedX:
            offset  = this->read(this->pc);
            address = Byte(offset + this->regX);
            this->pc++;
            break;
        case AddressingMode::ZeroPageIndexedY:
            offset  = this->read(this->pc);
            address = Byte(offset + this->regY);
            this->pc++;
            break;
    }


    if (DBG_PRINT) {
        auto nextPC = this->pc;

        // debug buffer, more than enough to include everything
        // C000  4C F5 C5  JMP $C5F5                       A:00 X:00 Y:00 P:24 SP:FD CYC:7
        char buf[120]   = "";
        char *remaining = buf;
        remaining += sprintf(remaining, "%04X  ", prePC);

        // print each byte in the instFirstByte
        for (auto addr = prePC; addr < prePC + 3; addr++) {
            if (addr < nextPC) {
                auto instrByte = this->read(addr);
                remaining += sprintf(remaining, "%02X ", instrByte);
            } else {
                remaining += sprintf(remaining, "   ");
            }
        }

        // formatted opcode
        char charPrefix = ' ';
        if (decoded.opcode == Opcode::NOP && instFirstByte != 0xea)
            charPrefix = ' '; // '*'

        remaining += sprintf(remaining, "%c%s ", charPrefix, OpcodeStrings[uint8_t(decoded.opcode)]);

        switch (decoded.addressingMode) {
            case AddressingMode::Implied:
                break;
            case AddressingMode::Accumulator:
                remaining += sprintf(remaining, "A");
                break;
            case AddressingMode::Absolute:
                remaining += sprintf(remaining, "$%04X", address);
                break;
            case AddressingMode::AbsoluteIndexedX:
                remaining += sprintf(remaining, "$%04X,X @ %04X", indirect, address);
                break;
            case AddressingMode::AbsoluteIndexedY:
                remaining += sprintf(remaining, "$%04X,Y @ %04X", indirect, address);
                break;
            case AddressingMode::Immediate:
                remaining += sprintf(remaining, "#$%02X", this->read(address));
                break;
            case AddressingMode::IndexedIndirect:
                //                address = Address(this->read(this->pc)) + Address(this->regX);
                remaining += sprintf(remaining, "($%02X,X) @ %02X = %04X", offset, indirect, address);
                break;
            case AddressingMode::Indirect:
                remaining += sprintf(remaining, "($%04X)", indirect);
                break;
            case AddressingMode::IndirectIndexed:
                remaining += sprintf(remaining, "($%02X),Y = %04X @ %04X", offset, indirect, address);
                break;
            case AddressingMode::Relative:
                remaining += sprintf(remaining, "$%04X", address);
                break;
            case AddressingMode::ZeroPage:
                remaining += sprintf(remaining, "$%02X", address);
                break;
            case AddressingMode::ZeroPageIndexedX:
                remaining += sprintf(remaining, "$%02X,X @ %02X", offset, address);
                break;
            case AddressingMode::ZeroPageIndexedY:
                remaining += sprintf(remaining, "$%02X,Y @ %02X", offset, address);
                break;
        }

        switch (decoded.addressingMode) {
            case AddressingMode::Absolute:
                if (decoded.opcode == Opcode::JSR || decoded.opcode == Opcode::JMP)
                    break;
            case AddressingMode::AbsoluteIndexedX:
            case AddressingMode::AbsoluteIndexedY:
            case AddressingMode::IndirectIndexed:
            case AddressingMode::IndexedIndirect:
            case AddressingMode::ZeroPage:
            case AddressingMode::ZeroPageIndexedX:
            case AddressingMode::ZeroPageIndexedY:
                remaining += sprintf(remaining, " = %02X", this->read(address));
                break;
            case AddressingMode::Indirect:
                remaining += sprintf(remaining, " = %04X", address);
                break;
            default:
                break;
        }

        // fill in whitespace to get to the registers
        for (; remaining < buf + 48; remaining++) {
            *remaining = ' ';
        }

        // registers
        remaining += sprintf(remaining, "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d", this->regA, this->regX,
                             this->regY, uint8_t(this->status.to_ulong()), this->regSP, uint16_t(prevCycle));

        (void) remaining;
        std::cout << std::endl << buf;
    }

    this->dispatch(decoded, address);
    return this->cycle - prevCycle;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ADC
template<>
void CPU::op<Opcode::ADC>(AddressingMode, Address addr) {
    WordWithCarry a = this->regA;
    WordWithCarry b = this->read(addr);
    WordWithCarry c = this->status[Flag::C];
    auto sum        = a + b + c;
    this->regA      = Byte(sum);

    //  A  +   B  + C  =>  A   C Z V N
    // 127    127   1     -1   0 0 1 1
    // 127    127   1     -1   1 0 1 1
    // 127   -128   0     -1   0 0 0 1
    // 127   -128   1      0   1 1 0 0

    // 0x7f A + 0x80 B + 0x01 C
    // 127   -128 + 1
    // A = 0, C = 0
    this->status[Flag::C] = sum & 0x100;

    // detect overflow by checking the resulting sign to be different
    // from both operands
    // the resulting sign has to be different from both the operands
    this->status[Flag::V] = (a ^ sum) & (b ^ sum) & 0x80;
    this->setCNZ(sum);
}

// http://www.oxyron.de/html/opcodes02.html
template<>
void CPU::op<Opcode::AHX>(AddressingMode mode, Address addr) {
    // AHX {adr} = stores A&X&H into {adr}
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ALR
template<>
void CPU::op<Opcode::ALR>(AddressingMode mode, Address addr) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ANC
template<>
void CPU::op<Opcode::ANC>(AddressingMode, Address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AND
template<>
void CPU::op<Opcode::AND>(AddressingMode, Address addr) {
    this->regA &= this->read(addr);
    this->setNZ(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ARR
template<>
void CPU::op<Opcode::ARR>(AddressingMode mode, Address addr) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ASL
template<>
void CPU::op<Opcode::ASL>(AddressingMode mode, Address addr) {
    WordWithCarry resultWide = 0;

    if (mode == AddressingMode::Accumulator) {
        resultWide = WordWithCarry(this->regA) << 1;
        this->regA = Byte(resultWide);
    } else {
        resultWide = WordWithCarry(this->read(addr)) << 1;
        this->write(addr, Byte(resultWide));
    }

    this->setCNZ(resultWide);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AXS
template<>
void CPU::op<Opcode::AXS>(AddressingMode mode, Address addr) {
}


void CPU::BXX(Flag flag, bool isSet, Address addr) {
    if (this->status[flag] == isSet) {
        this->cycle++;
        this->cycle += crossesPageBoundary(this->pc, addr);
        this->pc = addr;
    }
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCC
template<>
void CPU::op<Opcode::BCC>(AddressingMode, Address addr) {
    return this->BXX(Flag::C, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCS
template<>
void CPU::op<Opcode::BCS>(AddressingMode, Address addr) {
    return this->BXX(Flag::C, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BEQ
template<>
void CPU::op<Opcode::BEQ>(AddressingMode, Address addr) {
    return this->BXX(Flag::Z, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BIT
template<>
void CPU::op<Opcode::BIT>(AddressingMode, Address addr) {
    auto m                = this->read(addr);
    auto result           = this->regA & m;
    this->status[Flag::Z] = (result == 0);
    this->status[Flag::V] = (m & 0x40) != 0;
    this->status[Flag::N] = (m & 0x80) != 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BMI
template<>
void CPU::op<Opcode::BMI>(AddressingMode, Address addr) {
    return this->BXX(Flag::N, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BNE
template<>
void CPU::op<Opcode::BNE>(AddressingMode, Address addr) {
    return this->BXX(Flag::Z, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BPL
template<>
void CPU::op<Opcode::BPL>(AddressingMode, Address addr) {
    return this->BXX(Flag::N, false, addr);
}

template<>
void CPU::op<Opcode::BRK>(AddressingMode, Address) {
    // https://www.nesdev.org/obelisk-6502-guide/reference.html#BRK
    // https://www.nesdev.org/6502_cpu.txt
    //
    //  #  address R/W description
    // -- ------- --- -----------------------------------------------
    // 3  $0100,S  W  push PCH on stack (with B flag set), decrement S
    // 4  $0100,S  W  push PCL on stack, decrement S
    // 5  $0100,S  W  push P on stack, decrement S
    // 6   $FFFE   R  fetch PCL
    // 7   $FFFF   R  fetch PCH
    this->pushAddress(this->pc);
    this->op<Opcode::PHP>(AddressingMode::Implied, 0);
    this->status[uint8_t(Flag::I)] = true;
    this->pc                       = this->readAddress(0xfffe);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVC
template<>
void CPU::op<Opcode::BVC>(AddressingMode mode, Address addr) {
    return this->BXX(Flag::V, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVS
template<>
void CPU::op<Opcode::BVS>(AddressingMode mode, Address addr) {
    return this->BXX(Flag::V, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLC
template<>
void CPU::op<Opcode::CLC>(AddressingMode mode, Address addr) {
    this->status[Flag::C] = false;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLD
template<>
void CPU::op<Opcode::CLD>(AddressingMode mode, Address addr) {
    this->status[Flag::D] = false;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLI
template<>
void CPU::op<Opcode::CLI>(AddressingMode mode, Address addr) {
    this->status[Flag::I] = false;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLV
template<>
void CPU::op<Opcode::CLV>(AddressingMode mode, Address addr) {
    this->status[Flag::V] = false;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CMP
template<>
void CPU::op<Opcode::CMP>(AddressingMode mode, Address addr) {
    auto a    = this->regA;
    auto m    = this->read(addr);
    auto data = a - m;
    this->setNZ(data);
    this->status[Flag::C] = a >= m;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPX
template<>
void CPU::op<Opcode::CPX>(AddressingMode mode, Address addr) {
    auto x = this->regX;
    auto m = this->read(addr);
    this->setNZ(x - m);
    this->status[Flag::C] = x >= m;
    // read
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPY
template<>
void CPU::op<Opcode::CPY>(AddressingMode mode, Address addr) {
    auto y    = this->regY;
    auto m    = this->read(addr);
    auto data = y - m;
    this->setNZ(data);
    this->status[Flag::C] = y >= m;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEC
template<>
void CPU::op<Opcode::DEC>(AddressingMode, Address addr) {
    auto m = this->read(addr) - 1;
    this->write(addr, m);
    this->setNZ(m);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DCP
template<>
void CPU::op<Opcode::DCP>(AddressingMode mode, Address addr) {
    this->op<Opcode::DEC>(mode, addr);
    this->op<Opcode::CMP>(mode, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEX
template<>
void CPU::op<Opcode::DEX>(AddressingMode, Address) {
    this->regX--;
    this->setNZ(this->regX);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEY
template<>
void CPU::op<Opcode::DEY>(AddressingMode, Address) {
    this->regY--;
    this->setNZ(this->regY);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#EOR
template<>
void CPU::op<Opcode::EOR>(AddressingMode, Address addr) {
    this->regA ^= this->read(addr);
    this->setNZ(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INC
template<>
void CPU::op<Opcode::INC>(AddressingMode, Address addr) {
    auto data = this->read(addr) + 1;
    this->write(addr, data);
    this->setNZ(data);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INX
template<>
void CPU::op<Opcode::INX>(AddressingMode, Address) {
    this->regX++;
    this->setNZ(this->regX);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INY
template<>
void CPU::op<Opcode::INY>(AddressingMode, Address) {
    this->regY++;
    this->setNZ(this->regY);
}


// https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
template<>
void CPU::op<Opcode::JMP>(AddressingMode, Address addr) {
    this->pc = addr;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JSR
template<>
void CPU::op<Opcode::JSR>(AddressingMode, Address addr) {
    this->pushAddress(this->pc - 1);
    this->pc = addr;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LAS
template<>
void CPU::op<Opcode::LAS>(AddressingMode mode, Address addr) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LAX
template<>
void CPU::op<Opcode::LAX>(AddressingMode mode, Address addr) {
    this->regA = this->regX = this->read(addr);
    this->setNZ(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDA
template<>
void CPU::op<Opcode::LDA>(AddressingMode mode, Address addr) {
    this->regA = this->read(addr);
    this->setNZ(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDX
template<>
void CPU::op<Opcode::LDX>(AddressingMode mode, Address addr) {
    this->regX = this->read(addr);
    this->setNZ(this->regX);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDY
template<>
void CPU::op<Opcode::LDY>(AddressingMode mode, Address addr) {
    this->regY = this->read(addr);
    this->setNZ(this->regY);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LSR
template<>
void CPU::op<Opcode::LSR>(AddressingMode mode, Address addr) {
    WordWithCarry wideData;

    if (mode == AddressingMode::Accumulator) {
        wideData   = WordWithCarry(this->regA);
        wideData   = wideData >> 1 | (wideData & 0x01) << 8;
        this->regA = Byte(wideData);
    } else {
        wideData = WordWithCarry(this->read(addr));
        wideData = wideData >> 1 | (wideData & 0x01) << 8;
        this->write(addr, Byte(wideData));
    }

    this->setCNZ(wideData);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#NOP
template<>
void CPU::op<Opcode::NOP>(AddressingMode, Address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ORA
template<>
void CPU::op<Opcode::ORA>(AddressingMode, Address addr) {
    this->regA |= this->read(addr);
    this->setNZ(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHA
template<>
void CPU::op<Opcode::PHA>(AddressingMode, Address) {
    this->push(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHP
template<>
void CPU::op<Opcode::PHP>(AddressingMode, Address) {
    auto statusCopy = this->status;
    // B flag is always set when pushed to the stack
    // https://www.nesdev.org/wiki/Status_flags#The_B_flag
    statusCopy[Flag::B] = true;

    this->push(statusCopy.to_ulong());
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLA
template<>
void CPU::op<Opcode::PLA>(AddressingMode, Address) {
    this->regA = this->pop();
    this->setNZ(regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLP
template<>
void CPU::op<Opcode::PLP>(AddressingMode, Address) {
    // Two instructions (PLP and RTI) pull a byte from the stack and set all the flags.
    // They ignore bits 5 (U) and 4 (B).
    // https://www.nesdev.org/wiki/Status_flags
    this->status          = this->pop();
    this->status[Flag::U] = true;
    this->status[Flag::B] = false;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROL
template<>
void CPU::op<Opcode::ROL>(AddressingMode mode, Address addr) {
    WordWithCarry wideData;

    if (mode == AddressingMode::Accumulator) {
        wideData   = WordWithCarry(this->regA);
        wideData   = wideData << 1 | this->status[Flag::C];
        this->regA = wideData;
    } else {
        wideData = WordWithCarry(this->read(addr));
        wideData = wideData << 1 | this->status[Flag::C];
        this->write(addr, Byte(wideData));
    }

    this->setCNZ(wideData);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROR
template<>
void CPU::op<Opcode::ROR>(AddressingMode mode, Address addr) {
    WordWithCarry wideData;

    // Bit 7 is filled with the current value of the carry flag whilst the
    // old bit 0 becomes the new carry flag value.

    if (mode == AddressingMode::Accumulator) {
        wideData = WordWithCarry(this->regA);
        wideData |= this->status[Flag::C] << 8;
        wideData |= (wideData & 0x1) << 9;
        wideData >>= 1;
        this->regA = wideData;
    } else {
        wideData = WordWithCarry(this->read(addr));
        wideData |= this->status[Flag::C] << 8;
        wideData |= (wideData & 0x1) << 9;
        wideData >>= 1;
        this->write(addr, Byte(wideData));
    }

    this->setCNZ(wideData);
}

// http://www.oxyron.de/html/opcodes02.html
template<>
void CPU::op<Opcode::RRA>(AddressingMode mode, Address addr) {
    // RRA {adr} = ROR {adr} + ADC {adr}
    this->op<Opcode::ROR>(mode, addr);
    this->op<Opcode::ADC>(mode, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTI
template<>
void CPU::op<Opcode::RTI>(AddressingMode, Address) {
    // Two instructions (PLP and RTI) pull a byte from the stack and set all the flags.
    // They ignore bits 5 (U) and 4 (B).
    // https://www.nesdev.org/wiki/Status_flags
    this->op<Opcode::PLP>(AddressingMode::Implied, 0);
    this->pc = this->popAddress();
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTS
template<>
void CPU::op<Opcode::RTS>(AddressingMode, Address) {
    this->pc = this->popAddress() + 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SAX
template<>
void CPU::op<Opcode::SAX>(AddressingMode mode, Address addr) {
    this->write(addr, this->regA & this->regX);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SBC
template<>
void CPU::op<Opcode::SBC>(AddressingMode mode, Address addr) {
    auto a      = WordWithCarry(this->regA);
    auto m      = WordWithCarry(this->read(addr));

    auto result = a - m - (1 - this->status[Flag::C]);
    this->regA  = Byte(result);
    this->setNZ(result);

    // signs are the same before, but the sign changed after
    this->status[Flag::V] = (a ^ result) & (~m ^ result) & 0x80;
    this->status[Flag::C] = !(result & 0x100);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEC
template<>
void CPU::op<Opcode::SEC>(AddressingMode mode, Address addr) {
    this->status[Flag::C] = 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SED
template<>
void CPU::op<Opcode::SED>(AddressingMode, Address) {
    this->status[Flag::D] = 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEI
template<>
void CPU::op<Opcode::SEI>(AddressingMode, Address) {
    this->status[Flag::I] = 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SHX
template<>
void CPU::op<Opcode::SHX>(AddressingMode, Address) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SHY
template<>
void CPU::op<Opcode::SHY>(AddressingMode mode, Address addr) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SLO
template<>
void CPU::op<Opcode::SLO>(AddressingMode mode, Address addr) {
    this->op<Opcode::ASL>(mode, addr);
    this->op<Opcode::ORA>(mode, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SRE
template<>
void CPU::op<Opcode::SRE>(AddressingMode mode, Address addr) {
    this->op<Opcode::LSR>(mode, addr);
    this->op<Opcode::EOR>(mode, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STA
template<>
void CPU::op<Opcode::STA>(AddressingMode, Address addr) {
    this->write(addr, this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STP
template<>
void CPU::op<Opcode::STP>(AddressingMode, Address addr) {
    // TODO: check if this was a good guess
    this->write(addr, this->status.to_ulong());
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STX
template<>
void CPU::op<Opcode::STX>(AddressingMode, Address addr) {
    this->write(addr, this->regX);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STY
template<>
void CPU::op<Opcode::STY>(AddressingMode, Address addr) {
    this->write(addr, this->regY);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAS
template<>
void CPU::op<Opcode::TAS>(AddressingMode mode, Address addr) {
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAX
template<>
void CPU::op<Opcode::TAX>(AddressingMode mode, Address addr) {
    this->regX = this->regA;
    this->setNZ(this->regX);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAY
template<>
void CPU::op<Opcode::TAY>(AddressingMode mode, Address addr) {
    this->regY = this->regA;
    this->setNZ(this->regY);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TSX
template<>
void CPU::op<Opcode::TSX>(AddressingMode mode, Address addr) {
    this->regX = this->regSP;
    this->setNZ(this->regX);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXA
template<>
void CPU::op<Opcode::TXA>(AddressingMode mode, Address addr) {
    this->regA = this->regX;
    this->setNZ(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXS
template<>
void CPU::op<Opcode::TXS>(AddressingMode mode, Address addr) {
    this->regSP = this->regX;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TYA
template<>
void CPU::op<Opcode::TYA>(AddressingMode mode, Address addr) {
    this->regA = this->regY;
    this->setNZ(this->regA);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#XAA
template<>
void CPU::op<Opcode::XAA>(AddressingMode, Address) {
}


template<>
void CPU::op<Opcode::ISB>(AddressingMode mode, Address addr) {
    this->op<Opcode::INC>(mode, addr);
    this->op<Opcode::SBC>(mode, addr);
}

template<>
void CPU::op<Opcode::RLA>(AddressingMode mode, Address addr) {
    this->op<Opcode::ROL>(mode, addr);
    this->op<Opcode::AND>(mode, addr);
}


void CPU::dispatch(const DecodedInstruction &decodedInstruction, Address addr) {
    switch (decodedInstruction.opcode) {
        case Opcode::ADC:
            return this->op<Opcode::ADC>(decodedInstruction.addressingMode, addr);
        case Opcode::AHX:
            return this->op<Opcode::AHX>(decodedInstruction.addressingMode, addr);
        case Opcode::ALR:
            return this->op<Opcode::ALR>(decodedInstruction.addressingMode, addr);
        case Opcode::ANC:
            return this->op<Opcode::ANC>(decodedInstruction.addressingMode, addr);
        case Opcode::AND:
            return this->op<Opcode::AND>(decodedInstruction.addressingMode, addr);
        case Opcode::ARR:
            return this->op<Opcode::ARR>(decodedInstruction.addressingMode, addr);
        case Opcode::ASL:
            return this->op<Opcode::ASL>(decodedInstruction.addressingMode, addr);
        case Opcode::AXS:
            return this->op<Opcode::AXS>(decodedInstruction.addressingMode, addr);
        case Opcode::BCC:
            return this->op<Opcode::BCC>(decodedInstruction.addressingMode, addr);
        case Opcode::BCS:
            return this->op<Opcode::BCS>(decodedInstruction.addressingMode, addr);
        case Opcode::BEQ:
            return this->op<Opcode::BEQ>(decodedInstruction.addressingMode, addr);
        case Opcode::BIT:
            return this->op<Opcode::BIT>(decodedInstruction.addressingMode, addr);
        case Opcode::BMI:
            return this->op<Opcode::BMI>(decodedInstruction.addressingMode, addr);
        case Opcode::BNE:
            return this->op<Opcode::BNE>(decodedInstruction.addressingMode, addr);
        case Opcode::BPL:
            return this->op<Opcode::BPL>(decodedInstruction.addressingMode, addr);
        case Opcode::BRK:
            return this->op<Opcode::BRK>(decodedInstruction.addressingMode, addr);
        case Opcode::BVC:
            return this->op<Opcode::BVC>(decodedInstruction.addressingMode, addr);
        case Opcode::BVS:
            return this->op<Opcode::BVS>(decodedInstruction.addressingMode, addr);
        case Opcode::CLC:
            return this->op<Opcode::CLC>(decodedInstruction.addressingMode, addr);
        case Opcode::CLD:
            return this->op<Opcode::CLD>(decodedInstruction.addressingMode, addr);
        case Opcode::CLI:
            return this->op<Opcode::CLI>(decodedInstruction.addressingMode, addr);
        case Opcode::CLV:
            return this->op<Opcode::CLV>(decodedInstruction.addressingMode, addr);
        case Opcode::CMP:
            return this->op<Opcode::CMP>(decodedInstruction.addressingMode, addr);
        case Opcode::CPX:
            return this->op<Opcode::CPX>(decodedInstruction.addressingMode, addr);
        case Opcode::CPY:
            return this->op<Opcode::CPY>(decodedInstruction.addressingMode, addr);
        case Opcode::DCP:
            return this->op<Opcode::DCP>(decodedInstruction.addressingMode, addr);
        case Opcode::DEC:
            return this->op<Opcode::DEC>(decodedInstruction.addressingMode, addr);
        case Opcode::DEX:
            return this->op<Opcode::DEX>(decodedInstruction.addressingMode, addr);
        case Opcode::DEY:
            return this->op<Opcode::DEY>(decodedInstruction.addressingMode, addr);
        case Opcode::EOR:
            return this->op<Opcode::EOR>(decodedInstruction.addressingMode, addr);
        case Opcode::INC:
            return this->op<Opcode::INC>(decodedInstruction.addressingMode, addr);
        case Opcode::INX:
            return this->op<Opcode::INX>(decodedInstruction.addressingMode, addr);
        case Opcode::INY:
            return this->op<Opcode::INY>(decodedInstruction.addressingMode, addr);
        case Opcode::ISB:
            return this->op<Opcode::ISB>(decodedInstruction.addressingMode, addr);
        case Opcode::JMP:
            return this->op<Opcode::JMP>(decodedInstruction.addressingMode, addr);
        case Opcode::JSR:
            return this->op<Opcode::JSR>(decodedInstruction.addressingMode, addr);
        case Opcode::LAS:
            return this->op<Opcode::LAS>(decodedInstruction.addressingMode, addr);
        case Opcode::LAX:
            return this->op<Opcode::LAX>(decodedInstruction.addressingMode, addr);
        case Opcode::LDA:
            return this->op<Opcode::LDA>(decodedInstruction.addressingMode, addr);
        case Opcode::LDX:
            return this->op<Opcode::LDX>(decodedInstruction.addressingMode, addr);
        case Opcode::LDY:
            return this->op<Opcode::LDY>(decodedInstruction.addressingMode, addr);
        case Opcode::LSR:
            return this->op<Opcode::LSR>(decodedInstruction.addressingMode, addr);
        case Opcode::NOP:
            return this->op<Opcode::NOP>(decodedInstruction.addressingMode, addr);
        case Opcode::ORA:
            return this->op<Opcode::ORA>(decodedInstruction.addressingMode, addr);
        case Opcode::PHA:
            return this->op<Opcode::PHA>(decodedInstruction.addressingMode, addr);
        case Opcode::PHP:
            return this->op<Opcode::PHP>(decodedInstruction.addressingMode, addr);
        case Opcode::PLA:
            return this->op<Opcode::PLA>(decodedInstruction.addressingMode, addr);
        case Opcode::PLP:
            return this->op<Opcode::PLP>(decodedInstruction.addressingMode, addr);
        case Opcode::RLA:
            return this->op<Opcode::RLA>(decodedInstruction.addressingMode, addr);
        case Opcode::ROL:
            return this->op<Opcode::ROL>(decodedInstruction.addressingMode, addr);
        case Opcode::ROR:
            return this->op<Opcode::ROR>(decodedInstruction.addressingMode, addr);
        case Opcode::RRA:
            return this->op<Opcode::RRA>(decodedInstruction.addressingMode, addr);
        case Opcode::RTI:
            return this->op<Opcode::RTI>(decodedInstruction.addressingMode, addr);
        case Opcode::RTS:
            return this->op<Opcode::RTS>(decodedInstruction.addressingMode, addr);
        case Opcode::SAX:
            return this->op<Opcode::SAX>(decodedInstruction.addressingMode, addr);
        case Opcode::SBC:
            return this->op<Opcode::SBC>(decodedInstruction.addressingMode, addr);
        case Opcode::SEC:
            return this->op<Opcode::SEC>(decodedInstruction.addressingMode, addr);
        case Opcode::SED:
            return this->op<Opcode::SED>(decodedInstruction.addressingMode, addr);
        case Opcode::SEI:
            return this->op<Opcode::SEI>(decodedInstruction.addressingMode, addr);
        case Opcode::SHX:
            return this->op<Opcode::SHX>(decodedInstruction.addressingMode, addr);
        case Opcode::SHY:
            return this->op<Opcode::SHY>(decodedInstruction.addressingMode, addr);
        case Opcode::SLO:
            return this->op<Opcode::SLO>(decodedInstruction.addressingMode, addr);
        case Opcode::SRE:
            return this->op<Opcode::SRE>(decodedInstruction.addressingMode, addr);
        case Opcode::STA:
            return this->op<Opcode::STA>(decodedInstruction.addressingMode, addr);
        case Opcode::STP:
            return this->op<Opcode::STP>(decodedInstruction.addressingMode, addr);
        case Opcode::STX:
            return this->op<Opcode::STX>(decodedInstruction.addressingMode, addr);
        case Opcode::STY:
            return this->op<Opcode::STY>(decodedInstruction.addressingMode, addr);
        case Opcode::TAS:
            return this->op<Opcode::TAS>(decodedInstruction.addressingMode, addr);
        case Opcode::TAX:
            return this->op<Opcode::TAX>(decodedInstruction.addressingMode, addr);
        case Opcode::TAY:
            return this->op<Opcode::TAY>(decodedInstruction.addressingMode, addr);
        case Opcode::TSX:
            return this->op<Opcode::TSX>(decodedInstruction.addressingMode, addr);
        case Opcode::TXA:
            return this->op<Opcode::TXA>(decodedInstruction.addressingMode, addr);
        case Opcode::TXS:
            return this->op<Opcode::TXS>(decodedInstruction.addressingMode, addr);
        case Opcode::TYA:
            return this->op<Opcode::TYA>(decodedInstruction.addressingMode, addr);
        case Opcode::XAA:
            return this->op<Opcode::XAA>(decodedInstruction.addressingMode, addr);
    }
}

void CPU::setNZ(Byte data) {
    this->status[Flag::N] = (data & 0x80) != 0;
    this->status[Flag::Z] = data == 0;
}

void CPU::setCNZ(WordWithCarry data) {
    this->status[Flag::C] = (data & 0x0100) != 0;
    this->status[Flag::N] = (data & 0x80) != 0;
    this->status[Flag::Z] = (data & 0xff) == 0;
}


void CPU::push(Byte data) {
    this->write(0x100 + Address(this->regSP), data);
    this->regSP--;
}

void CPU::pushAddress(Address addr) {
    this->push(addr >> 8);
    this->push(addr & 0xff);
}

Byte CPU::pop() {
    this->regSP++;
    auto data = this->read(0x100 + this->regSP);
    return data;
}

Address CPU::popAddress() {
    Address lo = this->pop();
    Address hi = this->pop();
    return hi << 8 | lo;
}

Byte CPU::read(Address addr) const {
    // https://www.nesdev.org/wiki/CPU_memory_map
    if (addr < 0x2000)
        return this->ram[addr % this->ram.size()];
    else if (addr < 0x4000)
        return this->console.ppu->readRegister(0x2000 | (addr & 0xf));
    else if (addr == 0x4016)
        return this->console.controller.Read();
    else if (addr < 0x4018)
        return this->console.apu->readRegister(addr);
    else if (addr < 0x401f)
        // only enabled for CPU test mode
        return 0;
    else
        return this->console.mapper->Read(addr);

    return 0;
}

const Byte *CPU::DMAStart(nes::Address addr) const {
    // https://www.nesdev.org/wiki/CPU_memory_map
    if (addr <= (0x2000 - 256))
        return &this->ram[addr % this->ram.size()];

    return this->console.mapper->DMAStart(addr);
}

void CPU::write(Address addr, Byte data) {
    if (addr < 0x2000)
        this->ram[addr % this->ram.size()] = data;
    else if (addr < 0x4000)
        this->console.ppu->writeRegister(0x2000 | (addr & 0x7), data);
    else if (addr == 0x4014) {
        // https://www.nesdev.org/wiki/PPU_registers#OAM_DMA_($4014)_%3E_write
        this->console.ppu->writeDMA(this->DMAStart(Address(data) << 8));
        this->cycle += 512;
        this->cycle++;
        this->cycle += (this->cycle & 1); // extra cycle for odd
    } else if (addr == 0x4016)
        return this->console.controller.Write(data);
    else if (addr < 0x4018)
        return this->console.apu->writeRegister(addr, data);
    else if (addr < 0x401f)
        // only enabled for CPU test mode
        return;
    else
        this->console.mapper->Write(addr, data);
}

Address CPU::readAddress(Address addr) const {
    auto lowBits  = this->read(addr);
    auto highBits = this->read(addr + 1);

    return highBits << 8 | lowBits;
}

Address CPU::readAddressIndirectWraparound(Address addr) const {
    // there was a bug in the original if the high byte crosses a page boundary,
    // it instead will wrap around
    auto loAddr   = addr;
    auto hiAddr   = (loAddr & 0xff00) | (0x00ff & (loAddr + 1));

    auto lowBits  = this->read(loAddr);
    auto highBits = this->read(hiAddr);

    return highBits << 8 | lowBits;
}

void CPU::interrupt(Interrupt interrupt) {
    switch (interrupt) {
        case Interrupt::IRQ:
            if (!this->status[Flag::I])
                this->pendingInterrupt = interrupt;
            break;
        case Interrupt::NMI:
            this->pendingInterrupt = interrupt;
            break;
        case Interrupt::None:
            break;
    }
}

void CPU::PC(Address addr) {
    this->pc = addr;
}

bool CPU::handleInterrupt() {
    Address handlerAddress;
    switch (this->pendingInterrupt) {
        case Interrupt::None:
            return false;
        case Interrupt::NMI:
            handlerAddress = 0xFFFA;
            break;
        case Interrupt::IRQ:
            handlerAddress = 0xFFFE;
            break;
    }

    this->pushAddress(this->pc);
    // TODO: verify with https://www.nesdev.org/wiki/Status_flags#The_B_flag
    this->op<Opcode::PHP>(AddressingMode::Implied, 0);
    this->pc               = this->readAddress(handlerAddress);
    this->status[Flag::I]  = true;
    this->pendingInterrupt = Interrupt::None;
    this->cycle += 7;
    return true;
}

void CPU::reset() {
    // https://www.nesdev.org/wiki/CPU_ALL#At_power-up
    this->regA            = 0;
    this->regX            = 0;
    this->regY            = 0;
    this->regSP           = 0xfd;
    this->status          = 0;
    this->cycle           = 7;

    this->status[Flag::U] = true;
    this->status[Flag::I] = true;
    //    this->status[Flag::B] = true;

    this->pc = this->readAddress(0xfffc);

    this->write(0x4017, 0x40); // disable frame IRQ
    this->write(0x4015, 0x40); // disable all channels

    for (Address addr = 0x4000; addr <= 0x4013; addr++)
        this->write(addr, 0x0);
}

CPU::CPU(Console &c) :
    console(c) {
    this->reset();
}
} // namespace nes