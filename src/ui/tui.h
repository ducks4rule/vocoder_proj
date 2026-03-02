#pragma once

#include <string>
#include <vector>

struct AudioStats {
    float input_level;
    float output_level;
    float pitch_ratio;
    int pitch_semitones;
    std::vector<float> spectrum;
    std::vector<float> output_spectrum;
    bool muted;
    float volume;
    float detected_freq;
    int active_note;

    // Loop Machine
    std::string loop_state;
    int loop_recorded_samples;
    int loop_max_samples;
    std::string loop_message;
};

class TUI {
public:
    TUI();
    ~TUI();

    void init();
    void shutdown();
    void render(const AudioStats& stats);
    int get_key_input();
    void draw_spectrum_boxed(const char* label, int row, int col, const float* spectrum, size_t num_bins);

private:
    bool initialized_;
    int width_;
    int height_;
    float smoothed_input_;
    float smoothed_output_;
};
