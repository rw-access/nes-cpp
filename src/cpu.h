#pragma once
#include "cartridge.h"
#include "console.h"
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

enum class Interrupt : uint8_t {
    None = 0,
    IRQ  = 1,
    NMI  = 2,
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
    Opcode opcode                 : 7;
    AddressingMode addressingMode : 4;
    uint8_t MinCycles             : 5;
    bool PageBoundaryHit          : 1;
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
    using Status        = std::bitset<8>;
    using WordWithCarry = uint16_t;

private:
    Console &console;
    std::array<Byte, 0x800> ram = {0};
    Byte regA, regX, regY, regSP;
    Address pc;
    Status status;
    uint64_t cycle;
    Interrupt pendingInterrupt = Interrupt::None;

    // TODO: consider make an opcode-based jump table of 256 entries for more specialized code,
    //   that can optimize inlining of functions because the addressing node is known at compile time
    uint8_t dispatch(const DecodedInstruction &decodedInstruction, Address addr);

    inline void setNZ(Byte data);
    inline void setCNZ(WordWithCarry data);

    template<Opcode>
    inline Byte op(AddressingMode mode, Address addr);

#define OP_MACRO(opcode)                                                                                               \
    template<>                                                                                                         \
    Byte op<Opcode::opcode>(AddressingMode mode, Address addr);
    FOREACH_OPCODE(OP_MACRO)
#undef OP_MACRO

    // general purpose branch instruction
    inline Byte BXX(Flag flag, bool isSet, Address addr);

    inline void push(Byte data);
    inline void pushAddress(Address addr);

    inline Byte pop();
    inline Address popAddress();

    Byte read(Address addr) const;
    Address readAddress(Address addr) const;
    Address readAddressIndirectWraparound(Address addr) const;

    void write(Address addr, Byte data);

public:
    CPU(Console &c);

    // Execute a single instruction and return the number of cycles it took
    uint8_t step();
    void reset();

    void interrupt(Interrupt interrupt);

    void PC(Address addr);
    uint8_t handleInterrupt();
};

} // namespace nes