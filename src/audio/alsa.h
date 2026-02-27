#pragma once

#include <cstdint>
#include <cstddef>
#include <alsa/asoundlib.h>

class ALSADevice {
public:
    ALSADevice();
    ~ALSADevice();

    bool open_capture(const char* device_name = "default");
    bool open_playback(const char* device_name = "default");
    void close();

    int capture(float* buffer, int frames);
    int playback(const float* buffer, int frames);

    int get_sample_rate() const { return sample_rate_; }
    int get_channels() const { return channels_; }
    int get_buffer_size() const { return static_cast<int>(buffer_size_); }

private:
    snd_pcm_t* pcm_capture_;
    snd_pcm_t* pcm_playback_;
    int sample_rate_;
    int channels_;
    snd_pcm_uframes_t buffer_size_;
};
