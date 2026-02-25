#include <iostream>
#include <csignal>
#include <atomic>
#include "audio/alsa.h"
#include "dsp/pitchshift.h"
#include "ui/tui.h"

namespace {
    std::atomic<bool> running{true};
}

void signal_handler(int signal) {
    (void)signal;
    running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "Vocoder-TUI v1.0.0" << std::endl;
    std::cout << "Press 'q' to quit" << std::endl;

    ALSADevice audio;
    if (!audio.open_capture()) {
        std::cerr << "Failed to open capture device" << std::endl;
        return 1;
    }
    std::cout << "Capture device opened" << std::endl;

    if (!audio.open_playback()) {
        std::cerr << "Failed to open playback device" << std::endl;
        return 1;
    }
    std::cout << "Playback device opened" << std::endl;

    PitchShifter shifter(4096, 1024, 44100);
    TUI ui;
    ui.init();

    const int buffer_frames = 1024;
    float input_buffer[buffer_frames];
    float output_buffer[buffer_frames];

    int loop_count = 0;
    while (running) {
        int captured = audio.capture(input_buffer, buffer_frames);
        if (captured > 0) {
            if (loop_count % 100 == 0) {
                std::cout << "Captured: " << captured << " frames" << std::endl;
            }
            shifter.process(input_buffer, output_buffer, captured);
            int played = audio.playback(output_buffer, captured);
            if (loop_count % 100 == 0) {
                std::cout << "Played: " << played << " frames" << std::endl;
            }
        }

        int key = ui.get_key_input();
        if (key == 'q' || key == 'Q') {
            std::cout << "Quit key pressed" << std::endl;
            running = false;
        }

        AudioStats stats;
        stats.input_level = 0.0f;
        stats.pitch_ratio = shifter.get_pitch_ratio();
        stats.pitch_semitones = 0;
        stats.spectrum.resize(64);
        stats.muted = false;
        ui.render(stats);
        
        loop_count++;
    }

    ui.shutdown();
    audio.close();

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
