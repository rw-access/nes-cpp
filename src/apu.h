#pragma once
#include "console.h"
#include "dsp.h"
#include "nes.h"

namespace nes {


// https://www.nesdev.org/wiki/APU_Sweep
struct SweepUnit {
    bool enabled;
    bool negate;
    bool reload;
    bool carry;
    Byte period;
    Byte shiftCount;
    Byte delay;

    bool step();
};

// In a synthesizer, an envelope is the way a sound's parameter changes over time.
// The NES APU has an envelope generator that controls the volume in one of two ways:
// - it can generate a decreasing saw envelope (like a decay phase of an ADSR) with optional looping,
// - or it can generate a constant volume that a more sophisticated software envelope generator can manipulate.
//
//Each volume envelope unit contains the following: start flag, divider, and decay level counter.
// https://www.nesdev.org/wiki/APU_Envelope
struct VolumeEnvelope {
    bool start;
    union {
        bool loop;
        bool disabled;
    };
    bool useConstVolume;
    Byte decayLevelCounter;
    Byte divider;
    union {
        Byte period;
        Byte constVolume;
    };

    inline void step();
    inline Byte volume() const;
};

// https://www.nesdev.org/wiki/APU_Pulse
struct Pulse {
    SweepUnit sweep;
    VolumeEnvelope envelope;
    Byte dutyType; // 12.5%, 25%, 50%, 75%
    Byte dutyOffset;
    uint16_t timer; // 11 bits
    uint16_t timerPeriod;
    uint16_t lengthCounter;
    bool enabled = false;

    Byte sample() const;
    void stepTimer();
    void stepLength();
    void stepSweep();
};

struct Triangle {
    uint16_t timer; // 11 bits
    uint16_t timerPeriod;
    uint16_t lengthCounter;
    uint16_t linearCounterPeriod;
    uint16_t linearCounterOffset;
    uint16_t phase;
    uint16_t lengthEnabled;
    bool enabled = false;
    bool reload;

    void stepTimer();
    void stepLength();
    void stepLinearCounter();
    Byte sample();
};

struct Noise {
    VolumeEnvelope envelope;
    uint16_t lengthCounter;
    uint16_t shiftRegister; // 15 bits
    uint8_t enabled = false;
    uint8_t period;
    uint8_t timer;
    uint8_t feedbackBit;
    void stepTimer();
    void stepLength();
    Byte sample();
};

struct DMC {};

class APU {
private:
    Console &console;
    ProcessAudioSample processSampleFn;

    Pulse pulses[2];
    Triangle triangle;
    Noise noise;

    std::array<FirstOrderFilter, 3> filters{
            HighPassFilter(48000, 90),
            HighPassFilter(48000, 440),
            LowPassFilter(48000, 14000),
    };

    uint32_t cyclesXFrameCounterFreq = 0;
    uint32_t cyclesXSampleFreq       = 0;
    uint32_t sampleFreq              = 48000;

    Byte framePeriod                 = 1;

    bool onFrameCounterEdge          = false;
    bool onSampleEdge                = false;
    bool useFiveStep                 = false;
    bool enableIRQ                   = false;
    bool onAPUCycle                  = true;

    void updateTicks();
    void stepFrameCounter();
    float sample();


public:
    APU(Console &);
    // receive a CPU clock
    void step();

    void registerAudioCallback(ProcessAudioSample processAudioSampleFn);

    // Only supports 0x4015
    Byte readRegister(Address addr) const;

    void writeRegister(Address addr, Byte data);
};

} // namespace nes