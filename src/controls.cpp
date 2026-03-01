#include "controls.h"
#include "dsp/pitchshift.h"
#include "utils/logger.h"
#include <cmath>
#include <cstdint>

namespace {
    constexpr float SEMITONE_RATIO = 1.059463094359f;  // 2^(1/12)

    float semitones_to_ratio(int semitones) {
        return std::pow(2.0f, static_cast<float>(semitones) / 12.0f);
    }
}

Controls::Controls(PitchShifter& shifter)
    : shifter_(shifter) {
}

Controls::~Controls() = default;

void Controls::handle_key(int key, bool& running, bool& muted) {
    switch (key) {
        case 'q':
        case 'Q':
            LOG_INFO("Quit key pressed");
            running = false;
            break;

        case 'm':
        case 'M':
            muted = !muted;
            LOG_INFO(std::string("Mute: ") + (muted ? "ON" : "OFF"));
            break;

        case ']': {
            float v = shifter_.get_volume();
            shifter_.set_volume(std::min(v + 0.05f, 1.0f));
            break;
        }

        case '[': {
            float v = shifter_.get_volume();
            shifter_.set_volume(std::max(v - 0.05f, 0.0f));
            break;
        }

        case '+':
            shifter_.set_pitch_ratio(shifter_.get_pitch_ratio() + 0.1f);
            break;

        case '-':
            shifter_.set_pitch_ratio(shifter_.get_pitch_ratio() - 0.1f);
            break;

        case '=':
            shifter_.set_pitch_ratio(shifter_.get_pitch_ratio() + 0.01f);
            break;

        case '_':
            shifter_.set_pitch_ratio(shifter_.get_pitch_ratio() - 0.01f);
            break;

        case 'r':
        case 'R':
            shifter_.set_pitch_ratio(1.0f);
            break;

        case 'a':
        case 'A':
            shifter_.set_pitch_ratio(semitones_to_ratio(0));   // C4
            break;

        case 'w':
        case 'W':
            shifter_.set_pitch_ratio(semitones_to_ratio(1));   // C#4
            break;

        case 's':
        case 'S':
            shifter_.set_pitch_ratio(semitones_to_ratio(2));   // D4
            break;

        case 'e':
        case 'E':
            shifter_.set_pitch_ratio(semitones_to_ratio(3));   // D#4
            break;

        case 'd':
        case 'D':
            shifter_.set_pitch_ratio(semitones_to_ratio(4));   // E4
            break;

        case 'f':
        case 'F':
            shifter_.set_pitch_ratio(semitones_to_ratio(5));   // F4
            break;

        case 't':
        case 'T':
            shifter_.set_pitch_ratio(semitones_to_ratio(6));   // F#4
            break;

        case 'g':
        case 'G':
            shifter_.set_pitch_ratio(semitones_to_ratio(7));   // G4
            break;

        case 'y':
        case 'Y':
            shifter_.set_pitch_ratio(semitones_to_ratio(8));   // G#4
            break;

        case 'h':
        case 'H':
            shifter_.set_pitch_ratio(semitones_to_ratio(9));   // A4
            break;

        case 'u':
        case 'U':
            shifter_.set_pitch_ratio(semitones_to_ratio(10));  // A#4
            break;

        case 'j':
        case 'J':
            shifter_.set_pitch_ratio(semitones_to_ratio(11));  // B4
            break;

        case 'k':
        case 'K':
            shifter_.set_pitch_ratio(semitones_to_ratio(12));  // C5
            break;

        default:
            break;
    }
}
