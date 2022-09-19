#include "dsp.h"
#include "math.h"

namespace nes {
float FirstOrderFilter::filter(float input) {
    this->outPrev = (this->inPrev * this->inPrevWeight) + (this->outPrev * outPrevWeight) + (this->inWeight * input);
    this->inPrev  = input;
    return this->outPrev;
}

HighPassFilter::HighPassFilter(float sampleRate, float freq) {
    float dt            = 1 / sampleRate;
    float rc            = 1 / (2.0 * M_PI * freq);

    this->outPrevWeight = rc / (rc + dt);
    this->inWeight      = this->outPrevWeight;
    this->inPrevWeight  = -this->inWeight;
}

LowPassFilter::LowPassFilter(float sampleRate, float freq) {
    // https://en.wikipedia.org/wiki/Low-pass_filter#Discrete-time_realization
    float dt            = 1 / sampleRate;
    float rc            = 1 / (2.0 * M_PI * freq);

    this->inWeight      = dt / (rc + dt);
    this->inPrevWeight  = 0;
    this->outPrevWeight = (1 - this->inWeight);
}
} // namespace nes