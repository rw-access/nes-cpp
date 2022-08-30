#pragma once
#include "nes.h"

namespace nes {

enum class Buttons : uint8_t {
    A      = 0,
    B      = 1,
    Select = 2,
    Start  = 3,
    Up     = 4,
    Down   = 5,
    Left   = 6,
    Right  = 7,
};

class Controller {
public:
    std::bitset<8> buttons;
    bool strobe;

    Byte Read();
    void Write(Byte data);

private:
    uint8_t index = 0;
};


} // namespace nes
