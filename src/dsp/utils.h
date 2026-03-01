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

inline void calculate_spectrum_db(const float* real, const float* imag, 
                                  float* spectrum, size_t num_bins,
                                  float min_db = -35.0f, float max_db = 0.0f) {
    for (size_t i = 0; i < num_bins; i++) {
        float mag = std::sqrt(real[i] * real[i] + imag[i] * imag[i]);
        float db = 20.0f * std::log10(mag + 1e-10f);
        spectrum[i] = std::max(min_db, std::min(db, max_db));
    }
}
