#pragma once

#include <cstddef>

class PitchShifter;

class Controls {
public:
    Controls(PitchShifter& shifter);
    ~Controls();

    void handle_key(int key, bool& running, bool& muted);

private:
    PitchShifter& shifter_;
};
