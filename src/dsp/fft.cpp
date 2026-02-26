#include "dsp/fft.h"
#include <fftw3.h>
#include <cstring>

FFTProcessor::FFTProcessor(size_t fft_size) : fft_size_(fft_size),
    plan_forward_(nullptr), plan_inverse_(nullptr) {
    
    float* in = fftwf_alloc_real(fft_size_);
    fftwf_complex* out = fftwf_alloc_complex(fft_size_ / 2 + 1);
    
    plan_forward_ = fftwf_plan_dft_r2c_1d(
        static_cast<int>(fft_size_), in, out, FFTW_ESTIMATE);
    
    fftwf_free(in);
    fftwf_free(out);
    
    float* out_inv = fftwf_alloc_real(fft_size_);
    fftwf_complex* in_inv = fftwf_alloc_complex(fft_size_ / 2 + 1);
    
    plan_inverse_ = fftwf_plan_dft_c2r_1d(
        static_cast<int>(fft_size_), in_inv, out_inv, FFTW_ESTIMATE);
    
    fftwf_free(out_inv);
    fftwf_free(in_inv);
}

FFTProcessor::~FFTProcessor() {
    if (plan_forward_) {
        fftwf_destroy_plan(static_cast<fftwf_plan>(plan_forward_));
    }
    if (plan_inverse_) {
        fftwf_destroy_plan(static_cast<fftwf_plan>(plan_inverse_));
    }
}

void FFTProcessor::forward(const float* input, float* real_out, float* imag_out) {
    float in[fft_size_];
    std::memcpy(in, input, fft_size_ * sizeof(float));
    
    fftwf_complex out[fft_size_ / 2 + 1];
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(
        static_cast<int>(fft_size_), in, out, FFTW_ESTIMATE | FFTW_DESTROY_INPUT);
    
    fftwf_execute(plan);
    fftwf_destroy_plan(plan);
    
    for (size_t i = 0; i < fft_size_ / 2 + 1; i++) {
        real_out[i] = out[i][0];
        imag_out[i] = out[i][1];
    }
}

void FFTProcessor::inverse(const float* real_in, const float* imag_in, float* output) {
    fftwf_complex in[fft_size_ / 2 + 1];
    for (size_t i = 0; i < fft_size_ / 2 + 1; i++) {
        in[i][0] = real_in[i];
        in[i][1] = imag_in[i];
    }
    
    float out[fft_size_];
    fftwf_plan plan = fftwf_plan_dft_c2r_1d(
        static_cast<int>(fft_size_), in, out, FFTW_ESTIMATE | FFTW_DESTROY_INPUT);
    
    fftwf_execute(plan);
    fftwf_destroy_plan(plan);
    
    for (size_t i = 0; i < fft_size_; i++) {
        output[i] = out[i] / static_cast<float>(fft_size_);
    }
}
