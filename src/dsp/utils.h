#pragma once

#include <cmath>

inline float calculate_db(const float* buffer, int frames) {
    if (frames <= 0) return -60.0f;
    float sum = 0.0f;
    for (int i = 0; i < frames; i++) {
        sum += buffer[i] * buffer[i];
    }
    float rms = std::sqrt(sum / frames);
    if (rms > 0.0f) {
        float db = 20.0f * std::log10(rms);
        return std::max(-60.0f, std::min(db, 0.0f));
    }
    return -60.0f;
}
