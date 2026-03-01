#include "dsp/pitchshift.h"
#include "config.h"
#include <cmath>
#include <algorithm>

namespace {

float shortest_angle_interp(float a, float b, float t) {
    float diff = b - a;
    while (diff > M_PI) diff -= 2.0f * M_PI;
    while (diff < -M_PI) diff += 2.0f * M_PI;
    return a + diff * t;
}

}

PitchShifter::PitchShifter(size_t fft_size, size_t hop_size, int sample_rate)
    : fft_size_(fft_size), hop_size_(hop_size), sample_rate_(sample_rate),
      pitch_ratio_(1.0f), volume_(1.0f),
      fft_real_(fft_size / 2 + 1),
      fft_imag_(fft_size / 2 + 1),
      last_phase_real_(fft_size / 2 + 1, 0.0f),
      last_phase_imag_(fft_size / 2 + 1, 0.0f),
      last_phase_(fft_size / 2 + 1, 0.0f) {

    std::fill(fft_real_.begin(), fft_real_.end(), 0.0f);
    std::fill(fft_imag_.begin(), fft_imag_.end(), 0.0f);
}

PitchShifter::~PitchShifter() = default;

void PitchShifter::set_pitch_ratio(float ratio) {
    if (std::abs(ratio - pitch_ratio_) > 0.1f) {
        std::fill(last_phase_.begin(), last_phase_.end(), 0.0f);
    }
    pitch_ratio_ = std::max(0.25f, std::min(ratio, 4.0f));
}

void PitchShifter::set_volume(float vol) {
    volume_ = std::max(0.0f, std::min(vol, 1.0f));
}

void PitchShifter::process(float* real, float* imag, size_t num_bins) {
    // Step 1: Extract magnitude and phase from input
    std::vector<float> mag(num_bins);
    std::vector<float> phase(num_bins);
    for (size_t i = 0; i < num_bins; i++) {
        mag[i] = std::sqrt(real[i] * real[i] + imag[i] * imag[i]);
        phase[i] = std::atan2(imag[i], real[i]);
    }

    // Step 2: Phase propagation (only when pitch ratio ≈ 1.0 for clean passthrough)
    // This ensures smooth phase continuity between frames
    if (std::abs(pitch_ratio_ - 1.0f) < 0.01f) {
        for (size_t i = 1; i < num_bins; i++) {  // Skip DC (i=0)
            float bin_freq = static_cast<float>(i) * sample_rate_ / fft_size_;
            float expected_phase_advance = 2.0f * M_PI * bin_freq * hop_size_ / sample_rate_;
            
            // Accumulate phase from previous frame
            float new_phase = last_phase_[i] + expected_phase_advance;
            
            // Store wrapped phase for next frame
            while (new_phase > M_PI) new_phase -= 2.0f * M_PI;
            while (new_phase < -M_PI) new_phase += 2.0f * M_PI;
            phase[i] = new_phase;
        }
    }

    // Store current phase for next frame
    for (size_t i = 0; i < num_bins; i++) {
        last_phase_[i] = phase[i];
    }

    // Step 3: If pitch ratio is 1.0, just apply volume and return
    if (std::abs(pitch_ratio_ - 1.0f) < 0.001f) {
        for (size_t i = 0; i < num_bins; i++) {
            real[i] = mag[i] * std::cos(phase[i]) * volume_;
            imag[i] = mag[i] * std::sin(phase[i]) * volume_;
        }
        return;
    }

    // Step 4: Magnitude scaling for pitch shifting
    std::fill(real, real + num_bins, 0.0f);
    std::fill(imag, imag + num_bins, 0.0f);

    float ratio = pitch_ratio_;
    for (size_t i = 0; i < num_bins; i++) {
        float src_bin = static_cast<float>(i) / ratio;
        
        if (src_bin >= num_bins - 1) continue;
        
        size_t i0 = static_cast<size_t>(src_bin);
        size_t i1 = std::min(i0 + 1, num_bins - 1);
        float frac = src_bin - static_cast<float>(i0);
        
        float mag_interp = mag[i0] * (1.0f - frac) + mag[i1] * frac;
        float phase_interp = shortest_angle_interp(phase[i0], phase[i1], frac);
        
        real[i] = mag_interp * std::cos(phase_interp) * volume_;
        imag[i] = mag_interp * std::sin(phase_interp) * volume_;
    }

    for (size_t i = 0; i < num_bins && i < last_phase_real_.size(); i++) {
        last_phase_real_[i] = real[i];
        last_phase_imag_[i] = imag[i];
    }
}
