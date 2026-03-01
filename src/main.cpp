#include <iostream>
#include <csignal>
#include <atomic>
#include <cstring>
#include <cmath>
#include <vector>
#include "audio/alsa.h"
#include "dsp/stft.h"
#include "dsp/pitchshift.h"
#include "dsp/pitchdetect.h"
#include "dsp/utils.h"
#include "ui/tui.h"
#include "utils/logger.h"
#include "config.h"
#include "controls.h"

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

    list_alsa_devices();

    ALSADevice audio;
    if (!audio.open_capture("plug:default")) {
        LOG_ERROR("Failed to open capture device");
        return 1;
    }
    LOG_INFO("Capture device opened");

    if (!audio.open_playback("plug:default")) {
        LOG_ERROR("Failed to open playback device");
        return 1;
    }
    LOG_INFO("Playback device opened");

    PitchShifter shifter(FFT_SIZE, HOP_SIZE, SAMPLE_RATE);
    STFTProcessor stft(FFT_SIZE, HOP_SIZE, SAMPLE_RATE);
    PitchDetector pitch_detector(BUFFER_FRAMES, SAMPLE_RATE);
    TUI ui;
    ui.init();
    Controls controls(shifter);

    std::vector<float> input_buffer(BUFFER_FRAMES);
    std::vector<float> output_buffer(BUFFER_FRAMES);
    std::vector<float> freq_real(FFT_SIZE / 2 + 1);
    std::vector<float> freq_imag(FFT_SIZE / 2 + 1);
    const size_t spectrum_bins = FFT_SIZE / 2 + 1;

    bool muted = false;

    while (running) {
        std::fill(input_buffer.begin(), input_buffer.end(), 0.0f);
        std::fill(output_buffer.begin(), output_buffer.end(), 0.0f);

        int captured = audio.capture(input_buffer.data(), BUFFER_FRAMES);
        if (captured > 0) {
            float detected_freq = pitch_detector.detect(input_buffer.data(), captured);
            controls.set_detected_frequency(detected_freq);
            
            stft.forward(input_buffer.data(), captured, freq_real.data(), freq_imag.data());
            shifter.process(freq_real.data(), freq_imag.data(), freq_real.size());
            stft.inverse(freq_real.data(), freq_imag.data(), output_buffer.data(), captured);

            if (muted) {
                for (int i = 0; i < captured; i++) {
                    output_buffer[i] = 0.0f;
                }
                for (size_t i = 0; i < freq_real.size(); i++) {
                    freq_real[i] = 0.0f;
                    freq_imag[i] = 0.0f;
                }
            }

            audio.playback(output_buffer.data(), captured);

            AudioStats stats;
            float input_db = calculate_db(input_buffer.data(), captured);
            float output_db = calculate_db(output_buffer.data(), captured);
            stats.input_level = input_db;
            stats.output_level = output_db;
            stats.pitch_ratio = shifter.get_pitch_ratio();
            stats.pitch_semitones = static_cast<int>(12.0f * std::log2(stats.pitch_ratio));
            stats.spectrum.resize(spectrum_bins);
            stft.get_spectrum(stats.spectrum.data(), stats.spectrum.size());
            stats.output_spectrum.resize(spectrum_bins);
            calculate_spectrum_db(freq_real.data(), freq_imag.data(), 
                                 stats.output_spectrum.data(), stats.output_spectrum.size());
            stats.muted = muted;
            stats.volume = shifter.get_volume();
            stats.detected_freq = controls.get_detected_frequency();
            stats.active_note = controls.get_last_note();
            ui.render(stats);
        }

        int key = ui.get_key_input();
        if (key != 0) {
            bool running_copy = running.load();
            controls.handle_key(key, running_copy, muted);
            if (!running_copy) {
                running.store(false);
            }
        }

    }

    ui.shutdown();
    audio.close();

    LOG_INFO("Goodbye!");
    return 0;
}
