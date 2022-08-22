#include "nes.h"
#include <iostream>

int main() {

    nes::mem::Memory m;
    nes::cpu::CPU cpu(m);

    auto numCycles = cpu.step();
    std::cout << "Executed " << std::to_string(numCycles) << " cycles";
    return 0;
}
