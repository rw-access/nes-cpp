#pragma once
#include "memory.h"
#include "nes.h"
#include "opcodes.def"

namespace nes {

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
#define OP_MACRO(op) op,
    FOREACH_OPCODE(OP_MACRO)
#undef OP_MACRO
};

static const char *OpcodeStrings[256]{
#define OP_MACRO(op) #op,
        FOREACH_OPCODE(OP_MACRO)
#undef OP_MACRO
};

struct DecodedInstruction {
    Opcode opcode;
    AddressingMode addressingMode;
};


enum Flag : uint8_t {
    C = 0, // Carry Flag
    Z = 1, // Zero Flag
    I = 2, // Interrupt Disable
    D = 3, // Decimal Mode Flag
    B = 4, // Break Command
    U = 5, // Unused flag
    V = 6, // Overflow Flag
    N = 7, // Negative Flag
};

class CPU {
    using Status = std::bitset<8>;
    using WordWithCarry = uint16_t;
    using SignedWord = int8_t;
    using SignedWordWithCarry = int16_t;

private:
    std::unique_ptr<Memory> memory;
    Word regA, regX, regY, regSP;
    Address pc;
    Status status;
    uint64_t cycle;

    // TODO: consider make an opcode-based jump table of 256 entries for more specialized code,
    //   that can optimize inlining of functions because the addressing node is known at compile time
    uint8_t dispatch(const DecodedInstruction &decodedInstruction, Address addr);

    inline void setNZ(Word data);
    inline void setCNZ(WordWithCarry data);

    // one templated forward declaration for all opcodes
    template<Opcode>
    inline Word op(AddressingMode mode, Address addr);

    // general purpose branch instruction
    inline Word BXX(Flag flag, bool isSet, Address addr);

    inline void push(Word data);
    inline void pushAddress(Address addr);

    inline Word pop();
    inline Address popAddress();

    Word read(Address addr) const;
    Address readAddress(Address addr) const;
    Address readAddressBug(Address addr) const;

    void write(Address addr, Word data);

public:
    CPU(std::unique_ptr<Memory> &&memory);

    // Execute a single instruction and return the number of cycles it took
    uint8_t step();

    uint8_t debugStep();

    void PC(Address addr);
};

} // namespace nes