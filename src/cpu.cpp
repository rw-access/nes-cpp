#include "nes.h"

namespace nes::cpu {
/*
    d,x	Zero page indexed	val = PEEK((arg + X) % 256)	4
    d,y	Zero page indexed	val = PEEK((arg + Y) % 256)	4
    a,x	Absolute indexed	val = PEEK(arg + X)	4+
    a,y	Absolute indexed	val = PEEK(arg + Y)	4+
    (d,x)	Indexed indirect	val = PEEK(PEEK((arg + X) % 256) + PEEK((arg + X + 1) % 256) * 256)	6
    (d),y Indirect indexed	val = PEEK(PEEK(arg) + PEEK((arg + 1) % 256) * 256 + Y)	5+

IMD     #$xx
REL     $xx,PC
0PG     $xx
0PX     $xx,X
0PY     $xx,Y
ABS     $xxxx
ABX     $xxxx,X
ABY     $xxxx,Y
IND     ($xxxx)
NDX     ($xx,X)
NDY     ($xx),Y

 */


enum class Register : uint8_t {
    Accumulator = 0,
    IndexX = 1,
    IndexY = 2,
    Status = 3,
    StackPointer = 4,
};


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
        {Opcode::ISC, AddressingMode::IndexedIndirect},
        {Opcode::CPX, AddressingMode::ZeroPage},
        {Opcode::SBC, AddressingMode::ZeroPage},
        {Opcode::INC, AddressingMode::ZeroPage},
        {Opcode::ISC, AddressingMode::ZeroPage},
        {Opcode::INX, AddressingMode::Implied},
        {Opcode::SBC, AddressingMode::Immediate},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::SBC, AddressingMode::Immediate},
        {Opcode::CPX, AddressingMode::Absolute},
        {Opcode::SBC, AddressingMode::Absolute},
        {Opcode::INC, AddressingMode::Absolute},
        {Opcode::ISC, AddressingMode::Absolute},
        {Opcode::BEQ, AddressingMode::Relative},
        {Opcode::SBC, AddressingMode::IndirectIndexed},
        {Opcode::STP, AddressingMode::Implied},
        {Opcode::ISC, AddressingMode::IndirectIndexed},
        {Opcode::NOP, AddressingMode::ZeroPageIndexedX},
        {Opcode::SBC, AddressingMode::ZeroPageIndexedX},
        {Opcode::INC, AddressingMode::ZeroPageIndexedX},
        {Opcode::ISC, AddressingMode::ZeroPageIndexedX},
        {Opcode::SED, AddressingMode::Implied},
        {Opcode::SBC, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::Implied},
        {Opcode::ISC, AddressingMode::AbsoluteIndexedY},
        {Opcode::NOP, AddressingMode::AbsoluteIndexedX},
        {Opcode::SBC, AddressingMode::AbsoluteIndexedX},
        {Opcode::INC, AddressingMode::AbsoluteIndexedX},
        {Opcode::ISC, AddressingMode::AbsoluteIndexedX},
};
// clang-format on

CPU::CPU(mem::Memory &m) :
    memory(m),
    pc(0),
    regA(0),
    regX(0),
    regY(0),
    regS(0),
    regSP(0) {
}

static bool crossesPageBoundary(uint16_t addrA, uint16_t addrB) {
    return (addrA & 0xff00) != (addrB & 0xff00);
}

uint8_t CPU::step() {
    uint8_t numCycles = 1;
    auto instruction = this->memory.Read(this->pc);
    auto decodedInstruction = decodeTable[instruction];

    mem::Address address = 0;
    this->pc++;

    switch (decodedInstruction.addressingMode) {
        case AddressingMode::Implied:
        case AddressingMode::Accumulator:
            break;
        case AddressingMode::Absolute:
            address = this->memory.Read16(this->pc);
            this->pc += 2;
            numCycles += 2;
            break;
        case AddressingMode::Immediate:
            address = this->pc;
            this->pc++;
            numCycles++;
            break;
        case AddressingMode::AbsoluteIndexedX:
            address = this->memory.Read16(this->pc) + (uint16_t) this->regX;
            this->pc += 2;
            numCycles += 2;
            break;
        case AddressingMode::AbsoluteIndexedY:
            address = this->memory.Read16(this->pc) + (uint16_t) this->regY;
            this->pc += 2;
            numCycles += 2;
            break;
        case AddressingMode::IndexedIndirect:
            address = this->memory.Read16((uint16_t) this->memory.Read(this->pc) + (uint16_t) this->regX);
            this->pc += 2;
            numCycles += 2;
            break;
        case AddressingMode::Indirect:
            address = this->memory.Read16(this->pc);
            this->pc += 2;
            numCycles += 2;
            break;
        case AddressingMode::IndirectIndexed:
            address = this->memory.Read16(this->memory.Read(this->pc)) + (uint16_t) this->regY;
            this->pc += 2;
            numCycles += 3;
            break;
        case AddressingMode::Relative:
            address = this->pc + (uint16_t) this->memory.Read(this->pc);
            this->pc++;
            numCycles++;
            break;
        case AddressingMode::ZeroPage:
            address = (uint16_t) this->memory.Read(this->pc);
            this->pc++;
            numCycles++;
            break;
        case AddressingMode::ZeroPageIndexedX:
            address = (uint16_t) this->regX;
            numCycles++;
            this->pc++;
            break;
        case AddressingMode::ZeroPageIndexedY:
            address = (uint16_t) this->regY;
            numCycles++;
            this->pc++;
            break;
    };

    numCycles += this->dispatch(decodedInstruction, address);
    return numCycles;
}

template<>
uint8_t CPU::op<Opcode::ADC>(AddressingMode mode, mem::Address addr) {
    return 0;
}

template<>
uint8_t CPU::op<Opcode::AHX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ALR>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ANC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::AND>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ARR>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ASL>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::AXS>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BCC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BCS>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BEQ>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BIT>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BMI>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BNE>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BPL>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BRK>(AddressingMode mode, mem::Address addr) {
    return 1;
}
template<>
uint8_t CPU::op<Opcode::BVC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::BVS>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::CLC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::CLD>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::CLI>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::CLV>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::CMP>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::CPX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::CPY>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::DCP>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::DEC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::DEX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::DEY>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::EOR>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::INC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::INX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::INY>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ISC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::JMP>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::JSR>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::LAS>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::LAX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::LDA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::LDX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::LDY>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::LSR>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::NOP>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ORA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::PHA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::PHP>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::PLA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::PLP>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::RLA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ROL>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::ROR>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::RRA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::RTI>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::RTS>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SAX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SBC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SEC>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SED>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SEI>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SHX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SHY>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SLO>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::SRE>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::STA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::STP>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::STX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::STY>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::TAS>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::TAX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::TAY>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::TSX>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::TXA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::TXS>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::TYA>(AddressingMode mode, mem::Address addr) {
    return 0;
}
template<>
uint8_t CPU::op<Opcode::XAA>(AddressingMode mode, mem::Address addr) {
    return 0;
}


uint8_t CPU::dispatch(const DecodedInstruction &decodedInstruction, mem::Address addr) {
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
        case Opcode::ISC:
            return this->op<Opcode::ISC>(decodedInstruction.addressingMode, addr);
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


} // namespace nes::cpu