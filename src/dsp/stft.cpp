#include "dsp/stft.h"
#include "config.h"
#include <cmath>
#include <cstring>
#include <algorithm>

STFTProcessor::STFTProcessor(size_t fft_size, size_t hop_size, int sample_rate)
    : fft_size_(fft_size), hop_size_(hop_size), sample_rate_(sample_rate),
      input_history_(fft_size, 0.0f),
      processing_frame_(fft_size, 0.0f),
      output_accum_(fft_size + hop_size, 0.0f),
      Hann_window_(fft_size),
      fft_real_(fft_size / 2 + 1, 0.0f),
      fft_imag_(fft_size / 2 + 1, 0.0f) {

    fft_input_ = fftwf_alloc_real(fft_size);
    fft_output_ = fftwf_alloc_real(fft_size);
    fft_complex_ = fftwf_alloc_complex(fft_size / 2 + 1);

    plan_forward_ = fftwf_plan_dft_r2c_1d(
        static_cast<int>(fft_size_),
        fft_input_,
        fft_complex_,
        FFTW_ESTIMATE);

    plan_inverse_ = fftwf_plan_dft_c2r_1d(
        static_cast<int>(fft_size_),
        fft_complex_,
        fft_output_,
        FFTW_ESTIMATE);

    for (size_t i = 0; i < fft_size; i++) {
        Hann_window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fft_size - 1)));
    }

    std::fill(input_history_.begin(), input_history_.end(), 0.0f);
    std::fill(output_accum_.begin(), output_accum_.end(), 0.0f);
}

STFTProcessor::~STFTProcessor() {
    if (plan_forward_) {
        fftwf_destroy_plan(static_cast<fftwf_plan>(plan_forward_));
    }
    if (plan_inverse_) {
        fftwf_destroy_plan(static_cast<fftwf_plan>(plan_inverse_));
    }
    if (fft_input_) {
        fftwf_free(fft_input_);
    }
    if (fft_output_) {
        fftwf_free(fft_output_);
    }
    if (fft_complex_) {
        fftwf_free(fft_complex_);
    }
}

void STFTProcessor::forward(const float* new_samples, int num_samples,
                           float* real_out, float* imag_out) {
    // Shift input history down and add new samples at the end
    // This is a linear shift - in production we'd use a circular buffer
    // but this is clearer for understanding
    if (num_samples > 0 && static_cast<size_t>(num_samples) <= hop_size_) {
        size_t shift_amount = fft_size_ - num_samples;
        
        // Shift existing samples down
        for (size_t i = 0; i < shift_amount; i++) {
            input_history_[i] = input_history_[i + num_samples];
        }
        
        // Add new samples at the end
        for (size_t i = 0; i < static_cast<size_t>(num_samples); i++) {
            input_history_[shift_amount + i] = new_samples[i];
        }
    }

    // Copy to processing frame and apply Hann window
    for (size_t i = 0; i < fft_size_; i++) {
        processing_frame_[i] = input_history_[i] * Hann_window_[i];
    }

    // FFT
    std::memcpy(fft_input_, processing_frame_.data(), fft_size_ * sizeof(float));
    fftwf_execute(static_cast<fftwf_plan>(plan_forward_));

    for (size_t i = 0; i < fft_size_ / 2 + 1; i++) {
        real_out[i] = fft_complex_[i][0];
        imag_out[i] = fft_complex_[i][1];
        
        fft_real_[i] = fft_complex_[i][0];
        fft_imag_[i] = fft_complex_[i][1];
    }
}

void STFTProcessor::inverse(const float* real_in, const float* imag_in,
                           float* output, int num_samples) {
    // Copy frequency bins to FFT complex buffer
    for (size_t i = 0; i < fft_size_ / 2 + 1; i++) {
        fft_complex_[i][0] = real_in[i];
        fft_complex_[i][1] = imag_in[i];
    }

    // IFFT
    fftwf_execute(static_cast<fftwf_plan>(plan_inverse_));

    // Apply Hann window and normalize
    for (size_t i = 0; i < fft_size_; i++) {
        processing_frame_[i] = fft_output_[i] * Hann_window_[i] / static_cast<float>(fft_size_);
    }

    // Overlap-add to output accumulator
    for (size_t i = 0; i < fft_size_; i++) {
        output_accum_[i] += processing_frame_[i];
    }

    // Copy hop_size samples to output (these are ready now)
    for (int i = 0; i < num_samples; i++) {
        output[i] = output_accum_[i];
    }

    // Shift remaining samples down in the accumulator
    // This makes room for the next overlap-add
    size_t remaining = output_accum_.size() - num_samples;
    for (size_t i = 0; i < remaining; i++) {
        output_accum_[i] = output_accum_[i + num_samples];
    }
    // Zero out the rest
    for (size_t i = remaining; i < output_accum_.size(); i++) {
        output_accum_[i] = 0.0f;
    }
}

void STFTProcessor::get_spectrum(float* spectrum, size_t num_bins) {
    for (size_t i = 0; i < num_bins && i < fft_size_ / 2 + 1; i++) {
        float magnitude = std::sqrt(fft_real_[i] * fft_real_[i] + 
                                     fft_imag_[i] * fft_imag_[i]);
        
        float db = 20.0f * std::log10(magnitude + 1e-10f);
        spectrum[i] = std::max(SPECTRUM_MIN_DB, std::min(db, SPECTRUM_MAX_DB));
    }
}
