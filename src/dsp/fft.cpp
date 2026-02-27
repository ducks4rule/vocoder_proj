#include "dsp/fft.h"
#include <fftw3.h>
#include <cstring>

FFTProcessor::FFTProcessor(size_t fft_size) : fft_size_(fft_size),
    plan_forward_(nullptr), plan_inverse_(nullptr),
    input_buffer_(fftwf_alloc_real(fft_size)),
    output_buffer_(fftwf_alloc_real(fft_size)),
    complex_buffer_(fftwf_alloc_complex(fft_size / 2 + 1)) {
    
    plan_forward_ = fftwf_plan_dft_r2c_1d(
        static_cast<int>(fft_size_), 
        input_buffer_, 
        complex_buffer_, 
        FFTW_ESTIMATE);
    
    plan_inverse_ = fftwf_plan_dft_c2r_1d(
        static_cast<int>(fft_size_), 
        complex_buffer_, 
        output_buffer_, 
        FFTW_ESTIMATE);
}

FFTProcessor::~FFTProcessor() {
    if (plan_forward_) {
        fftwf_destroy_plan(static_cast<fftwf_plan>(plan_forward_));
    }
    if (plan_inverse_) {
        fftwf_destroy_plan(static_cast<fftwf_plan>(plan_inverse_));
    }
    if (input_buffer_) {
        fftwf_free(input_buffer_);
    }
    if (output_buffer_) {
        fftwf_free(output_buffer_);
    }
    if (complex_buffer_) {
        fftwf_free(complex_buffer_);
    }
}

void FFTProcessor::forward(const float* input, float* real_out, float* imag_out) {
    std::memcpy(input_buffer_, input, fft_size_ * sizeof(float));
    
    fftwf_execute(static_cast<fftwf_plan>(plan_forward_));
    
    for (size_t i = 0; i < fft_size_ / 2 + 1; i++) {
        real_out[i] = complex_buffer_[i][0];
        imag_out[i] = complex_buffer_[i][1];
    }
}

void FFTProcessor::inverse(const float* real_in, const float* imag_in, float* output) {
    for (size_t i = 0; i < fft_size_ / 2 + 1; i++) {
        complex_buffer_[i][0] = real_in[i];
        complex_buffer_[i][1] = imag_in[i];
    }
    
    fftwf_execute(static_cast<fftwf_plan>(plan_inverse_));
    
    for (size_t i = 0; i < fft_size_; i++) {
        output[i] = output_buffer_[i] / static_cast<float>(fft_size_);
    }
}
