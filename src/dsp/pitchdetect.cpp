#include "dsp/pitchdetect.h"
#include "config.h"
#include <cmath>
#include <algorithm>

namespace {
    constexpr float YIN_THRESHOLD = 0.15f;
    constexpr float MIN_FREQ = 50.0f;
    constexpr float MAX_FREQ = 2000.0f;
}

PitchDetector::PitchDetector(size_t buffer_size, int sample_rate)
    : buffer_size_(buffer_size),
      sample_rate_(sample_rate),
      last_freq_(0.0f),
      difference_buffer_(buffer_size / 2),
      cmndf_buffer_(buffer_size / 2) {
}

PitchDetector::~PitchDetector() = default;

float PitchDetector::detect(const float* buffer, int num_samples) {
    if (num_samples < 2) {
        return 0.0f;
    }

    size_t half_size = num_samples / 2;
    if (difference_buffer_.size() < half_size) {
        difference_buffer_.resize(half_size);
        cmndf_buffer_.resize(half_size);
    }

    float min_period = sample_rate_ / MAX_FREQ;
    float max_period = sample_rate_ / MIN_FREQ;
    
    size_t tau_min = static_cast<size_t>(min_period);
    size_t tau_max = static_cast<size_t>(std::min(static_cast<float>(half_size), max_period));
    
    if (tau_min >= tau_max || tau_max > half_size) {
        return 0.0f;
    }

    for (size_t tau = tau_min; tau < tau_max; tau++) {
        float sum = 0.0f;
        for (size_t i = 0; i < half_size; i++) {
            float diff = buffer[i] - buffer[i + tau];
            sum += diff * diff;
        }
        difference_buffer_[tau] = sum;
    }

    cmndf_buffer_[tau_min] = 1.0f;
    
    float running_sum = difference_buffer_[tau_min];
    for (size_t tau = tau_min + 1; tau < tau_max; tau++) {
        running_sum += difference_buffer_[tau];
        if (running_sum > 0.0f) {
            cmndf_buffer_[tau] = difference_buffer_[tau] * tau / running_sum;
        } else {
            cmndf_buffer_[tau] = 1.0f;
        }
    }

    size_t tau_estimate = tau_min;
    for (size_t tau = tau_min + 1; tau < tau_max; tau++) {
        if (cmndf_buffer_[tau] < cmndf_buffer_[tau_estimate]) {
            tau_estimate = tau;
        }
    }

    float best_tau = tau_estimate;
    float best_val = cmndf_buffer_[tau_estimate];

    if (best_val < YIN_THRESHOLD) {
        if (tau_estimate > tau_min && tau_estimate < tau_max - 1) {
            float s0 = cmndf_buffer_[tau_estimate - 1];
            float s1 = cmndf_buffer_[tau_estimate];
            float s2 = cmndf_buffer_[tau_estimate + 1];
            float shift = (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
            best_tau = tau_estimate + shift;
        }
        
        float freq = sample_rate_ / best_tau;
        
        if (freq >= MIN_FREQ && freq <= MAX_FREQ) {
            last_freq_ = freq;
            return freq;
        }
    }

    return 0.0f;
}
