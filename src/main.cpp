#include <iostream>
#include <csignal>
#include <atomic>
#include <cmath>
#include <cstring>
#include "audio/alsa.h"
#include "dsp/pitchshift.h"
#include "ui/tui.h"
#include "utils/logger.h"

namespace {
    std::atomic<bool> running{true};
}

void signal_handler(int signal) {
    (void)signal;
    running = false;
}

float calculate_db(const float* buffer, int frames) {
    if (frames <= 0) return -60.0f;
    float sum = 0.0f;
    for (int i = 0; i < frames; i++) {
        sum += buffer[i] * buffer[i];
    }
    float rms = std::sqrt(sum / frames);
    if (rms > 0.0f) {
        float db = 20.0f * std::log10(rms);
        return std::max(-60.0f, std::min(db, 0.0f));
    }
    return -60.0f;
}

int main() {
    Logger::instance().set_file("/tmp/vocoder-tui.log");
    Logger::instance().set_level(LogLevel::INFO);

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    LOG_INFO("Vocoder-TUI v1.0.0 starting...");
    LOG_INFO("Press 'q' to quit");

    ALSADevice audio;
    if (!audio.open_capture()) {
        LOG_ERROR("Failed to open capture device");
        return 1;
    }
    LOG_INFO("Capture device opened");

    if (!audio.open_playback()) {
        LOG_ERROR("Failed to open playback device");
        return 1;
    }
    LOG_INFO("Playback device opened");

    PitchShifter shifter(4096, 1024, 44100);
    TUI ui;
    ui.init();

    const int buffer_frames = 1024;
    float input_buffer[buffer_frames];
    float output_buffer[buffer_frames];

    bool muted = false;

    int loop_count = 0;
    while (running) {
        memset(input_buffer, 0, sizeof(input_buffer));
        memset(output_buffer, 0, sizeof(output_buffer));

        int captured = audio.capture(input_buffer, buffer_frames);
        if (captured > 0) {

            if (!muted) {
                shifter.process(input_buffer, output_buffer, captured);
            } else {
                for (int i = 0; i < captured; i++) {
                    output_buffer[i] = 0.0f;
                }
            }

            int played = audio.playback(output_buffer, captured);

            AudioStats stats;
            float input_db = calculate_db(input_buffer, captured);
            float output_db = calculate_db(output_buffer, captured);
            stats.input_level = input_db;
            stats.output_level = output_db;
            stats.pitch_ratio = shifter.get_pitch_ratio();
            stats.pitch_semitones = 0;
            stats.spectrum.resize(64);
            stats.muted = muted;
            stats.volume = shifter.get_volume();
            ui.render(stats);
        }

        int key = ui.get_key_input();
        if (key == 'q' || key == 'Q') {
            LOG_INFO("Quit key pressed");
            running = false;
        } else if (key == 'm' || key == 'M') {
            muted = !muted;
            LOG_INFO(std::string("Mute: ") + (muted ? "ON" : "OFF"));
        } else if (key == ']') {
            float v = shifter.get_volume();
            shifter.set_volume(std::min(v + 0.05f, 1.0f));
        } else if (key == '[') {
            float v = shifter.get_volume();
            shifter.set_volume(std::max(v - 0.05f, 0.0f));
        }

        loop_count++;
    }

    ui.shutdown();
    audio.close();

    LOG_INFO("Goodbye!");
    return 0;
}
