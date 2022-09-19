#pragma once

#include "nes.h"

namespace nes {
class FirstOrderFilter {
protected:
    float inPrev = 0, outPrev = 0;
    float inWeight, inPrevWeight, outPrevWeight;

public:
    float filter(float input);
};

class LowPassFilter : public FirstOrderFilter {
public:
    LowPassFilter(float sampleRate, float freq);
};

class HighPassFilter : public FirstOrderFilter {
public:
    HighPassFilter(float sampleRate, float freq);
};
} // namespace nes