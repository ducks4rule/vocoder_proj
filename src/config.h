#ifndef VOCODER_CONFIG_H
#define VOCODER_CONFIG_H

// Audio
constexpr int SAMPLE_RATE = 44100;
constexpr int FFT_SIZE = 4096;
constexpr int HOP_SIZE = 1024;
constexpr int BUFFER_FRAMES = 1024;
constexpr float SPECTRUM_MIN_DB = -30.0f;
constexpr float SPECTRUM_MAX_DB = 0.0f;
constexpr int SPECTRUM_BARS = 32;
constexpr int SPECTRUM_HEIGHT = 10;
constexpr float SPECTRUM_MIN_FREQ = 20.0f;
constexpr float SPECTRUM_MAX_FREQ = 20000.0f;

// Level meters
constexpr int METER_WIDTH = 20;
constexpr float METER_MIN_DB = -60.0f;
constexpr float METER_MAX_DB = 0.0f;
constexpr float METER_YELLOW_DB = -12.0f;
constexpr float METER_RED_DB = -3.0f;

// Master meter
constexpr int MASTER_HEIGHT = 10;

#endif
