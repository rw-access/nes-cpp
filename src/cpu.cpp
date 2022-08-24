#include "nes.h"

namespace nes::cpu {

enum Flags : uint8_t {
    C = 0, // Carry Flag          Set if overflow in bit 7
    Z = 1, // Zero Flag           Set if A = 0
    I = 2, // Interrupt Disable   Not affected
    D = 3, // Decimal Mode Flag   Not affected
    B = 4, // Break Command       Not affected
    U = 5, // Unused flag
    V = 6, // Overflow Flag       Set if sign bit is incorrect
    N = 7, // Negative Flag       Set if bit 7 set
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

static bool crossesPageBoundary(uint16_t addrA, uint16_t addrB) {
    return (addrA & 0xff00) != (addrB & 0xff00);
}


uint8_t CPU::step() {
    auto instruction = this->memory.Read(this->pc);
    auto decodedInstruction = decodeTable[instruction];

    mem::Address address = 0;
    this->pc++;
    uint8_t numCycles = 1;

    switch (decodedInstruction.addressingMode) {
        case AddressingMode::Implied:
        case AddressingMode::Accumulator:
            // read + alu cycles tracked in instruction
            break;
        case AddressingMode::Absolute:
            address = this->memory.Read16(this->pc);
            this->pc += 2;
            numCycles += 2; // read + read
            break;
        case AddressingMode::Immediate:
            address = this->pc;
            this->pc++;
            numCycles += 0; // address already known
            break;
        case AddressingMode::AbsoluteIndexedX:
            address = this->memory.Read16(this->pc) + (uint16_t) this->regX;
            this->pc += 2;
            numCycles += 3; // read + read + add
            break;
        case AddressingMode::AbsoluteIndexedY:
            address = this->memory.Read16(this->pc) + (uint16_t) this->regY;
            this->pc += 2;
            numCycles += 3; // read + read + add
            break;
        case AddressingMode::IndexedIndirect:
            address = (uint16_t) this->memory.Read(this->pc) + (uint16_t) this->regX;
            this->pc += 2;
            numCycles += 4; // read + add
            break;
        case AddressingMode::Indirect:
            address = this->memory.Read16(this->pc);
            this->pc += 2;
            numCycles += 2;
            break;
        case AddressingMode::IndirectIndexed:
            address = this->memory.Read16(this->memory.Read(this->pc)) + (uint16_t) this->regY;
            this->pc += 2;
            numCycles += 3; // read + read + read + add
            break;
        case AddressingMode::Relative:
            // TODO: check that this handles negative offsets correctly
            {
                auto offset = this->memory.Read(this->pc);
                this->pc++;
                numCycles++;

                if (offset & 0x80)
                    address = this->pc - (uint16_t) (0x100 - offset);
                else
                    address = this->pc + (uint16_t) offset;
            }
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

    // add this point, num cycles doesn't include any Read operations,
    // and it only contains time for address calculations
    numCycles += this->dispatch(decodedInstruction, address);

    // every instruction should be at least two cycles
    numCycles += numCycles < 2;
    this->cycle += numCycles;
    return numCycles;
}
//
//std::pair<uint8_t, std::string> CPU::debugStep() {
//    auto numCycles = this->step();
//
//    auto instruction = this->memory.Read(this->pc);
//    auto decodedInstruction = decodeTable[instruction];
//}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ADC
template<>
uint8_t CPU::op<Opcode::ADC>(AddressingMode mode, mem::Address addr) {
    auto a = uint16_t(this->regA);
    auto b = uint16_t(this->memory.Read(addr));
    auto result = a + b;
    this->regA = uint8_t(result);

    this->setCNZ(result);

    // signs are the same before, but the sign changed after
    this->status[Flags::V] = (a & 0x80) == (b & 0x80) && (a & 0x80) != (result & 0x80);

    // read (no ALU hit apparently)
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AHX
template<>
uint8_t CPU::op<Opcode::AHX>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ALR
template<>
uint8_t CPU::op<Opcode::ALR>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ANC
template<>
uint8_t CPU::op<Opcode::ANC>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AND
template<>
uint8_t CPU::op<Opcode::AND>(AddressingMode mode, mem::Address addr) {
    this->regA &= this->memory.Read(addr);
    this->setNZ(this->regA);
    // read + ALU
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ARR
template<>
uint8_t CPU::op<Opcode::ARR>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ASL
template<>
uint8_t CPU::op<Opcode::ASL>(AddressingMode mode, mem::Address addr) {
    uint16_t resultWide = 0;
    uint8_t numCycles = 2;

    if (mode == AddressingMode::Accumulator) {
        resultWide = uint16_t(this->regA) << 1;
        this->regA = uint8_t(resultWide);
    } else {
        resultWide = uint16_t(this->memory.Read(addr));
        this->regA = uint8_t(resultWide);
        numCycles += 2;
    }

    this->setNZ(uint8_t(resultWide));
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#AXS
template<>
uint8_t CPU::op<Opcode::AXS>(AddressingMode mode, mem::Address addr) {
    return 0;
}


uint8_t CPU::BXX(uint8_t flag, bool isSet, mem::Address addr) {
    uint8_t numCycles = 0;

    if (this->status[flag] == isSet) {
        numCycles = 1 + crossesPageBoundary(this->pc, addr);
        this->pc = addr;
    }

    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCC
template<>
uint8_t CPU::op<Opcode::BCC>(AddressingMode, mem::Address addr) {
    return this->BXX(Flags::C, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BCS
template<>
uint8_t CPU::op<Opcode::BCS>(AddressingMode, mem::Address addr) {
    return this->BXX(Flags::C, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BEQ
template<>
uint8_t CPU::op<Opcode::BEQ>(AddressingMode, mem::Address addr) {
    return this->BXX(Flags::Z, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BIT
template<>
uint8_t CPU::op<Opcode::BIT>(AddressingMode, mem::Address addr) {
    auto result = this->regA & this->memory.Read(addr);
    this->status[Flags::Z] = (result == 0);
    this->status[Flags::N] = (result & 0x80) != 0;
    this->status[Flags::V] = (result & 0x40) != 0;
    // read + ALU
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BMI
template<>
uint8_t CPU::op<Opcode::BMI>(AddressingMode, mem::Address addr) {
    return this->BXX(Flags::N, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BNE
template<>
uint8_t CPU::op<Opcode::BNE>(AddressingMode, mem::Address addr) {
    return this->BXX(Flags::Z, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BPL
template<>
uint8_t CPU::op<Opcode::BPL>(AddressingMode, mem::Address addr) {
    return this->BXX(Flags::N, false, addr);
}

template<>
uint8_t CPU::op<Opcode::BRK>(AddressingMode, mem::Address) {
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

    this->status[Flags::B] = true;

    this->push16(this->pc);
    this->push(uint8_t(this->status.to_ulong()));

    this->memory.Read16(0xfffe);

    return 7;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVC
template<>
uint8_t CPU::op<Opcode::BVC>(AddressingMode mode, mem::Address addr) {
    return this->BXX(Flags::V, false, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#BVS
template<>
uint8_t CPU::op<Opcode::BVS>(AddressingMode mode, mem::Address addr) {
    return this->BXX(Flags::V, true, addr);
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLC
template<>
uint8_t CPU::op<Opcode::CLC>(AddressingMode mode, mem::Address addr) {
    this->status[Flags::C] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLD
template<>
uint8_t CPU::op<Opcode::CLD>(AddressingMode mode, mem::Address addr) {
    this->status[Flags::D] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLI
template<>
uint8_t CPU::op<Opcode::CLI>(AddressingMode mode, mem::Address addr) {
    this->status[Flags::I] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CLV
template<>
uint8_t CPU::op<Opcode::CLV>(AddressingMode mode, mem::Address addr) {
    this->status[Flags::V] = false;
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CMP
template<>
uint8_t CPU::op<Opcode::CMP>(AddressingMode mode, mem::Address addr) {
    auto a = this->regA;
    auto m = this->memory.Read(addr);
    auto data = a - m;
    this->setNZ(data);
    this->status[Flags::C] = a > m;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPX
template<>
uint8_t CPU::op<Opcode::CPX>(AddressingMode mode, mem::Address addr) {
    auto x = this->regX;
    auto m = this->memory.Read(addr);
    this->setNZ(x - m);
    this->status[Flags::C] = x >= m;
    // read
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#CPY
template<>
uint8_t CPU::op<Opcode::CPY>(AddressingMode mode, mem::Address addr) {
    auto y = this->regY;
    auto m = this->memory.Read(addr);
    auto data = y - m;
    this->setNZ(data);
    this->status[Flags::C] = y > m;
    // read
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DCP
template<>
uint8_t CPU::op<Opcode::DCP>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEC
template<>
uint8_t CPU::op<Opcode::DEC>(AddressingMode, mem::Address addr) {
    auto m = this->memory.Read(addr) - 1;
    this->memory.Write(addr, m);
    this->setNZ(m);
    // read + alu + write
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEX
template<>
uint8_t CPU::op<Opcode::DEX>(AddressingMode, mem::Address) {
    this->regX--;
    this->setNZ(this->regX);
    // ALU
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#DEY
template<>
uint8_t CPU::op<Opcode::DEY>(AddressingMode mode, mem::Address addr) {
    this->regY--;
    this->setNZ(this->regY);
    // ALU
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#EOR
template<>
uint8_t CPU::op<Opcode::EOR>(AddressingMode, mem::Address addr) {
    this->regA ^= this->memory.Read(addr);
    this->setNZ(addr);
    // read
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INC
template<>
uint8_t CPU::op<Opcode::INC>(AddressingMode, mem::Address addr) {
    auto data = this->memory.Read(addr) - 1;
    this->memory.Write(addr, data);
    this->setNZ(data);
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INX
template<>
uint8_t CPU::op<Opcode::INX>(AddressingMode, mem::Address) {
    this->regX++;
    this->setNZ(this->regX);
    // 2 cycle minimum, already counted one
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#INY
template<>
uint8_t CPU::op<Opcode::INY>(AddressingMode, mem::Address) {
    this->regY++;
    this->setNZ(this->regY);
    // 2 cycle minimum, already counted one
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ISC
template<>
uint8_t CPU::op<Opcode::ISC>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
template<>
uint8_t CPU::op<Opcode::JMP>(AddressingMode, mem::Address addr) {
    this->pc = addr;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#JSR
template<>
uint8_t CPU::op<Opcode::JSR>(AddressingMode, mem::Address addr) {
    this->push16(this->pc);
    this->pc = addr;
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LAS
template<>
uint8_t CPU::op<Opcode::LAS>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LAX
template<>
uint8_t CPU::op<Opcode::LAX>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDA
template<>
uint8_t CPU::op<Opcode::LDA>(AddressingMode mode, mem::Address addr) {
    this->regA = this->memory.Read(addr);
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDX
template<>
uint8_t CPU::op<Opcode::LDX>(AddressingMode mode, mem::Address addr) {
    this->regX = this->memory.Read(addr);
    this->setNZ(this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LDY
template<>
uint8_t CPU::op<Opcode::LDY>(AddressingMode mode, mem::Address addr) {
    this->regY = this->memory.Read(addr);
    this->setNZ(this->regY);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#LSR
template<>
uint8_t CPU::op<Opcode::LSR>(AddressingMode mode, mem::Address addr) {
    uint16_t wideData;
    uint8_t numCycles = 0;

    if (mode == AddressingMode::Accumulator) {
        wideData = uint16_t(this->regA);
        wideData = wideData >> 1 | (wideData & 0x01) << 8;
        this->regA = uint8_t(wideData);
    } else {
        wideData = uint16_t(this->memory.Read(addr));
        wideData = wideData >> 1 | (wideData & 0x01) << 8;
        this->memory.Write(addr, uint8_t(wideData));
        numCycles += 2;
    }

    // ALU
    numCycles++;
    this->setCNZ(wideData);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#NOP
template<>
uint8_t CPU::op<Opcode::NOP>(AddressingMode, mem::Address) {
    // two cycle minimum
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ORA
template<>
uint8_t CPU::op<Opcode::ORA>(AddressingMode, mem::Address addr) {
    this->regA |= this->memory.Read(addr);
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHA
template<>
uint8_t CPU::op<Opcode::PHA>(AddressingMode, mem::Address) {
    this->push(this->regA);
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PHP
template<>
uint8_t CPU::op<Opcode::PHP>(AddressingMode, mem::Address) {
    this->push(this->status.to_ulong());
    return 2;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLA
template<>
uint8_t CPU::op<Opcode::PLA>(AddressingMode, mem::Address) {
    this->regA = this->pop();
    this->setNZ(regA);
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#PLP
template<>
uint8_t CPU::op<Opcode::PLP>(AddressingMode, mem::Address) {
    this->status = this->pop();
    this->setNZ(regA);
    return 3;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RLA
template<>
uint8_t CPU::op<Opcode::RLA>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROL
template<>
uint8_t CPU::op<Opcode::ROL>(AddressingMode mode, mem::Address addr) {
    uint16_t wideData;
    uint8_t numCycles = 0;

    if (mode == AddressingMode::Accumulator) {
        wideData = uint16_t(this->regA);
        wideData = wideData << 1 | this->status[Flags::C];
        this->regA = wideData;
    } else {
        wideData = uint16_t(this->memory.Read(addr));
        wideData = wideData << 1 | this->status[Flags::C];
        this->memory.Write(addr, uint8_t(wideData));
        numCycles += 2;
    }

    // ALU
    numCycles++;
    this->setCNZ(wideData);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#ROR
template<>
uint8_t CPU::op<Opcode::ROR>(AddressingMode mode, mem::Address addr) {
    uint16_t wideData;
    uint8_t numCycles = 0;

    if (mode == AddressingMode::Accumulator) {
        wideData = uint16_t(this->regA);
        wideData = wideData >> 1 | (this->status[Flags::C] << 8);
        this->regA = wideData;
    } else {
        wideData = uint16_t(this->memory.Read(addr));
        wideData = wideData >> 1 | (this->status[Flags::C] << 8);
        this->memory.Write(addr, uint8_t(wideData));
        numCycles += 2;
    }

    // ALU
    numCycles++;
    this->setCNZ(wideData);
    return numCycles;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RRA
template<>
uint8_t CPU::op<Opcode::RRA>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTI
template<>
uint8_t CPU::op<Opcode::RTI>(AddressingMode, mem::Address) {
    this->status = this->pop();
    this->pc = this->pop16();
    return 5;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#RTS
template<>
uint8_t CPU::op<Opcode::RTS>(AddressingMode, mem::Address) {
    this->pc = this->pop16();
    this->pc++;
    return 5;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SAX
template<>
uint8_t CPU::op<Opcode::SAX>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SBC
template<>
uint8_t CPU::op<Opcode::SBC>(AddressingMode mode, mem::Address addr) {
    auto a = uint16_t(this->regA);
    auto m = uint16_t(this->memory.Read(addr));
    auto result = a - m - (1 - this->status[Flags::C]);
    this->regA = uint8_t(result);

    this->setCNZ(result);

    // signs are the same before, but the sign changed after
    this->status[Flags::V] = (a & 0x80) == (m & 0x80) && (a & 0x80) != (result & 0x80);

    // read (no ALU hit apparently)
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEC
template<>
uint8_t CPU::op<Opcode::SEC>(AddressingMode mode, mem::Address addr) {
    this->status[Flags::C] = 1;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SED
template<>
uint8_t CPU::op<Opcode::SED>(AddressingMode, mem::Address) {
    this->status[Flags::D] = 1;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SEI
template<>
uint8_t CPU::op<Opcode::SEI>(AddressingMode, mem::Address) {
    this->status[Flags::I] = 1;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SHX
template<>
uint8_t CPU::op<Opcode::SHX>(AddressingMode, mem::Address) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SHY
template<>
uint8_t CPU::op<Opcode::SHY>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SLO
template<>
uint8_t CPU::op<Opcode::SLO>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#SRE
template<>
uint8_t CPU::op<Opcode::SRE>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STA
template<>
uint8_t CPU::op<Opcode::STA>(AddressingMode, mem::Address addr) {
    this->memory.Write(addr, this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STP
template<>
uint8_t CPU::op<Opcode::STP>(AddressingMode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STX
template<>
uint8_t CPU::op<Opcode::STX>(AddressingMode, mem::Address addr) {
    this->memory.Write(addr, this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#STY
template<>
uint8_t CPU::op<Opcode::STY>(AddressingMode, mem::Address addr) {
    this->memory.Write(addr, this->regY);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAS
template<>
uint8_t CPU::op<Opcode::TAS>(AddressingMode mode, mem::Address addr) {
    return 0;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAX
template<>
uint8_t CPU::op<Opcode::TAX>(AddressingMode mode, mem::Address addr) {
    this->regX = this->regA;
    this->setNZ(this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TAY
template<>
uint8_t CPU::op<Opcode::TAY>(AddressingMode mode, mem::Address addr) {
    this->regY = this->regA;
    this->setNZ(this->regY);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TSX
template<>
uint8_t CPU::op<Opcode::TSX>(AddressingMode mode, mem::Address addr) {
    this->regX = this->regSP;
    this->setNZ(this->regX);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXA
template<>
uint8_t CPU::op<Opcode::TXA>(AddressingMode mode, mem::Address addr) {
    this->regA = this->regX;
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TXS
template<>
uint8_t CPU::op<Opcode::TXS>(AddressingMode mode, mem::Address addr) {
    this->regSP = this->regX;
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#TYA
template<>
uint8_t CPU::op<Opcode::TYA>(AddressingMode mode, mem::Address addr) {
    this->regA = this->regY;
    this->setNZ(this->regA);
    return 1;
}

// https://www.nesdev.org/obelisk-6502-guide/reference.html#XAA
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

void CPU::setNZ(uint8_t data) {
    this->status[Flags::N] = (data & 0x80) != 0;
    this->status[Flags::Z] = data == 0;
}

void CPU::setCNZ(uint16_t data) {
    this->status[Flags::C] = (data & 0x0100) != 0;
    this->status[Flags::N] = (data & 0x80) != 0;
    this->status[Flags::Z] = data == 0;
}


void CPU::push(uint8_t data) {
    this->memory.Write(0x100 + uint16_t(this->regSP), data);
    this->regSP--;
}

void CPU::push16(uint16_t data) {
    this->push(uint8_t(data >> 8));
    this->push(uint8_t(data & 0x80));
}

uint8_t CPU::pop() {
    this->regSP++;
    auto data = this->memory.Read(0x100 + this->regSP);
    return data;
}

uint16_t CPU::pop16() {
    uint16_t lo = this->pop();
    uint16_t hi = this->pop();
    return hi << 8 | lo;
}


CPU::CPU(mem::Memory &m) :
    memory(m),
    pc(0),
    regA(0),
    regX(0),
    regY(0),
    regSP(0xfd),
    status(0x0),
    cycle(0) {
    // https://www.nesdev.org/wiki/CPU_ALL#At_power-up

    this->status[Flags::U] = true;
    this->status[Flags::I] = true;
    this->status[Flags::B] = true;

    this->pc = this->memory.Read16(0xfffc);

    this->memory.Write(0x4017, 0x40); // disable frame IRQ
    this->memory.Write(0x4015, 0x40); // disable all channels

    for (mem::Address addr = 0x4000; addr <= 0x4013; addr++)
        this->memory.Write(addr, 0x0);
}
} // namespace nes::cpu