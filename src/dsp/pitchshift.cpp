#include "dsp/pitchshift.h"
#include "config.h"
#include <cmath>
#include <algorithm>
#include <vector>

PitchShifter::PitchShifter(size_t fft_size, size_t hop_size, int sample_rate)
    : fft_size_(fft_size), hop_size_(hop_size), sample_rate_(sample_rate),
      pitch_ratio_(1.0f), volume_(1.0f),
      Hann_window_(fft_size),
      input_buffer_(fft_size),
      output_buffer_(fft_size),
      fft_real_(fft_size / 2 + 1),
      fft_imag_(fft_size / 2 + 1) {

    fft_ = std::make_unique<FFTProcessor>(fft_size);

    for (size_t i = 0; i < fft_size; i++) {
        Hann_window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fft_size - 1)));
    }

    std::fill(input_buffer_.begin(), input_buffer_.end(), 0.0f);
    std::fill(output_buffer_.begin(), output_buffer_.end(), 0.0f);
}

PitchShifter::~PitchShifter() = default;

void PitchShifter::set_pitch_ratio(float ratio) {
    pitch_ratio_ = ratio;
}

void PitchShifter::set_volume(float vol) {
    volume_ = std::max(0.0f, std::min(vol, 1.0f));
}

void PitchShifter::process(const float* input, float* output, int num_frames) {
    for (size_t i = 0; i < fft_size_; i++) {
        if (i < static_cast<size_t>(num_frames)) {
            input_buffer_[i] = input[i] * Hann_window_[i];
        } else {
            input_buffer_[i] = 0.0f;
        }
    }

    fft_->forward(input_buffer_.data(), fft_real_.data(), fft_imag_.data());

    fft_->inverse(fft_real_.data(), fft_imag_.data(), output_buffer_.data());

    for (size_t i = 0; i < fft_size_; i++) {
        output_buffer_[i] *= Hann_window_[i] * volume_;
    }

    for (int i = 0; i < num_frames; i++) {
        output[i] = output_buffer_[i];
    }
}

void PitchShifter::get_spectrum(float* spectrum, size_t num_bins) {
    for (size_t i = 0; i < num_bins && i < fft_size_ / 2 + 1; i++) {
        float magnitude = std::sqrt(fft_real_[i] * fft_real_[i] + 
                                     fft_imag_[i] * fft_imag_[i]);
        
        float db = 20.0f * std::log10(magnitude + 1e-10f);
        spectrum[i] = std::max(SPECTRUM_MIN_DB, std::min(db, SPECTRUM_MAX_DB));
    }
}
