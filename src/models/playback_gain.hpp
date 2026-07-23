#pragma once

#include <cstddef>

namespace recorder::playback_gain {

float observePeak(float current_peak, const float* samples, size_t sample_count);
float fullScaleGain(float peak);
float apply(float sample, float gain);

}  // namespace recorder::playback_gain
