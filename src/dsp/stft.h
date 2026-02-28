#pragma once

#include <cstddef>
#include <vector>
#include <fftw3.h>

class STFTProcessor {
public:
    STFTProcessor(size_t fft_size, size_t hop_size, int sample_rate);
    ~STFTProcessor();

    void forward(const float* new_samples, int num_samples,
                 float* real_out, float* imag_out);

    void inverse(const float* real_in, const float* imag_in,
                 float* output, int num_samples);

    void get_spectrum(float* spectrum, size_t num_bins);

    size_t fft_size() const { return fft_size_; }
    size_t hop_size() const { return hop_size_; }

private:
    size_t fft_size_;
    size_t hop_size_;
    int sample_rate_;

    std::vector<float> input_history_;
    std::vector<float> processing_frame_;
    std::vector<float> output_accum_;
    std::vector<float> Hann_window_;

    std::vector<float> fft_real_;
    std::vector<float> fft_imag_;

    void* plan_forward_;
    void* plan_inverse_;
    float* fft_input_;
    float* fft_output_;
    fftwf_complex* fft_complex_;
};
