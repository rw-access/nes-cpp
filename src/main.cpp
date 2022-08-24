#include "nes.h"
#include <iostream>

int main() {

    nes::Memory m;
    nes::CPU cpu(m);

    auto numCycles = cpu.step();
    std::cout << "Executed " << std::to_string(numCycles) << " cycles";
    return 0;
}
