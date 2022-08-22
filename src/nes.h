#pragma once
#include <array>
#include <cinttypes>
#include <cstddef>
#include <vector>


namespace nes {
namespace mem {

typedef uint16_t Address;

class Memory {
private:
    std::vector<uint8_t> rawMemory;

public:
    Memory();

    uint8_t Read(Address address) const;
    uint16_t Read16(Address lowAddress) const;
};
} // namespace mem

namespace cpu {

enum class AddressingMode : uint8_t {
    Implied,
    Accumulator, // A

    Absolute,         // $xxxx       a
    AbsoluteIndexedX, // $xxxx,Y     a,x ?? $xx,PC
    AbsoluteIndexedY, // $xxxx,Y     a,y $xx,PC
    Immediate,        // #$xx:       #i
    IndexedIndirect,  // ($xx,X)     (d,x) $xx
    Indirect,         // ($xxxx)     (a)
    IndirectIndexed,  // ($xx),Y     (d),y $xx,X
    Relative,         // $xx,PC      *+d
    ZeroPage,         // $xx         d
    ZeroPageIndexedX, // $xx,X       d,x
    ZeroPageIndexedY, // $xx,X       d,y
};

enum class Opcode : uint8_t {
    // clang-format off
    ADC, AHX, ALR, ANC, AND, ARR, ASL, AXS,
    BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK,
    BVC, BVS, CLC, CLD, CLI, CLV, CMP, CPX,
    CPY, DCP, DEC, DEX, DEY, EOR, INC, INX,
    INY, ISC, JMP, JSR, LAS, LAX, LDA, LDX,
    LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP,
    RLA, ROL, ROR, RRA, RTI, RTS, SAX, SBC,
    SEC, SED, SEI, SHX, SHY, SLO, SRE, STA,
    STP, STX, STY, TAS, TAX, TAY, TSX, TXA,
    TXS, TYA, XAA,
    // clang-format on
};

struct DecodedInstruction {
    Opcode opcode;
    AddressingMode addressingMode;
};

class CPU {
private:
    mem::Memory &memory;
    uint8_t regA, regX, regY, regS, regSP;
    mem::Address pc;

public:
    CPU(mem::Memory &memory);

    // Execute a single instruction and return the number of cycles it took
    uint8_t step();

    uint8_t dispatch(const DecodedInstruction &decodedInstruction, mem::Address addr);

    // one templated forward declaration for all opcodes
    template<Opcode>
    uint8_t op(AddressingMode mode, mem::Address addr);
};
} // namespace cpu
} // namespace nes