#include "dsp/pitchshift.h"
#include <cmath>
#include <algorithm>
#include <cstring>

PitchShifter::PitchShifter(size_t fft_size, size_t hop_size, int sample_rate)
    : fft_size_(fft_size), hop_size_(hop_size), sample_rate_(sample_rate),
      pitch_ratio_(1.0f), volume_(1.0f), buffer_pos_(0) {

    fft_ = new FFTProcessor(fft_size);

    Hann_window_ = new float[fft_size];
    input_buffer_ = new float[fft_size];
    output_buffer_ = new float[fft_size];
    fft_real_ = new float[fft_size / 2 + 1];
    fft_imag_ = new float[fft_size / 2 + 1];

    for (size_t i = 0; i < fft_size; i++) {
        Hann_window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fft_size - 1)));
    }

    std::memset(input_buffer_, 0, fft_size * sizeof(float));
    std::memset(output_buffer_, 0, fft_size * sizeof(float));
}

PitchShifter::~PitchShifter() {
    delete fft_;
    delete[] Hann_window_;
    delete[] input_buffer_;
    delete[] output_buffer_;
    delete[] fft_real_;
    delete[] fft_imag_;
}

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

    fft_->forward(input_buffer_, fft_real_, fft_imag_);

    fft_->inverse(fft_real_, fft_imag_, output_buffer_);

    for (size_t i = 0; i < fft_size_; i++) {
        output_buffer_[i] *= Hann_window_[i] * volume_;
    }

    for (int i = 0; i < num_frames; i++) {
        output[i] = output_buffer_[i];
    }
}
