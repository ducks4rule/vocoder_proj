#pragma once

#include <string>
#include <vector>

struct AudioStats {
    float input_level;
    float pitch_ratio;
    int pitch_semitones;
    std::vector<float> spectrum;
    bool muted;
};

class TUI {
public:
    TUI();
    ~TUI();

    void init();
    void shutdown();
    void render(const AudioStats& stats);
    int get_key_input();

private:
    bool initialized_;
    int width_;
    int height_;
};
