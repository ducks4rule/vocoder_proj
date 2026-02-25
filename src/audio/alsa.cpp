#include "audio/alsa.h"

ALSADevice::ALSADevice() : pcm_capture_(nullptr), pcm_playback_(nullptr),
    sample_rate_(44100), channels_(1), buffer_size_(1024) {
}

ALSADevice::~ALSADevice() {
    close();
}

bool ALSADevice::open_capture(const char* device_name) {
    (void)device_name;
    return true;
}

bool ALSADevice::open_playback(const char* device_name) {
    (void)device_name;
    return true;
}

void ALSADevice::close() {
    pcm_capture_ = nullptr;
    pcm_playback_ = nullptr;
}

int ALSADevice::capture(float* buffer, int frames) {
    (void)buffer;
    (void)frames;
    return 0;
}

int ALSADevice::playback(const float* buffer, int frames) {
    (void)buffer;
    (void)frames;
    return 0;
}
