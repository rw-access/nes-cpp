#include "apu.h"
#include "cpu.h"

namespace nes {


static const uint8_t lengthCounterTable[32] = {
        10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
        12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

#define PT(n) (95.52 / ((8128.0 / n) + 100))
static float pulseTable[31] = {
        0,      PT(1),  PT(2),  PT(3),  PT(4),  PT(5),  PT(6),  PT(7),  PT(8),  PT(9),  PT(10),
        PT(11), PT(12), PT(13), PT(14), PT(15), PT(16), PT(17), PT(18), PT(19), PT(20), PT(21),
        PT(22), PT(23), PT(24), PT(25), PT(26), PT(27), PT(28), PT(29), PT(30),
};
#undef PT

// read in the order 0, 7, 6, 5, 4, 3, 2, 1.
static const uint8_t dutyOn[4]{
        0b01000000, // 12.5%
        0b01100000, // 25%
        0b01111000, // 50%
        0b10011111, // 75%
};

bool SweepUnit::step() {
    auto shouldSweep = false;
    if (this->reload || this->delay == 0) {
        shouldSweep  = this->enabled && this->delay == 0;
        this->delay  = this->period;
        this->reload = false;
    } else {
        this->delay--;
    }

    return shouldSweep;
}

inline void VolumeEnvelope::step() {
    // https://www.nesdev.org/wiki/APU_Envelope
    if (this->start) {
        this->decayLevelCounter = 15;
        this->divider           = this->period;
        this->start             = false;
    } else if (this->divider > 0) {
        this->divider--;
    } else {
        this->divider = this->period;

        if (this->decayLevelCounter > 0)
            this->decayLevelCounter--;
        else if (this->loop)
            this->decayLevelCounter = 15;
    }
}

inline Byte VolumeEnvelope::volume() const {
    return this->useConstVolume ? this->constVolume : this->decayLevelCounter;
}

void Pulse::stepTimer() {
    // clocked by APU cycles (half CPU)
    if (this->timer > 0) {
        this->timer--;
    } else {
        this->timer      = this->period;
        this->dutyOffset = (this->dutyOffset + 1) % 8;
    }
}

void Pulse::stepLength() {
    if (!this->envelope.disabled && this->lengthCounter > 0)
        this->lengthCounter--;
}

void Pulse::stepSweep() {
    if (this->sweep.step()) {
        auto delta = this->period >> this->sweep.shiftCount;
        if (this->sweep.negate)
            delta = ~delta;

        this->period += delta + this->sweep.carry;
    }
}

Byte Pulse::sample() const {
    const bool high     = (dutyOn[this->dutyType] >> (7 - this->dutyOffset)) & 1;
    const bool silenced = this->period < 8 || (this->period > 0x7ff);

    if (!this->enabled || !this->lengthCounter || !high || silenced)
        return 0;

    return this->envelope.volume();
}

void APU::updateTicks() {
    // need to divide the CPU frequency into a non-integer amount.
    // this means that the positive edges won't consistently line up,
    // so need to detect positive edges that occur between CPU clock ticks
    const static uint32_t frameCounterFreq = 240;
    const static uint32_t cpuFreq          = 1789773;

    // calculate 240Hz ticks
    this->cyclesXFrameCounterFreq += frameCounterFreq;
    if (this->cyclesXFrameCounterFreq >= cpuFreq) {
        this->cyclesXFrameCounterFreq -= cpuFreq;
        this->onFrameCounterEdge = true;
    } else
        this->onFrameCounterEdge = false;


    // calculate sample rate (probably 44.1 kHz) ticks
    this->cyclesXSampleFreq += sampleFreq;
    if (this->cyclesXSampleFreq >= cpuFreq) {
        this->cyclesXSampleFreq -= cpuFreq;
        this->onSampleEdge = true;
    } else
        this->onSampleEdge = false;

    this->onAPUCycle = !this->onAPUCycle;
}

void APU::stepFrameCounter() {
    // https://www.nesdev.org/wiki/APU_Frame_Counter
    const Byte stepWithPeriod = 0x40 | (this->useFiveStep << 4) | (this->framePeriod & 0xf);
    switch (stepWithPeriod) {
        case 0x42:
        case 0x44:
        case 0x52:
        case 0x54:
            // TODO: length counters
            this->pulses[0].stepSweep();
            this->pulses[1].stepSweep();

            this->pulses[0].stepLength();
            this->pulses[1].stepLength();

            [[fallthrough]];
        case 0x41:
        case 0x43:
        case 0x51:
        case 0x53:
            // step: envelop + triangle linear counters
            // TODO: triangle + noise counters
            this->pulses[0].envelope.step();
            this->pulses[1].envelope.step();
            break;
        default:
            break;
    }

    // TODO: check interrupts
    if (stepWithPeriod == 0x44 && this->enableIRQ) {
        this->console.cpu->interrupt(Interrupt::IRQ);
    }

    // increment the frame period and wrap around
    this->framePeriod = (stepWithPeriod == 0x55 || stepWithPeriod == 0x44) ? 1 : this->framePeriod + 1;
}

float APU::sample() {
    // TODO: add other registers and non-linear sampling
    return pulseTable[this->pulses[0].sample() + this->pulses[1].sample()];
}

void APU::registerAudioCallback(ProcessAudioSample processAudioSampleFn) {
    this->processSampleFn = processAudioSampleFn;
}

APU::APU(Console &c) :
    console(c) {
    this->pulses[1].sweep.carry = 1;
}

void APU::step() {
    if (this->onAPUCycle) {
        // pulse + noise + dmc
        this->pulses[0].stepTimer();
        this->pulses[1].stepTimer();
    }

    // TODO: update triangle on every CPU

    if (this->onFrameCounterEdge)
        this->stepFrameCounter();

    if (this->onSampleEdge && this->processSampleFn != nullptr)
        this->processSampleFn(this->sample());

    this->updateTicks();
}

Byte APU::readRegister(Address addr) const {
    switch (addr) {
        case 0x4015:
            // TODO: triangle + dmc + noise
            return (!!this->pulses[0].lengthCounter) | (!!this->pulses[1].lengthCounter << 1);
    }
    return 0;
}

void APU::writeRegister(Address addr, Byte data) {
    Byte pulseReg = (addr & 0x4) >> 2;

    switch (addr) {
        case 0x4000:
        case 0x4004:
            this->pulses[pulseReg].envelope.period         = data & 0xf;
            this->pulses[pulseReg].envelope.useConstVolume = (data >> 4) & 0x1;
            this->pulses[pulseReg].envelope.loop           = (data >> 5) & 0x1;
            this->pulses[pulseReg].dutyType                = (data >> 6) & 0x3;
            this->pulses[pulseReg].envelope.start          = true;
            break;
        case 0x4001:
        case 0x4005:
            this->pulses[pulseReg].sweep.shiftCount = data & 0x7;
            this->pulses[pulseReg].sweep.negate     = (data >> 3) & 1;
            this->pulses[pulseReg].sweep.period     = ((data >> 4) & 7) + 1;
            this->pulses[pulseReg].sweep.enabled    = (data >> 7) & 1;
            this->pulses[pulseReg].sweep.reload     = true;
            break;
        case 0x4002:
        case 0x4006:
            this->pulses[pulseReg].period &= 0xff00;
            this->pulses[pulseReg].period |= data;
            break;
        case 0x4003:
        case 0x4007:
            this->pulses[pulseReg].period &= 0x00ff;
            this->pulses[pulseReg].period |= uint16_t(data & 0x7) << 8;
            this->pulses[pulseReg].lengthCounter  = lengthCounterTable[data >> 3];
            this->pulses[pulseReg].envelope.start = true;
            this->pulses[pulseReg].dutyOffset     = 0;
            break;
        case 0x4008:
        case 0x4009:
        case 0x400A:
        case 0x400B:
        case 0x400C:
        case 0x400D:
        case 0x400E:
        case 0x400F:
        case 0x4010:
        case 0x4011:
        case 0x4012:
        case 0x4013:
            break;
        case 0x4015:
            this->pulses[0].enabled = data & 0x1;
            this->pulses[1].enabled = (data >> 1) & 0x1;
            break;
        case 0x4017:
            this->useFiveStep = (data >> 7) & 1;
            this->enableIRQ   = (~data >> 6) & 1;

            if (this->useFiveStep) {
                for (auto i = 0; i < 2; i++) {
                    this->pulses[i].envelope.step();
                    this->pulses[i].stepSweep();
                    this->pulses[i].stepLength();
                }
            }
            break;
    }
}

} // namespace nes