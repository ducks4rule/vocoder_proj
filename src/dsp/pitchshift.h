#pragma once

#include <cstddef>
#include "dsp/fft.h"

class PitchShifter {
public:
    PitchShifter(size_t fft_size, size_t hop_size, int sample_rate);
    ~PitchShifter();

    void set_pitch_ratio(float ratio);
    float get_pitch_ratio() const { return pitch_ratio_; }

    void set_volume(float vol);
    float get_volume() const { return volume_; }

    void process(const float* input, float* output, int num_frames);

private:
    size_t fft_size_;
    size_t hop_size_;
    int sample_rate_;
    float pitch_ratio_;
    float volume_;

    FFTProcessor* fft_;
    float* Hann_window_;
    float* input_buffer_;
    float* output_buffer_;
    float* fft_real_;
    float* fft_imag_;
    size_t buffer_pos_;
};
