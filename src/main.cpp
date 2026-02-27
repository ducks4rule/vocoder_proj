#include <iostream>
#include <csignal>
#include <atomic>
#include <cstring>
#include <vector>
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

    std::vector<float> input_buffer(BUFFER_FRAMES);
    std::vector<float> output_buffer(BUFFER_FRAMES);
    const size_t spectrum_bins = FFT_SIZE / 2 + 1;

    bool muted = false;

    while (running) {
        std::fill(input_buffer.begin(), input_buffer.end(), 0.0f);
        std::fill(output_buffer.begin(), output_buffer.end(), 0.0f);

        int captured = audio.capture(input_buffer.data(), BUFFER_FRAMES);
        if (captured > 0) {

            shifter.process(input_buffer.data(), output_buffer.data(), captured);

            if (muted) {
                for (int i = 0; i < captured; i++) {
                    output_buffer[i] = 0.0f;
                }
            }

            audio.playback(output_buffer.data(), captured);

            AudioStats stats;
            float input_db = calculate_db(input_buffer.data(), captured);
            float output_db = calculate_db(output_buffer.data(), captured);
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
