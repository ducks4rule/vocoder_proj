#include "controls.h"
#include "dsp/pitchshift.h"
#include "utils/logger.h"
#include <cmath>
#include <cstdint>

namespace {
    constexpr float SEMITONE_RATIO = 1.059463094359f;  // 2^(1/12)

    constexpr float NOTE_FREQUENCIES[] = {
        261.63f,  // A = C4 = 0
        277.18f,  // W = C#4 = 1
        293.66f,  // S = D4 = 2
        311.13f,  // E = D#4 = 3
        329.63f,  // D = E4 = 4
        349.23f,  // F = F4 = 5
        369.99f,  // T = F#4 = 6
        392.00f,  // G = G4 = 7
        415.30f,  // Y = G#4 = 8
        440.00f,  // H = A4 = 9
        466.16f,  // U = A#4 = 10
        493.88f,  // J = B4 = 11
        523.25f   // K = C5 = 12
    };
}

Controls::Controls(PitchShifter& shifter)
    : shifter_(shifter) {
}

Controls::~Controls() = default;

void Controls::set_detected_frequency(float freq) {
    detected_freq_ = freq;
}

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
            last_note_semitone_ = -1;
            break;

        case 'a':
        case 'A':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                float ratio = NOTE_FREQUENCIES[0] / detected_freq_;
                shifter_.set_pitch_ratio(ratio);
            }
            last_note_semitone_ = 0;
            break;

        case 'w':
        case 'W':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[1] / detected_freq_);
            }
            last_note_semitone_ = 1;
            break;

        case 's':
        case 'S':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[2] / detected_freq_);
            }
            last_note_semitone_ = 2;
            break;

        case 'e':
        case 'E':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[3] / detected_freq_);
            }
            last_note_semitone_ = 3;
            break;

        case 'd':
        case 'D':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[4] / detected_freq_);
            }
            last_note_semitone_ = 4;
            break;

        case 'f':
        case 'F':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[5] / detected_freq_);
            }
            last_note_semitone_ = 5;
            break;

        case 't':
        case 'T':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[6] / detected_freq_);
            }
            last_note_semitone_ = 6;
            break;

        case 'g':
        case 'G':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[7] / detected_freq_);
            }
            last_note_semitone_ = 7;
            break;

        case 'y':
        case 'Y':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[8] / detected_freq_);
            }
            last_note_semitone_ = 8;
            break;

        case 'h':
        case 'H':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[9] / detected_freq_);
            }
            last_note_semitone_ = 9;
            break;

        case 'u':
        case 'U':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[10] / detected_freq_);
            }
            last_note_semitone_ = 10;
            break;

        case 'j':
        case 'J':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[11] / detected_freq_);
            }
            last_note_semitone_ = 11;
            break;

        case 'k':
        case 'K':
            if (detected_freq_ > 50.0f && detected_freq_ < 5000.0f) {
                shifter_.set_pitch_ratio(NOTE_FREQUENCIES[12] / detected_freq_);
            }
            last_note_semitone_ = 12;
            break;

        default:
            break;
    }
}
