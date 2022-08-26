#pragma once
#include "cartridge.h"
#include "nes.h"

namespace nes {


class PPU;
class CPU;

class Console : std::enable_shared_from_this<Console> {
    friend PPU;
    friend CPU;
    friend std::unique_ptr<Console>;

#ifdef _NES_TEST
public:
#else
private:
#endif
    std::unique_ptr<Mapper> mapper;
    std::unique_ptr<PPU> ppu;
    std::unique_ptr<CPU> cpu;


    Console(std::unique_ptr<Mapper> &&);

public:
    static std::shared_ptr<Console> Create(std::unique_ptr<Mapper> &&);
};

} // namespace nes
