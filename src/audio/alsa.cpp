#include "audio/alsa.h"
#include <iostream>
#include <cstring>

ALSADevice::ALSADevice() : pcm_capture_(nullptr), pcm_playback_(nullptr),
    sample_rate_(44100), channels_(1), buffer_size_(1024) {
}

ALSADevice::~ALSADevice() {
    close();
}

bool ALSADevice::open_capture(const char* device_name) {
    int err;

    err = snd_pcm_open(&pcm_capture_, device_name, 
                       SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if (err < 0) {
        std::cerr << "Cannot open capture device '" << device_name << "': " 
                  << snd_strerror(err) << std::endl;
        return false;
    }

    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_malloc(&params);

    err = snd_pcm_hw_params_any(pcm_capture_, params);
    if (err < 0) {
        std::cerr << "Cannot initialize hardware parameters: " 
                  << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params_set_access(pcm_capture_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params_set_format(pcm_capture_, params, SND_PCM_FORMAT_FLOAT);
    if (err < 0) {
        std::cerr << "Cannot set sample format: " << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(params);
        return false;
    }

    unsigned int rate = sample_rate_;
    err = snd_pcm_hw_params_set_rate_near(pcm_capture_, params, &rate, 0);
    if (err < 0) {
        std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(params);
        return false;
    }
    sample_rate_ = rate;

    err = snd_pcm_hw_params_set_channels(pcm_capture_, params, channels_);
    if (err < 0) {
        std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params(pcm_capture_, params);
    if (err < 0) {
        std::cerr << "Cannot set hardware parameters: " << snd_strerror(err) << std::endl;
        snd_pcm_hw_params_free(params);
        return false;
    }

    snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t*)&buffer_size_);
    snd_pcm_hw_params_free(params);

    err = snd_pcm_prepare(pcm_capture_);
    if (err < 0) {
        std::cerr << "Cannot prepare capture interface: " << snd_strerror(err) << std::endl;
        return false;
    }

    return true;
}

bool ALSADevice::open_playback(const char* device_name) {
    (void)device_name;
    return true;
}

void ALSADevice::close() {
    if (pcm_capture_) {
        snd_pcm_drop(pcm_capture_);
        snd_pcm_close(pcm_capture_);
        pcm_capture_ = nullptr;
    }
    if (pcm_playback_) {
        snd_pcm_drop(pcm_playback_);
        snd_pcm_close(pcm_playback_);
        pcm_playback_ = nullptr;
    }
}

int ALSADevice::capture(float* buffer, int frames) {
    if (!pcm_capture_) return 0;

    snd_pcm_sframes_t result = snd_pcm_readi(pcm_capture_, buffer, frames);

    if (result == -EAGAIN) {
        return 0;
    } else if (result < 0) {
        std::cerr << "Capture error: " << snd_strerror(result) << std::endl;
        result = snd_pcm_recover(pcm_capture_, (int)result, 0);
        if (result < 0) {
            std::cerr << "Cannot recover from capture error: " << snd_strerror(result) << std::endl;
            return 0;
        }
        return 0;
    }

    return (int)result;
}

int ALSADevice::playback(const float* buffer, int frames) {
    (void)buffer;
    (void)frames;
    return 0;
}
