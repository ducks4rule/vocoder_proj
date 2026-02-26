#include "audio/alsa.h"
#include "utils/logger.h"
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
        LOG_ERROR(std::string("Cannot open capture device '") + device_name + "': " + snd_strerror(err));
        return false;
    }

    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_malloc(&params);

    err = snd_pcm_hw_params_any(pcm_capture_, params);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot initialize hardware parameters: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params_set_access(pcm_capture_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set access type: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params_set_format(pcm_capture_, params, SND_PCM_FORMAT_FLOAT);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set sample format: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    unsigned int rate = sample_rate_;
    err = snd_pcm_hw_params_set_rate_near(pcm_capture_, params, &rate, 0);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set sample rate: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }
    sample_rate_ = rate;

    err = snd_pcm_hw_params_set_channels(pcm_capture_, params, channels_);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set channel count: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params(pcm_capture_, params);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set hardware parameters: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t*)&buffer_size_);
    snd_pcm_hw_params_free(params);

    err = snd_pcm_prepare(pcm_capture_);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot prepare capture interface: ") + snd_strerror(err));
        return false;
    }

    return true;
}

bool ALSADevice::open_playback(const char* device_name) {
    int err;

    err = snd_pcm_open(&pcm_playback_, device_name, 
                       SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot open playback device '") + device_name + "': " + snd_strerror(err));
        return false;
    }

    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_malloc(&params);

    err = snd_pcm_hw_params_any(pcm_playback_, params);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot initialize playback parameters: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params_set_access(pcm_playback_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set playback access type: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params_set_format(pcm_playback_, params, SND_PCM_FORMAT_FLOAT);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set playback sample format: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    unsigned int rate = sample_rate_;
    err = snd_pcm_hw_params_set_rate_near(pcm_playback_, params, &rate, 0);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set playback sample rate: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params_set_channels(pcm_playback_, params, channels_);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set playback channel count: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    err = snd_pcm_hw_params(pcm_playback_, params);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot set playback hardware parameters: ") + snd_strerror(err));
        snd_pcm_hw_params_free(params);
        return false;
    }

    snd_pcm_uframes_t playback_buffer_size;
    snd_pcm_hw_params_get_buffer_size(params, &playback_buffer_size);
    snd_pcm_hw_params_free(params);

    err = snd_pcm_prepare(pcm_playback_);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot prepare playback interface: ") + snd_strerror(err));
        return false;
    }

    err = snd_pcm_start(pcm_playback_);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot start playback interface: ") + snd_strerror(err));
        return false;
    }

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
        LOG_ERROR(std::string("capture error: ") + snd_strerror(result));
        result = snd_pcm_recover(pcm_capture_, (int)result, 0);
        if (result < 0) {
            LOG_ERROR(std::string("capture recovery failed: ") + snd_strerror(result));
            return 0;
        }
        return 0;
    }

    return (int)result;
}

int ALSADevice::playback(const float* buffer, int frames) {
    if (!pcm_playback_) return 0;

    snd_pcm_sframes_t result = snd_pcm_writei(pcm_playback_, buffer, frames);

    if (result == -EAGAIN) {
        usleep(1000);
        result = snd_pcm_writei(pcm_playback_, buffer, frames);
        if (result == -EAGAIN) {
            return 0;
        }
    }
    
    if (result < 0) {
        LOG_ERROR(std::string("playback error: ") + snd_strerror(result));
        result = snd_pcm_recover(pcm_playback_, (int)result, 0);
        if (result < 0) {
            LOG_ERROR(std::string("recovery failed: ") + snd_strerror(result));
            return 0;
        }
        return 0;
    }

    return (int)result;
}
