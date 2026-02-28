#pragma once

#include <cstddef>
#include <vector>

class PitchShifter {
public:
    PitchShifter(size_t fft_size, size_t hop_size, int sample_rate);
    ~PitchShifter();

    void set_pitch_ratio(float ratio);
    float get_pitch_ratio() const { return pitch_ratio_; }

    void set_volume(float vol);
    float get_volume() const { return volume_; }

    void process(float* real, float* imag, size_t num_bins);

private:
    size_t fft_size_;
    size_t hop_size_;
    int sample_rate_;
    float pitch_ratio_;
    float volume_;

    std::vector<float> fft_real_;
    std::vector<float> fft_imag_;
};
