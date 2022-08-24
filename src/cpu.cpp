#include "cpu.h"
#include "opcodes.def"
#include <iostream>
#include <string_view>

#define DBG_PRINT 1

namespace nes {

//static_assert(__cplusplus == 0, "wtf");

// clang-format off
static const DecodedInstruction decodeTable[256]{
        {Opcode::BRK, AddressingMode::Implied},
        {Opcode::ORA, AddressingMode::IndexedIndirect},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::SLO, AddressingMode::IndexedIndirect},
        {Opcode::NOP, AddressingMode::ZeroPage},
        {Opcode::ORA, AddressingMode::ZeroPage},
        {Opcode::ASL, AddressingMode::ZeroPage},
        {Opcode::SLO, AddressingMode::ZeroPage},
        {Opcode::PHP, AddressingMode::Implied},
        {Opcode::ORA, AddressingMode::Immediate},
        {Opcode::ASL, AddressingMode::Accumulator},
        {Opcode::ANC, AddressingMode::Immediate},
        {Opcode::NOP, AddressingMode::Absolute},
        {Opcode::ORA, AddressingMode::Absolute},
        {Opcode::ASL, AddressingMode::Absolute},
        {Opcode::SLO, AddressingMode::Absolute},
        {Opcode::BPL, AddressingMode::Relative},
        {Opcode::ORA, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::SLO, AddressingMode::IndirectIndexed},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX},
        {Opcode::ORA, AddressingMode::ZeroPageIndexedX},
        {Opcode::ASL, AddressingMode::ZeroPageIndexedX},
        {Opcode::SLO, AddressingMode::ZeroPageIndexedX},
        {Opcode::CLC, AddressingMode::Implied},
        {Opcode::ORA, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::SLO, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX},
        {Opcode::ORA, AddressingMode::AbsoluteIndexedX},
        {Opcode::ASL, AddressingMode::AbsoluteIndexedX},
        {Opcode::SLO, AddressingMode::AbsoluteIndexedX},
        {Opcode::JSR, AddressingMode::Absolute},
        {Opcode::AND, AddressingMode::IndexedIndirect},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::RLA, AddressingMode::IndexedIndirect},
        {Opcode::BIT, AddressingMode::ZeroPage},
        {Opcode::AND, AddressingMode::ZeroPage},
        {Opcode::ROL, AddressingMode::ZeroPage},
        {Opcode::RLA, AddressingMode::ZeroPage},
        {Opcode::PLP, AddressingMode::Implied},
        {Opcode::AND, AddressingMode::Immediate},
        {Opcode::ROL, AddressingMode::Accumulator},
        {Opcode::ANC, AddressingMode::Immediate},
        {Opcode::BIT, AddressingMode::Absolute},
        {Opcode::AND, AddressingMode::Absolute},
        {Opcode::ROL, AddressingMode::Absolute},
        {Opcode::RLA, AddressingMode::Absolute},
        {Opcode::BMI, AddressingMode::Relative},
        {Opcode::AND, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::RLA, AddressingMode::IndirectIndexed},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX},
        {Opcode::AND, AddressingMode::ZeroPageIndexedX},
        {Opcode::ROL, AddressingMode::ZeroPageIndexedX},
        {Opcode::RLA, AddressingMode::ZeroPageIndexedX},
        {Opcode::SEC, AddressingMode::Implied},
        {Opcode::AND, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::RLA, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX},
        {Opcode::AND, AddressingMode::AbsoluteIndexedX},
        {Opcode::ROL, AddressingMode::AbsoluteIndexedX},
        {Opcode::RLA, AddressingMode::AbsoluteIndexedX},
        {Opcode::RTI, AddressingMode::Implied},
        {Opcode::EOR, AddressingMode::IndexedIndirect},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::SRE, AddressingMode::IndexedIndirect},
        {Opcode::NOP, AddressingMode::ZeroPage},
        {Opcode::EOR, AddressingMode::ZeroPage},
        {Opcode::LSR, AddressingMode::ZeroPage},
        {Opcode::SRE, AddressingMode::ZeroPage},
        {Opcode::PHA, AddressingMode::Implied},
        {Opcode::EOR, AddressingMode::Immediate},
        {Opcode::LSR, AddressingMode::Accumulator},
        {Opcode::ALR, AddressingMode::Immediate},
        {Opcode::JMP, AddressingMode::Absolute},
        {Opcode::EOR, AddressingMode::Absolute},
        {Opcode::LSR, AddressingMode::Absolute},
        {Opcode::SRE, AddressingMode::Absolute},
        {Opcode::BVC, AddressingMode::Relative},
        {Opcode::EOR, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::SRE, AddressingMode::IndirectIndexed},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX},
        {Opcode::EOR, AddressingMode::ZeroPageIndexedX},
        {Opcode::LSR, AddressingMode::ZeroPageIndexedX},
        {Opcode::SRE, AddressingMode::ZeroPageIndexedX},
        {Opcode::CLI, AddressingMode::Implied},
        {Opcode::EOR, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::SRE, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX},
        {Opcode::EOR, AddressingMode::AbsoluteIndexedX},
        {Opcode::LSR, AddressingMode::AbsoluteIndexedX},
        {Opcode::SRE, AddressingMode::AbsoluteIndexedX},
        {Opcode::RTS, AddressingMode::Implied},
        {Opcode::ADC, AddressingMode::IndexedIndirect},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::RRA, AddressingMode::IndexedIndirect},
        {Opcode::NOP, AddressingMode::ZeroPage},
        {Opcode::ADC, AddressingMode::ZeroPage},
        {Opcode::ROR, AddressingMode::ZeroPage},
        {Opcode::RRA, AddressingMode::ZeroPage},
        {Opcode::PLA, AddressingMode::Implied},
        {Opcode::ADC, AddressingMode::Immediate},
        {Opcode::ROR, AddressingMode::Accumulator},
        {Opcode::ARR, AddressingMode::Immediate},
        {Opcode::JMP, AddressingMode::Indirect},
        {Opcode::ADC, AddressingMode::Absolute},
        {Opcode::ROR, AddressingMode::Absolute},
        {Opcode::RRA, AddressingMode::Absolute},
        {Opcode::BVS, AddressingMode::Relative},
        {Opcode::ADC, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::RRA, AddressingMode::IndirectIndexed},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX},
        {Opcode::ADC, AddressingMode::ZeroPageIndexedX},
        {Opcode::ROR, AddressingMode::ZeroPageIndexedX},
        {Opcode::RRA, AddressingMode::ZeroPageIndexedX},
        {Opcode::SEI, AddressingMode::Implied},
        {Opcode::ADC, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::RRA, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX},
        {Opcode::ADC, AddressingMode::AbsoluteIndexedX},
        {Opcode::ROR, AddressingMode::AbsoluteIndexedX},
        {Opcode::RRA, AddressingMode::AbsoluteIndexedX},
        {Opcode::NOP, AddressingMode::Immediate},
        {Opcode::STA, AddressingMode::IndexedIndirect},
        {Opcode::NOP, AddressingMode::Immediate},
        {Opcode::SAX, AddressingMode::IndexedIndirect},
        {Opcode::STY, AddressingMode::ZeroPage},
        {Opcode::STA, AddressingMode::ZeroPage},
        {Opcode::STX, AddressingMode::ZeroPage},
        {Opcode::SAX, AddressingMode::ZeroPage},
        {Opcode::DEY, AddressingMode::Implied},
        {Opcode::NOP, AddressingMode::Immediate},
        {Opcode::TXA, AddressingMode::Implied},
        {Opcode::XAA, AddressingMode::Immediate},
        {Opcode::STY, AddressingMode::Absolute},
        {Opcode::STA, AddressingMode::Absolute},
        {Opcode::STX, AddressingMode::Absolute},
        {Opcode::SAX, AddressingMode::Absolute},
        {Opcode::BCC, AddressingMode::Relative},
        {Opcode::STA, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::AHX, AddressingMode::IndirectIndexed},
        {Opcode::STY, AddressingMode::ZeroPageIndexedX},
        {Opcode::STA, AddressingMode::ZeroPageIndexedX},
        {Opcode::STX, AddressingMode::ZeroPageIndexedY},
        {Opcode::SAX, AddressingMode::ZeroPageIndexedY},
        {Opcode::TYA, AddressingMode::Implied},
        {Opcode::STA, AddressingMode::AbsoluteIndexedY},
        {Opcode::TXS, AddressingMode::Implied},
        {Opcode::TAS, AddressingMode::AbsoluteIndexedY},
        {Opcode::SHY, AddressingMode::AbsoluteIndexedX},
        {Opcode::STA, AddressingMode::AbsoluteIndexedX},
        {Opcode::SHX, AddressingMode::AbsoluteIndexedY},
        {Opcode::AHX, AddressingMode::AbsoluteIndexedY},
        {Opcode::LDY, AddressingMode::Immediate},
        {Opcode::LDA, AddressingMode::IndexedIndirect},
        {Opcode::LDX, AddressingMode::Immediate},
        {Opcode::LAX, AddressingMode::IndexedIndirect},
        {Opcode::LDY, AddressingMode::ZeroPage},
        {Opcode::LDA, AddressingMode::ZeroPage},
        {Opcode::LDX, AddressingMode::ZeroPage},
        {Opcode::LAX, AddressingMode::ZeroPage},
        {Opcode::TAY, AddressingMode::Implied},
        {Opcode::LDA, AddressingMode::Immediate},
        {Opcode::TAX, AddressingMode::Implied},
        {Opcode::LAX, AddressingMode::Immediate},
        {Opcode::LDY, AddressingMode::Absolute},
        {Opcode::LDA, AddressingMode::Absolute},
        {Opcode::LDX, AddressingMode::Absolute},
        {Opcode::LAX, AddressingMode::Absolute},
        {Opcode::BCS, AddressingMode::Relative},
        {Opcode::LDA, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::LAX, AddressingMode::IndirectIndexed},
        {Opcode::LDY, AddressingMode::ZeroPageIndexedX},
        {Opcode::LDA, AddressingMode::ZeroPageIndexedX},
        {Opcode::LDX, AddressingMode::ZeroPageIndexedY},
        {Opcode::LAX, AddressingMode::ZeroPageIndexedY},
        {Opcode::CLV, AddressingMode::Implied},
        {Opcode::LDA, AddressingMode::AbsoluteIndexedY},
        {Opcode::TSX, AddressingMode::Implied},
        {Opcode::LAS, AddressingMode::AbsoluteIndexedY},
        {Opcode::LDY, AddressingMode::AbsoluteIndexedX},
        {Opcode::LDA, AddressingMode::AbsoluteIndexedX},
        {Opcode::LDX, AddressingMode::AbsoluteIndexedY},
        {Opcode::LAX, AddressingMode::AbsoluteIndexedY},
        {Opcode::CPY, AddressingMode::Immediate},
        {Opcode::CMP, AddressingMode::IndexedIndirect},
        {Opcode::NOP, AddressingMode::Immediate},
        {Opcode::DCP, AddressingMode::IndexedIndirect},
        {Opcode::CPY, AddressingMode::ZeroPage},
        {Opcode::CMP, AddressingMode::ZeroPage},
        {Opcode::DEC, AddressingMode::ZeroPage},
        {Opcode::DCP, AddressingMode::ZeroPage},
        {Opcode::INY, AddressingMode::Implied},
        {Opcode::CMP, AddressingMode::Immediate},
        {Opcode::DEX, AddressingMode::Implied},
        {Opcode::AXS, AddressingMode::Immediate},
        {Opcode::CPY, AddressingMode::Absolute},
        {Opcode::CMP, AddressingMode::Absolute},
        {Opcode::DEC, AddressingMode::Absolute},
        {Opcode::DCP, AddressingMode::Absolute},
        {Opcode::BNE, AddressingMode::Relative},
        {Opcode::CMP, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::DCP, AddressingMode::IndirectIndexed},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX},
        {Opcode::CMP, AddressingMode::ZeroPageIndexedX},
        {Opcode::DEC, AddressingMode::ZeroPageIndexedX},
        {Opcode::DCP, AddressingMode::ZeroPageIndexedX},
        {Opcode::CLD, AddressingMode::Implied},
        {Opcode::CMP, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::DCP, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX},
        {Opcode::CMP, AddressingMode::AbsoluteIndexedX},
        {Opcode::DEC, AddressingMode::AbsoluteIndexedX},
        {Opcode::DCP, AddressingMode::AbsoluteIndexedX},
        {Opcode::CPX, AddressingMode::Immediate},
        {Opcode::SBC, AddressingMode::IndexedIndirect},
        {Opcode::NOP, AddressingMode::Immediate},
        {Opcode:: ISB, AddressingMode::IndexedIndirect},
        {Opcode::CPX, AddressingMode::ZeroPage},
        {Opcode::SBC, AddressingMode::ZeroPage},
        {Opcode::INC, AddressingMode::ZeroPage},
        {Opcode:: ISB, AddressingMode::ZeroPage},
        {Opcode::INX, AddressingMode::Implied},
        {Opcode::SBC, AddressingMode::Immediate},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::SBC, AddressingMode::Immediate},
        {Opcode::CPX, AddressingMode::Absolute},
        {Opcode::SBC, AddressingMode::Absolute},
        {Opcode::INC, AddressingMode::Absolute},
        {Opcode:: ISB, AddressingMode::Absolute},
        {Opcode::BEQ, AddressingMode::Relative},
        {Opcode::SBC, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode:: ISB, AddressingMode::IndirectIndexed},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX},
        {Opcode::SBC, AddressingMode::ZeroPageIndexedX},
        {Opcode::INC, AddressingMode::ZeroPageIndexedX},
        {Opcode:: ISB, AddressingMode::ZeroPageIndexedX},
        {Opcode::SED, AddressingMode::Implied},
        {Opcode::SBC, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode:: ISB, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX},
        {Opcode::SBC, AddressingMode::AbsoluteIndexedX},
        {Opcode::INC, AddressingMode::AbsoluteIndexedX},
        {Opcode:: ISB, AddressingMode::AbsoluteIndexedX},
};
// clang-format on

static bool crossesPageBoundary(Address a, Address b) {
    return (a & 0xff00) != (b & 0xff00);
}

uint8_t CPU::step() {
    auto prePC = this->pc;
    auto instFirstByte = this->read(this->pc);
    auto decoded = decodeTable[instFirstByte];

    Address address = 0;
    Address indirect = 0;
    Word offset = 0;

    this->pc++;
    uint8_t numCycles = 1;

    switch (decoded.addressingMode) {
        case AddressingMode::Implied:
        case AddressingMode::Accumulator:
            // read + alu cycles tracked in instFirstByte
            break;
        case AddressingMode::Absolute:
            address = this->readAddress(this->pc);
            this->pc += 2;
            numCycles += 2; // read + read
            break;
        case AddressingMode::Immediate:
            address = this->pc;
            this->pc++;
            numCycles += 0; // address already known
            break;
        case AddressingMode::AbsoluteIndexedX:
            indirect = this->readAddress(this->pc);
            address = indirect + this->regX;
            this->pc += 2;
            numCycles += 3; // read + read + add
            break;
        case AddressingMode::AbsoluteIndexedY:
            indirect = this->readAddress(this->pc);
            address = indirect + this->regY;
            this->pc += 2;
            numCycles += 3; // read + read + add
            break;
        case AddressingMode::IndexedIndirect:
            // val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256)
            // val = PEEK(
            // zero page wrap around
            offset = this->read(this->pc);
            indirect = Word(offset + regX);
            address = this->readAddressBug(indirect);
            this->pc += 1;
            numCycles += 4; // read + add
            break;
        case AddressingMode::Indirect:
            indirect = this->readAddress(this->pc);
            address = this->readAddressBug(indirect);
            this->pc += 2;
            numCycles += 2;
            break;
        case AddressingMode::IndirectIndexed:
            // val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)
            offset = this->read(this->pc);
            indirect = this->readAddressBug(offset);
            address = indirect + this->regY;
            this->pc += 1;
            numCycles += 3; // read + read + read + add
            break;
        case AddressingMode::Relative:
            // TODO: check that this handles negative offsets correctly
            offset = this->read(this->pc);
            this->pc++;
            numCycles++;

            if (offset & 0x80)
                address = this->pc - Address(0x100 - offset);
            else
                address = this->pc + Address(offset);
            break;
        case AddressingMode::ZeroPage:
            offset = this->read(this->pc);
            address = offset;
            this->pc++;
            numCycles++;
            break;
        case AddressingMode::ZeroPageIndexedX:
            offset = this->read(this->pc);
            address = Word(offset + this->regX);
            numCycles++;
            this->pc++;
            break;
        case AddressingMode::ZeroPageIndexedY:
            offset = this->read(this->pc);
            address = Word(offset + this->regY);
            numCycles++;
            this->pc++;
            break;
    };

    auto nextPC = this->pc;

    // every instFirstByte should be at least two cycles
    numCycles += numCycles < 2;
    this->cycle += numCycles;


    if (DBG_PRINT) {
        // debug buffer, more than enough to include everything
        // C000  4C F5 C5  JMP $C5F5                       A:00 X:00 Y:00 P:24 SP:FD
        char buf[80] = "";
        char *remaining = buf;
        remaining += sprintf(remaining, "%04X  ", prePC);

        // print each byte in the instFirstByte
        uint8_t width = this->pc - prePC;

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
        remaining += sprintf(remaining, "A:%02X X:%02X Y:%02X P:%02X SP:%02X", this->regA, this->regX, this->regY,
                             uint8_t(this->status.to_ulong()), this->regSP);

        (void) remaining;
        std::cout << buf << std::endl;
    }

    // add this point, num cycles doesn't include any Read operations,
    // and it only contains time for address calculations
    numCycles += this->dispatch(decoded, address);


    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ADC
template<>
uint8_t CPU::op<Opcode::ADC>(AddressingMode mode, Address addr) {
    WordWithCarry a = this->regA;
    WordWithCarry b = this->read(addr);
    WordWithCarry c = this->status[Flag::C];
    auto sum = a + b + c;
    this->regA = Word(sum);

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

    // read (no ALU hit apparently)
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AHX
template<>
uint8_t CPU::op<Opcode::AHX>(AddressingMode mode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ALR
template<>
uint8_t CPU::op<Opcode::ALR>(AddressingMode mode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ANC
template<>
uint8_t CPU::op<Opcode::ANC>(AddressingMode mode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AND
template<>
uint8_t CPU::op<Opcode::AND>(AddressingMode, Address addr) {
    this->regA &= this->read(addr);
    this->setNZ(this->regA);
    // read + ALU
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ARR
template<>
uint8_t CPU::op<Opcode::ARR>(AddressingMode mode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ASL
template<>
uint8_t CPU::op<Opcode::ASL>(AddressingMode mode, Address addr) {
    WordWithCarry resultWide = 0;
    uint8_t numCycles = 2;

    if (mode == AddressingMode::Accumulator) {
        resultWide = WordWithCarry(this->regA) << 1;
        this->regA = Word(resultWide);
    } else {
        resultWide = WordWithCarry(this->read(addr)) << 1;
        this->write(addr, Word(resultWide));
        numCycles += 2;
    }

    this->setCNZ(resultWide);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AXS
template<>
uint8_t CPU::op<Opcode::AXS>(AddressingMode mode, Address addr) {
    return 0;
}


uint8_t CPU::BXX(Flag flag, bool isSet, Address addr) {
    uint8_t numCycles = 0;

    if (this->status[flag] == isSet) {
        numCycles = 1 + crossesPageBoundary(this->pc, addr);
        this->pc = addr;
    }

    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCC
template<>
uint8_t CPU::op<Opcode::BCC>(AddressingMode, Address addr) {
    return this->BXX(Flag::C, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCS
template<>
uint8_t CPU::op<Opcode::BCS>(AddressingMode, Address addr) {
    return this->BXX(Flag::C, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BEQ
template<>
uint8_t CPU::op<Opcode::BEQ>(AddressingMode, Address addr) {
    return this->BXX(Flag::Z, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BIT
template<>
uint8_t CPU::op<Opcode::BIT>(AddressingMode, Address addr) {
    auto m = this->read(addr);
    auto result = this->regA & m;
    this->status[Flag::Z] = (result == 0);
    this->status[Flag::V] = (m & 0x40) != 0;
    this->status[Flag::N] = (m & 0x80) != 0;
    // read
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BMI
template<>
uint8_t CPU::op<Opcode::BMI>(AddressingMode, Address addr) {
    return this->BXX(Flag::N, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BNE
template<>
uint8_t CPU::op<Opcode::BNE>(AddressingMode, Address addr) {
    return this->BXX(Flag::Z, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BPL
template<>
uint8_t CPU::op<Opcode::BPL>(AddressingMode, Address addr) {
    return this->BXX(Flag::N, false, addr);
}

template<>
uint8_t CPU::op<Opcode::BRK>(AddressingMode, Address) {
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

    this->status[Flag::B] = true;

    this->pushAddress(this->pc);
    this->push(Word(this->status.to_ulong()));

    this->readAddress(0xfffe);

    return 7;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVC
template<>
uint8_t CPU::op<Opcode::BVC>(AddressingMode mode, Address addr) {
    return this->BXX(Flag::V, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVS
template<>
uint8_t CPU::op<Opcode::BVS>(AddressingMode mode, Address addr) {
    return this->BXX(Flag::V, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLC
template<>
uint8_t CPU::op<Opcode::CLC>(AddressingMode mode, Address addr) {
    this->status[Flag::C] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLD
template<>
uint8_t CPU::op<Opcode::CLD>(AddressingMode mode, Address addr) {
    this->status[Flag::D] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLI
template<>
uint8_t CPU::op<Opcode::CLI>(AddressingMode mode, Address addr) {
    this->status[Flag::I] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLV
template<>
uint8_t CPU::op<Opcode::CLV>(AddressingMode mode, Address addr) {
    this->status[Flag::V] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CMP
template<>
uint8_t CPU::op<Opcode::CMP>(AddressingMode mode, Address addr) {
    auto a = this->regA;
    auto m = this->read(addr);
    auto data = a - m;
    this->setNZ(data);
    this->status[Flag::C] = a >= m;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPX
template<>
uint8_t CPU::op<Opcode::CPX>(AddressingMode mode, Address addr) {
    auto x = this->regX;
    auto m = this->read(addr);
    this->setNZ(x - m);
    this->status[Flag::C] = x >= m;
    // read
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPY
template<>
uint8_t CPU::op<Opcode::CPY>(AddressingMode mode, Address addr) {
    auto y = this->regY;
    auto m = this->read(addr);
    auto data = y - m;
    this->setNZ(data);
    this->status[Flag::C] = y >= m;
    // read
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEC
template<>
uint8_t CPU::op<Opcode::DEC>(AddressingMode, Address addr) {
    auto m = this->read(addr) - 1;
    this->write(addr, m);
    this->setNZ(m);
    // read + alu + write
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DCP
template<>
uint8_t CPU::op<Opcode::DCP>(AddressingMode mode, Address addr) {
    uint8_t numCycles = this->op<Opcode::DEC>(mode, addr);
    numCycles += this->op<Opcode::CMP>(mode, addr);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEX
template<>
uint8_t CPU::op<Opcode::DEX>(AddressingMode, Address) {
    this->regX--;
    this->setNZ(this->regX);
    // ALU
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEY
template<>
uint8_t CPU::op<Opcode::DEY>(AddressingMode, Address) {
    this->regY--;
    this->setNZ(this->regY);
    // ALU
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#EOR
template<>
uint8_t CPU::op<Opcode::EOR>(AddressingMode, Address addr) {
    this->regA ^= this->read(addr);
    this->setNZ(this->regA);
    // read
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INC
template<>
uint8_t CPU::op<Opcode::INC>(AddressingMode, Address addr) {
    auto data = this->read(addr) + 1;
    this->write(addr, data);
    this->setNZ(data);
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INX
template<>
uint8_t CPU::op<Opcode::INX>(AddressingMode, Address) {
    this->regX++;
    this->setNZ(this->regX);
    // 2 cycle minimum, already counted one
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INY
template<>
uint8_t CPU::op<Opcode::INY>(AddressingMode, Address) {
    this->regY++;
    this->setNZ(this->regY);
    // 2 cycle minimum, already counted one
    return 1;
}


// https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
template<>
uint8_t CPU::op<Opcode::JMP>(AddressingMode, Address addr) {
    this->pc = addr;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JSR
template<>
uint8_t CPU::op<Opcode::JSR>(AddressingMode, Address addr) {
    this->pushAddress(this->pc - 1);
    this->pc = addr;
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LAS
template<>
uint8_t CPU::op<Opcode::LAS>(AddressingMode mode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LAX
template<>
uint8_t CPU::op<Opcode::LAX>(AddressingMode mode, Address addr) {
    this->regA = this->regX = this->read(addr);
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDA
template<>
uint8_t CPU::op<Opcode::LDA>(AddressingMode mode, Address addr) {
    this->regA = this->read(addr);
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDX
template<>
uint8_t CPU::op<Opcode::LDX>(AddressingMode mode, Address addr) {
    this->regX = this->read(addr);
    this->setNZ(this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDY
template<>
uint8_t CPU::op<Opcode::LDY>(AddressingMode mode, Address addr) {
    this->regY = this->read(addr);
    this->setNZ(this->regY);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LSR
template<>
uint8_t CPU::op<Opcode::LSR>(AddressingMode mode, Address addr) {
    WordWithCarry wideData;
    uint8_t numCycles = 0;

    if (mode == AddressingMode::Accumulator) {
        wideData = WordWithCarry(this->regA);
        wideData = wideData >> 1 | (wideData & 0x01) << 8;
        this->regA = Word(wideData);
    } else {
        wideData = WordWithCarry(this->read(addr));
        wideData = wideData >> 1 | (wideData & 0x01) << 8;
        this->write(addr, Word(wideData));
        numCycles += 2;
    }

    // ALU
    numCycles++;
    this->setCNZ(wideData);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#NOP
template<>
uint8_t CPU::op<Opcode::NOP>(AddressingMode, Address) {
    // two cycle minimum
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ORA
template<>
uint8_t CPU::op<Opcode::ORA>(AddressingMode, Address addr) {
    this->regA |= this->read(addr);
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHA
template<>
uint8_t CPU::op<Opcode::PHA>(AddressingMode, Address) {
    this->push(this->regA);
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHP
template<>
uint8_t CPU::op<Opcode::PHP>(AddressingMode, Address) {
    auto statusCopy = this->status;
    // B flag is always set when pushed to the stack
    // https://www.nesdev.org/wiki/Status_flags#The_B_flag
    statusCopy[Flag::B] = true;

    this->push(statusCopy.to_ulong());
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLA
template<>
uint8_t CPU::op<Opcode::PLA>(AddressingMode, Address) {
    this->regA = this->pop();
    this->setNZ(regA);
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLP
template<>
uint8_t CPU::op<Opcode::PLP>(AddressingMode, Address) {
    // Two instructions (PLP and RTI) pull a byte from the stack and set all the flags.
    // They ignore bits 5 (U) and 4 (B).
    // https://www.nesdev.org/wiki/Status_flags
    auto prevStatus = this->status;
    this->status = this->pop();
    this->status[Flag::U] = prevStatus[Flag::U];
    this->status[Flag::B] = prevStatus[Flag::B];
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROL
template<>
uint8_t CPU::op<Opcode::ROL>(AddressingMode mode, Address addr) {
    WordWithCarry wideData;
    uint8_t numCycles = 0;

    if (mode == AddressingMode::Accumulator) {
        wideData = WordWithCarry(this->regA);
        wideData = wideData << 1 | this->status[Flag::C];
        this->regA = wideData;
    } else {
        wideData = WordWithCarry(this->read(addr));
        wideData = wideData << 1 | this->status[Flag::C];
        this->write(addr, Word(wideData));
        numCycles += 2;
    }

    // ALU
    numCycles++;
    this->setCNZ(wideData);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROR
template<>
uint8_t CPU::op<Opcode::ROR>(AddressingMode mode, Address addr) {
    WordWithCarry wideData;
    uint8_t numCycles = 0;

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
        this->write(addr, Word(wideData));
        numCycles += 2;
    }

    // ALU
    numCycles++;
    this->setCNZ(wideData);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RRA
template<>
uint8_t CPU::op<Opcode::RRA>(AddressingMode mode, Address addr) {
    auto numCycles = this->op<Opcode::ROR>(mode, addr);
    numCycles += this->op<Opcode::ADC>(mode, addr);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTI
template<>
uint8_t CPU::op<Opcode::RTI>(AddressingMode, Address) {
    // Two instructions (PLP and RTI) pull a byte from the stack and set all the flags.
    // They ignore bits 5 (U) and 4 (B).
    // https://www.nesdev.org/wiki/Status_flags
    auto prevStatus = this->status;
    this->status = this->pop();
    this->status[Flag::U] = prevStatus[Flag::U];
    this->status[Flag::B] = prevStatus[Flag::B];

    this->pc = this->popAddress();
    return 5;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTS
template<>
uint8_t CPU::op<Opcode::RTS>(AddressingMode, Address) {
    this->pc = this->popAddress() + 1;
    return 5;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SAX
template<>
uint8_t CPU::op<Opcode::SAX>(AddressingMode mode, Address addr) {
    this->write(addr, this->regA & this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SBC
template<>
uint8_t CPU::op<Opcode::SBC>(AddressingMode mode, Address addr) {
    auto a = WordWithCarry(this->regA);
    auto m = WordWithCarry(this->read(addr));

    auto result = a - m - (1 - this->status[Flag::C]);
    this->regA = Word(result);
    this->setNZ(result);

    // signs are the same before, but the sign changed after
    this->status[Flag::V] = (a ^ result) & (~m ^ result) & 0x80;
    this->status[Flag::C] = !(result & 0x100);

    // read (no ALU hit apparently)
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEC
template<>
uint8_t CPU::op<Opcode::SEC>(AddressingMode mode, Address addr) {
    this->status[Flag::C] = 1;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SED
template<>
uint8_t CPU::op<Opcode::SED>(AddressingMode, Address) {
    this->status[Flag::D] = 1;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEI
template<>
uint8_t CPU::op<Opcode::SEI>(AddressingMode, Address) {
    this->status[Flag::I] = 1;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SHX
template<>
uint8_t CPU::op<Opcode::SHX>(AddressingMode, Address) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SHY
template<>
uint8_t CPU::op<Opcode::SHY>(AddressingMode mode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SLO
template<>
uint8_t CPU::op<Opcode::SLO>(AddressingMode mode, Address addr) {
    auto numCycles = this->op<Opcode::ASL>(mode, addr);
    numCycles += this->op<Opcode::ORA>(mode, addr);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SRE
template<>
uint8_t CPU::op<Opcode::SRE>(AddressingMode mode, Address addr) {
    auto numCycles = this->op<Opcode::LSR>(mode, addr);
    numCycles += this->op<Opcode::EOR>(mode, addr);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STA
template<>
uint8_t CPU::op<Opcode::STA>(AddressingMode, Address addr) {
    this->memory->Write(addr, this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STP
template<>
uint8_t CPU::op<Opcode::STP>(AddressingMode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STX
template<>
uint8_t CPU::op<Opcode::STX>(AddressingMode, Address addr) {
    this->memory->Write(addr, this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STY
template<>
uint8_t CPU::op<Opcode::STY>(AddressingMode, Address addr) {
    this->memory->Write(addr, this->regY);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAS
template<>
uint8_t CPU::op<Opcode::TAS>(AddressingMode mode, Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAX
template<>
uint8_t CPU::op<Opcode::TAX>(AddressingMode mode, Address addr) {
    this->regX = this->regA;
    this->setNZ(this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAY
template<>
uint8_t CPU::op<Opcode::TAY>(AddressingMode mode, Address addr) {
    this->regY = this->regA;
    this->setNZ(this->regY);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TSX
template<>
uint8_t CPU::op<Opcode::TSX>(AddressingMode mode, Address addr) {
    this->regX = this->regSP;
    this->setNZ(this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXA
template<>
uint8_t CPU::op<Opcode::TXA>(AddressingMode mode, Address addr) {
    this->regA = this->regX;
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXS
template<>
uint8_t CPU::op<Opcode::TXS>(AddressingMode mode, Address addr) {
    this->regSP = this->regX;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TYA
template<>
uint8_t CPU::op<Opcode::TYA>(AddressingMode mode, Address addr) {
    this->regA = this->regY;
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#XAA
template<>
uint8_t CPU::op<Opcode::XAA>(AddressingMode, Address) {
    return 0;
}


// https://www.nesdev.org/obelisk-6502-guide/reference.html# ISB
template<>
uint8_t CPU::op<Opcode::ISB>(AddressingMode mode, Address addr) {
    uint8_t numCycles = this->op<Opcode::INC>(mode, addr);
    numCycles += this->op<Opcode::SBC>(mode, addr);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RLA
template<>
uint8_t CPU::op<Opcode::RLA>(AddressingMode mode, Address addr) {
    auto numCycles = this->op<Opcode::ROL>(mode, addr);
    numCycles += this->op<Opcode::AND>(mode, addr);
    return numCycles;
}


uint8_t CPU::dispatch(const DecodedInstruction &decodedInstruction, Address addr) {
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

void CPU::setNZ(Word data) {
    this->status[Flag::N] = (data & 0x80) != 0;
    this->status[Flag::Z] = data == 0;
}

void CPU::setCNZ(WordWithCarry data) {
    this->status[Flag::C] = (data & 0x0100) != 0;
    this->status[Flag::N] = (data & 0x80) != 0;
    this->status[Flag::Z] = (data & 0xff) == 0;
}


void CPU::push(Word data) {
    this->memory->Write(0x100 + Address(this->regSP), data);
    this->regSP--;
}

void CPU::pushAddress(Address addr) {
    Word hi = addr >> 8;
    Word lo = addr & 0xff;
    this->push(hi);
    this->push(lo);
}

Word CPU::pop() {
    this->regSP++;
    auto data = this->read(0x100 + this->regSP);
    return data;
}

Address CPU::popAddress() {
    Address lo = this->pop();
    Address hi = this->pop();
    return hi << 8 | lo;
}

Word CPU::read(Address addr) const {
    return this->memory->Read(addr);
}

void CPU::write(Address addr, Word data) {
    return this->memory->Write(addr, data);
}

Address CPU::readAddress(Address addr) const {
    auto lowBits = this->read(addr);
    auto highBits = this->read(addr + 1);

    return highBits << 8 | lowBits;
}

Address CPU::readAddressBug(Address addr) const {
    // there was a bug in the original if the high byte crosses a page boundary,
    // it instead will wrap around
    auto loAddr = addr;
    auto hiAddr = (loAddr & 0xff00) | 0x00ff & (loAddr + 1);

    auto lowBits = this->read(loAddr);
    auto highBits = this->read(hiAddr);

    return highBits << 8 | lowBits;
}

void CPU::PC(Address addr) {
    this->pc = addr;
}

CPU::CPU(std::unique_ptr<Memory> &&m) :
    memory(std::move(m)),
    pc(0),
    regA(0),
    regX(0),
    regY(0),
    regSP(0xfd),
    status(0x0),
    cycle(0) {
    // https://www.nesdev.org/wiki/CPU_ALL#At_power-up

    this->status[Flag::U] = true;
    this->status[Flag::I] = true;
    //    this->status[Flag::B] = true;

    this->pc = this->readAddress(0xfffc);

    this->write(0x4017, 0x40); // disable frame IRQ
    this->write(0x4015, 0x40); // disable all channels

    for (Address addr = 0x4000; addr <= 0x4013; addr++)
        this->write(addr, 0x0);
}
} // namespace nes