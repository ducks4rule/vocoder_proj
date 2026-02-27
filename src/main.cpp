#include <iostream>
#include <csignal>
#include <atomic>
#include <cstring>
#include "audio/alsa.h"
#include "dsp/pitchshift.h"
#include "dsp/utils.h"
#include "ui/tui.h"
#include "utils/logger.h"
#include "config.h"

namespace {
    std::atomic<bool> running{true};
}

void signal_handler(int signal) {
    (void)signal;
    running = false;
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

    PitchShifter shifter(FFT_SIZE, HOP_SIZE, SAMPLE_RATE);
    TUI ui;
    ui.init();

    const int buffer_frames = BUFFER_FRAMES;
    const size_t spectrum_bins = FFT_SIZE / 2 + 1;
    float input_buffer[buffer_frames];
    float output_buffer[buffer_frames];

    bool muted = false;

    while (running) {
        memset(input_buffer, 0, sizeof(input_buffer));
        memset(output_buffer, 0, sizeof(output_buffer));

        int captured = audio.capture(input_buffer, buffer_frames);
        if (captured > 0) {

            shifter.process(input_buffer, output_buffer, captured);

            if (muted) {
                for (int i = 0; i < captured; i++) {
                    output_buffer[i] = 0.0f;
                }
            }

            audio.playback(output_buffer, captured);

            AudioStats stats;
            float input_db = calculate_db(input_buffer, captured);
            float output_db = calculate_db(output_buffer, captured);
            stats.input_level = input_db;
            stats.output_level = output_db;
            stats.pitch_ratio = shifter.get_pitch_ratio();
            stats.pitch_semitones = 0;
            stats.spectrum.resize(spectrum_bins);
            shifter.get_spectrum(stats.spectrum.data(), stats.spectrum.size());
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

    }

    ui.shutdown();
    audio.close();

    LOG_INFO("Goodbye!");
    return 0;
}
