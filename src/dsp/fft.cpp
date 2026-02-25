#include "dsp/fft.h"

FFTProcessor::FFTProcessor(size_t fft_size) : fft_size_(fft_size),
    plan_forward_(nullptr), plan_inverse_(nullptr) {
}

FFTProcessor::~FFTProcessor() {
}

void FFTProcessor::forward(const float* input, float* real_out, float* imag_out) {
    (void)input;
    (void)real_out;
    (void)imag_out;
}

void FFTProcessor::inverse(const float* real_in, const float* imag_in, float* output) {
    (void)real_in;
    (void)imag_in;
    (void)output;
}
