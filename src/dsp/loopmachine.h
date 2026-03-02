#pragma once

#include <cstddef>
#include <vector>
#include <string>

enum class LoopState {
    IDLE,       // Normal passthrough (mic -> DSP -> speaker)
    RECORDING,  // Mic -> loop buffer (no output)
    PLAYING,    // Loop buffer -> DSP -> speaker (repeating)
    OVERDUB     // Mic -> buffer + loop -> DSP -> speaker
};

class LoopMachine {
public:
    explicit LoopMachine(int sample_rate);
    ~LoopMachine();

    int process(const float* mic_input, int mic_samples,
                float* output, int output_samples);

    void toggle_recording();
    void toggle_playback();
    void toggle_overdub();
    void clear();

    LoopState get_state() const { return state_; }
    int get_recorded_samples() const { return recorded_samples_; }
    int get_max_samples() const { return static_cast<int>(buffer_size_); }
    float get_recorded_length() const;
    bool has_content() const { return recorded_samples_ > 0; }

    std::string get_state_string() const;
    std::string get_empty_message() const { return empty_message_; }
    void clear_empty_message() { empty_message_.clear(); }

private:
    int sample_rate_;
    size_t buffer_size_;
    std::vector<float> buffer_;
    size_t write_pos_ = 0;
    size_t read_pos_ = 0;
    int recorded_samples_ = 0;

    LoopState state_ = LoopState::IDLE;
    std::string empty_message_;
};
