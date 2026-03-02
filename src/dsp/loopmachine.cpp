#include "dsp/loopmachine.h"
#include "config.h"
#include <cstring>
#include <algorithm>

LoopMachine::LoopMachine(int sample_rate)
    : sample_rate_(sample_rate),
      buffer_size_(static_cast<size_t>(sample_rate * LOOP_BUFFER_SECONDS)),
      buffer_(buffer_size_, 0.0f) {
}

LoopMachine::~LoopMachine() = default;

float LoopMachine::get_recorded_length() const {
    return static_cast<float>(recorded_samples_) / static_cast<float>(sample_rate_);
}

std::string LoopMachine::get_state_string() const {
    switch (state_) {
        case LoopState::IDLE:       return "IDLE";
        case LoopState::RECORDING:  return "REC";
        case LoopState::PLAYING:    return "PLAY";
        case LoopState::OVERDUB:    return "OVERDUB";
    }
    return "IDLE";
}

int LoopMachine::process(const float* mic_input, int mic_samples,
                         float* output, int output_samples) {
    int samples_to_process = std::min(mic_samples, output_samples);

    switch (state_) {
        case LoopState::IDLE:
            std::memcpy(output, mic_input, samples_to_process * sizeof(float));
            return samples_to_process;

        case LoopState::RECORDING: {
            for (int i = 0; i < samples_to_process; i++) {
                buffer_[write_pos_] = mic_input[i];
                write_pos_ = (write_pos_ + 1) % buffer_size_;
            }
            recorded_samples_ = std::min(recorded_samples_ + samples_to_process,
                                         static_cast<int>(buffer_size_));
            std::memset(output, 0, samples_to_process * sizeof(float));
            return samples_to_process;
        }

        case LoopState::PLAYING: {
            if (recorded_samples_ == 0) {
                std::memset(output, 0, samples_to_process * sizeof(float));
                return samples_to_process;
            }
            for (int i = 0; i < samples_to_process; i++) {
                output[i] = buffer_[read_pos_];
                read_pos_ = (read_pos_ + 1) % recorded_samples_;
            }
            return samples_to_process;
        }

        case LoopState::OVERDUB: {
            if (recorded_samples_ == 0) {
                for (int i = 0; i < samples_to_process; i++) {
                    buffer_[write_pos_] = mic_input[i];
                    write_pos_ = (write_pos_ + 1) % buffer_size_;
                }
                recorded_samples_ = std::min(recorded_samples_ + samples_to_process,
                                             static_cast<int>(buffer_size_));
                std::memset(output, 0, samples_to_process * sizeof(float));
                return samples_to_process;
            }
            float loop_mix = OVERDUB_MIX;
            float mic_mix = 1.0f - OVERDUB_MIX;
            for (int i = 0; i < samples_to_process; i++) {
                float loop_sample = buffer_[read_pos_];
                float mixed = loop_sample * loop_mix + mic_input[i] * mic_mix;
                buffer_[write_pos_] = mixed;
                output[i] = mixed;
                write_pos_ = (write_pos_ + 1) % buffer_size_;
                read_pos_ = (read_pos_ + 1) % recorded_samples_;
            }
            return samples_to_process;
        }
    }

    return 0;
}

void LoopMachine::toggle_recording() {
    if (state_ == LoopState::RECORDING) {
        state_ = LoopState::PLAYING;
        read_pos_ = 0;
    } else {
        state_ = LoopState::RECORDING;
        write_pos_ = 0;
        recorded_samples_ = 0;
    }
    empty_message_.clear();
}

void LoopMachine::toggle_playback() {
    if (!has_content()) {
        empty_message_ = "Buffer is empty";
        return;
    }

    if (state_ == LoopState::PLAYING) {
        state_ = LoopState::IDLE;
    } else {
        state_ = LoopState::PLAYING;
        read_pos_ = 0;
    }
    empty_message_.clear();
}

void LoopMachine::toggle_overdub() {
    if (!has_content()) {
        empty_message_ = "Buffer is empty";
        return;
    }

    if (state_ == LoopState::OVERDUB) {
        state_ = LoopState::PLAYING;
    } else {
        state_ = LoopState::OVERDUB;
    }
    empty_message_.clear();
}

void LoopMachine::clear() {
    std::fill(buffer_.begin(), buffer_.end(), 0.0f);
    write_pos_ = 0;
    read_pos_ = 0;
    recorded_samples_ = 0;
    state_ = LoopState::IDLE;
    empty_message_.clear();
}
