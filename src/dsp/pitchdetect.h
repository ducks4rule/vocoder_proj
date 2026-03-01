#pragma once

#include <cstddef>
#include <vector>

class PitchDetector {
public:
    PitchDetector(size_t buffer_size, int sample_rate);
    ~PitchDetector();

    float detect(const float* buffer, int num_samples);

    float get_last_detected_freq() const { return last_freq_; }

private:
    size_t buffer_size_;
    int sample_rate_;
    float last_freq_;
    std::vector<float> difference_buffer_;
    std::vector<float> cmndf_buffer_;
};
