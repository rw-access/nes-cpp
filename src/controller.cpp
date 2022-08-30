#include "controller.h"

namespace nes {
void Controller::Write(Byte data) {
    // https://www.nesdev.org/wiki/Standard_controller
    // 7  bit  0
    // ---- ----
    // xxxx xxxS
    //         |
    //         +- Controller shift register strobe
    this->strobe = (data & 1) == 1;

    if (this->strobe)
        this->index = 0;
}

Byte Controller::Read() {
    // https://www.nesdev.org/wiki/Standard_controller
    // Each read reports one bit at a time through D0. The first 8 reads will indicate which buttons
    // or directions are pressed (1 if pressed, 0 if not pressed). All subsequent reads will return 1 on official
    // Nintendo brand controllers but may return 0 on third party controllers such as the U-Force.
    bool isSet = true;

    if (this->index < 8) {
        isSet = this->buttons[this->index];

        // increment index if strobe is not set
        this->index += !this->strobe;
    }

    // 7  bit  0
    // ---- ----
    // xxxx xMES
    //       |||
    //       ||+- Primary controller status bit
    //       |+-- Expansion controller status bit (Famicom)
    //       +--- Microphone status bit (Famicom, $4016 only)
    return uint8_t(isSet);
}

} // namespace nes