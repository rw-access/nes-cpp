#include "apu.h"
#include "cpu.h"

namespace nes {


static const uint8_t lengthCounterTable[32] = {
        10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
        12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

static const uint16_t noisePeriodTable[16] = {
        4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};


// https://www.nesdev.org/wiki/APU_Mixer
#define PT(n) (95.52 / ((8128.0 / n) + 100))
static const float pulseTable[31] = {
        0,      PT(1),  PT(2),  PT(3),  PT(4),  PT(5),  PT(6),  PT(7),  PT(8),  PT(9),  PT(10),
        PT(11), PT(12), PT(13), PT(14), PT(15), PT(16), PT(17), PT(18), PT(19), PT(20), PT(21),
        PT(22), PT(23), PT(24), PT(25), PT(26), PT(27), PT(28), PT(29), PT(30),
};
#undef PT

#define TND(n) 163.67 / (24329.0 / n + 100)

static const float tndTable[203] = {
        0,        TND(1),   TND(2),   TND(3),   TND(4),   TND(5),   TND(6),   TND(7),   TND(8),   TND(9),   TND(10),
        TND(11),  TND(12),  TND(13),  TND(14),  TND(15),  TND(16),  TND(17),  TND(18),  TND(19),  TND(20),  TND(21),
        TND(22),  TND(23),  TND(24),  TND(25),  TND(26),  TND(27),  TND(28),  TND(29),  TND(30),  TND(31),  TND(32),
        TND(33),  TND(34),  TND(35),  TND(36),  TND(37),  TND(38),  TND(39),  TND(40),  TND(41),  TND(42),  TND(43),
        TND(44),  TND(45),  TND(46),  TND(47),  TND(48),  TND(49),  TND(50),  TND(51),  TND(52),  TND(53),  TND(54),
        TND(55),  TND(56),  TND(57),  TND(58),  TND(59),  TND(60),  TND(61),  TND(62),  TND(63),  TND(64),  TND(65),
        TND(66),  TND(67),  TND(68),  TND(69),  TND(70),  TND(71),  TND(72),  TND(73),  TND(74),  TND(75),  TND(76),
        TND(77),  TND(78),  TND(79),  TND(80),  TND(81),  TND(82),  TND(83),  TND(84),  TND(85),  TND(86),  TND(87),
        TND(88),  TND(89),  TND(90),  TND(91),  TND(92),  TND(93),  TND(94),  TND(95),  TND(96),  TND(97),  TND(98),
        TND(99),  TND(100), TND(101), TND(102), TND(103), TND(104), TND(105), TND(106), TND(107), TND(108), TND(109),
        TND(110), TND(111), TND(112), TND(113), TND(114), TND(115), TND(116), TND(117), TND(118), TND(119), TND(120),
        TND(121), TND(122), TND(123), TND(124), TND(125), TND(126), TND(127), TND(128), TND(129), TND(130), TND(131),
        TND(132), TND(133), TND(134), TND(135), TND(136), TND(137), TND(138), TND(139), TND(140), TND(141), TND(142),
        TND(143), TND(144), TND(145), TND(146), TND(147), TND(148), TND(149), TND(150), TND(151), TND(152), TND(153),
        TND(154), TND(155), TND(156), TND(157), TND(158), TND(159), TND(160), TND(161), TND(162), TND(163), TND(164),
        TND(165), TND(166), TND(167), TND(168), TND(169), TND(170), TND(171), TND(172), TND(173), TND(174), TND(175),
        TND(176), TND(177), TND(178), TND(179), TND(180), TND(181), TND(182), TND(183), TND(184), TND(185), TND(186),
        TND(187), TND(188), TND(189), TND(190), TND(191), TND(192), TND(193), TND(194), TND(195), TND(196), TND(197),
        TND(198), TND(199), TND(200), TND(201), TND(202),
};
#undef TND

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
        this->timer      = this->timerPeriod;
        this->dutyOffset = (this->dutyOffset + 1) % 8;
    }
}

void Pulse::stepLength() {
    if (!this->envelope.disabled && this->lengthCounter > 0)
        this->lengthCounter--;
}

void Pulse::stepSweep() {
    if (this->sweep.step()) {
        auto delta = this->timerPeriod >> this->sweep.shiftCount;
        if (this->sweep.negate)
            delta = ~delta;

        this->timerPeriod += delta + this->sweep.carry;
    }
}

Byte Pulse::sample() const {
    const bool high     = (dutyOn[this->dutyType] >> (7 - this->dutyOffset)) & 1;
    const bool silenced = this->timerPeriod < 8 || (this->timerPeriod > 0x7ff);

    if (!this->enabled || !this->lengthCounter || !high || silenced)
        return 0;

    return this->envelope.volume();
}

void Triangle::stepTimer() {
    // clocked by CPU cycles
    if (this->timer > 0) {
        this->timer--;
    } else {
        this->timer = this->timerPeriod;

        if (this->lengthCounter > 0 && this->linearCounterOffset > 0)
            this->phase = (this->phase + 1) % 32;
    }
}

void Triangle::stepLinearCounter() {
    if (this->reload)
        this->linearCounterOffset = this->linearCounterPeriod;
    else if (this->linearCounterOffset > 0)
        this->linearCounterOffset--;

    if (this->lengthEnabled)
        this->reload = false;
}

void Triangle::stepLength() {
    if (this->lengthEnabled && this->lengthCounter > 0)
        this->lengthCounter--;
}

Byte Triangle::sample() {
    if (this->enabled && this->timerPeriod && this->lengthCounter && this->linearCounterOffset)
        // index into 15 14 ... 1 0 0 1 ... 14 15
        // python: [(15 -phase) ^ -(phase >= 16) for phase in range(32)]
        return (15 - this->phase) ^ -(this->phase >= 16);

    return 0;
}

void Noise::stepTimer() {
    if (this->timer > 0) {
        this->timer--;
    } else {
        this->timer = this->period;
        this->shiftRegister |= uint16_t(((this->shiftRegister >> this->feedbackBit) ^ this->shiftRegister) & 0x1) << 15;
        this->shiftRegister >>= 1;
    }
}

Byte Noise::sample() {
    if (!this->enabled || !this->lengthCounter || ~this->shiftRegister & 0x1)
        return 0;

    return this->envelope.volume();
}

void Noise::stepLength() {
    if (!this->envelope.disabled && this->lengthCounter > 0)
        this->lengthCounter--;
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

            this->triangle.stepLength();
            this->noise.stepLength();

            [[fallthrough]];
        case 0x41:
        case 0x43:
        case 0x51:
        case 0x53:
            // step: envelop + triangle linear counters
            // TODO: triangle + noise counters
            this->pulses[0].envelope.step();
            this->pulses[1].envelope.step();
            this->noise.envelope.step();

            this->triangle.stepLinearCounter();
            break;
        default:
            break;
    }

    // TODO: check interrupts
    if (stepWithPeriod == 0x44 && this->enableIRQ) {
        this->console.cpu->interrupt(Interrupt::IRQ);
    }

    // increment the frame timerPeriod and wrap around
    this->framePeriod = (stepWithPeriod == 0x55 || stepWithPeriod == 0x44) ? 1 : this->framePeriod + 1;
}

float APU::sample() {
    // TODO: noise + DMC
    // https://www.nesdev.org/wiki/APU_Mixer#Lookup_Table
    auto pulseSample = pulseTable[this->pulses[0].sample() + this->pulses[1].sample()];
    auto tndSample   = tndTable[3 * this->triangle.sample() + 2 * this->noise.sample() + 0];
    return pulseSample + tndSample;
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
        this->noise.stepTimer();
    }

    // TODO: DMC + Noise
    this->triangle.stepTimer();

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
            return (this->pulses[0].lengthCounter) | (!!this->pulses[1].lengthCounter << 1) |
                   (!!this->triangle.lengthCounter << 2);
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
            this->pulses[pulseReg].timerPeriod &= 0xff00;
            this->pulses[pulseReg].timerPeriod |= data;
            break;
        case 0x4003:
        case 0x4007:
            this->pulses[pulseReg].timerPeriod &= 0x00ff;
            this->pulses[pulseReg].timerPeriod |= uint16_t(data & 0x7) << 8;
            this->pulses[pulseReg].lengthCounter  = lengthCounterTable[data >> 3];
            this->pulses[pulseReg].envelope.start = true;
            this->pulses[pulseReg].dutyOffset     = 0;
            break;
        case 0x4008:
            this->triangle.lengthEnabled       = ((data >> 7) & 0x1) == 0x0;
            this->triangle.linearCounterPeriod = data & 0x7f;
            break;
        case 0x4009:
            break;
        case 0x400A:
            this->triangle.timerPeriod &= 0xff00;
            this->triangle.timerPeriod |= data;
            break;
        case 0x400B:
            this->triangle.timerPeriod &= 0x00ff;
            this->triangle.timerPeriod |= uint16_t(data & 0x7) << 8;
            this->triangle.lengthCounter = lengthCounterTable[data >> 3];
            this->triangle.reload        = true;
            this->triangle.phase         = 0;
            break;
        case 0x400C:
            this->noise.envelope.period         = data & 0xf;
            this->noise.envelope.useConstVolume = (data >> 4) & 0x1;
            this->noise.envelope.loop           = (data >> 5) & 0x1;
            this->noise.envelope.start          = true;
            break;
        case 0x400D:
            break;
        case 0x400E:
            this->noise.feedbackBit = (data >> 7) ? 6 : 1;
            this->noise.period      = noisePeriodTable[data & 0xf];
            break;
        case 0x400F:
            this->noise.lengthCounter  = lengthCounterTable[data >> 3];
            this->noise.envelope.start = true;
            break;
        case 0x4010:
        case 0x4011:
        case 0x4012:
        case 0x4013:
            break;
        case 0x4015:
            this->pulses[0].enabled = data & 0x1;
            this->pulses[1].enabled = (data >> 1) & 0x1;
            this->triangle.enabled  = (data >> 2) & 0x1;
            this->noise.enabled     = (data >> 3) & 0x1;

            for (int i = 0; i < 2; i++)
                if (!this->pulses[i].enabled)
                    this->pulses[i].lengthCounter = 0;

            if (!this->triangle.enabled)
                this->triangle.lengthCounter = 0;

            if (!this->noise.enabled)
                this->noise.lengthCounter = 0;

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

                this->triangle.stepLength();
                this->noise.stepLength();
                this->noise.envelope.step();
            }
            break;
    }
}

} // namespace nes