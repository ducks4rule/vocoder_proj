#pragma once

#include <cstddef>

class FFTProcessor {
public:
    FFTProcessor(size_t fft_size);
    ~FFTProcessor();

    void forward(const float* input, float* real_out, float* imag_out);
    void inverse(const float* real_in, const float* imag_in, float* output);

    size_t size() const { return fft_size_; }

private:
    size_t fft_size_;
    void* plan_forward_;
    void* plan_inverse_;
};
