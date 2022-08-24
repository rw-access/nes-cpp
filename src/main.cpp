#include "cpu.h"
#include "nes.h"
#include <iostream>

int main() {
    /*
    nes::Memory m;
    nes::CPU cpu(m);

    auto numCycles = cpu.step();
     */
    auto numCycles = 0;
    std::cout << "Executed " << std::to_string(numCycles) << " cycles";
    return 0;
}
