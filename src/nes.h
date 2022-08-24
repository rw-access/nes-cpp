#pragma once
#include <array>
#include <bitset>
#include <cinttypes>
#include <cstddef>
#include <vector>


namespace nes {

typedef uint16_t Address;

class BaseMemory {
public:
    uint16_t Read16(Address lowAddress) const;

    virtual uint8_t Read(Address address) const;
    virtual void Write(Address address, uint8_t data);
};

class Cartridge : public BaseMemory {
private:
    // std::vector<uint8_t> prgRom;
    // std::vector<uint8_t> chrRom;
    // std::vector<uint8_t> instRom;
    // std::vector<uint8_t> prgRam;

public:
    virtual ~Cartridge() = 0;
};

class Memory final : public BaseMemory {
private:
    std::array<uint8_t, 0x800> ram;
    std::array<uint8_t, 0x008> ppuReg;
    std::array<uint8_t, 0x0018> apu;
    std::array<uint8_t, 0x008> io;

    std::unique_ptr<Cartridge> cartridge;

public:
    Memory(std::unique_ptr<Cartridge> &&cartridge);

    uint8_t Read(Address address) const override;
    void Write(Address address, uint8_t data) override;
};

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
    Memory &memory;
    uint8_t regA, regX, regY, regSP;
    Address pc;
    std::bitset<8> status;
    uint64_t cycle;

    // TODO: consider templating by addressing mode. it lets the compiler inline branches base on mode.
    //  but, it could lead to bloat and # modes jump tables of # opcodes (975 cases, instead of 256).
    //  instead, it would be better to make an opcode-based jump table of 256 entries
    uint8_t dispatch(const DecodedInstruction &decodedInstruction, Address addr);

    inline void setNZ(uint8_t data);
    inline void setCNZ(uint16_t data);

    // one templated forward declaration for all opcodes
    template<Opcode>
    inline uint8_t op(AddressingMode mode, Address addr);

    // general purpose branch instruction
    inline uint8_t BXX(uint8_t flag, bool isSet, Address addr);

    inline void push(uint8_t data);
    inline void push16(uint16_t data);

    inline uint8_t pop();
    inline uint16_t pop16();

public:
    CPU(Memory &memory);

    // Execute a single instruction and return the number of cycles it took
    uint8_t step();

    // Single step
    std::pair<uint8_t, std::string> debugStep();
};
} // namespace nes