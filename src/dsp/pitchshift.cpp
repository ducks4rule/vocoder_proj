#include "dsp/pitchshift.h"
#include "config.h"
#include <cmath>
#include <algorithm>

PitchShifter::PitchShifter(size_t fft_size, size_t hop_size, int sample_rate)
    : fft_size_(fft_size), hop_size_(hop_size), sample_rate_(sample_rate),
      pitch_ratio_(1.0f), volume_(1.0f),
      fft_real_(fft_size / 2 + 1),
      fft_imag_(fft_size / 2 + 1) {

    std::fill(fft_real_.begin(), fft_real_.end(), 0.0f);
    std::fill(fft_imag_.begin(), fft_imag_.end(), 0.0f);
}

PitchShifter::~PitchShifter() = default;

void PitchShifter::set_pitch_ratio(float ratio) {
    pitch_ratio_ = ratio;
}

void PitchShifter::set_volume(float vol) {
    volume_ = std::max(0.0f, std::min(vol, 1.0f));
}

void PitchShifter::process(float* real, float* imag, size_t num_bins) {
    // Store a copy for spectrum analysis
    for (size_t i = 0; i < num_bins && i < fft_real_.size(); i++) {
        fft_real_[i] = real[i];
        fft_imag_[i] = imag[i];
    }

    // TODO: Implement actual pitch shifting here
    // For now, just apply volume (passthrough)
    for (size_t i = 0; i < num_bins; i++) {
        real[i] *= volume_;
        imag[i] *= volume_;
    }
}
