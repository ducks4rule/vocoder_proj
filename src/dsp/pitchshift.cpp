#include "dsp/pitchshift.h"

PitchShifter::PitchShifter(size_t fft_size, size_t hop_size, int sample_rate)
    : fft_size_(fft_size), hop_size_(hop_size), sample_rate_(sample_rate),
      pitch_ratio_(1.0f) {
}

PitchShifter::~PitchShifter() {
}

void PitchShifter::set_pitch_ratio(float ratio) {
    pitch_ratio_ = ratio;
}

void PitchShifter::process(const float* input, float* output, int num_frames) {
    for (int i = 0; i < num_frames; i++) {
        output[i] = input[i];
    }
}
