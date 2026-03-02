#pragma once

#include <cstddef>

class PitchShifter;
class LoopMachine;

class Controls {
public:
    Controls(PitchShifter& shifter, LoopMachine& loop);
    ~Controls();

    void handle_key(int key, bool& running, bool& muted);
    void set_detected_frequency(float freq);
    float get_detected_frequency() const { return detected_freq_; }
    int get_last_note() const { return last_note_semitone_; }

private:
    PitchShifter& shifter_;
    LoopMachine& loop_;
    float detected_freq_ = 0.0f;
    int last_note_semitone_ = -1;
};
