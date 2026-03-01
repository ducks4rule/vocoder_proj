#include "audio/alsa.h"
#include "utils/logger.h"
#include <cstring>
#include <vector>
#include <cstdint>
#include <cstdlib>

void list_alsa_devices() {
    void** hints;
    int err = snd_device_name_hint(-1, "pcm", &hints);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot get device hints: ") + snd_strerror(err));
        return;
    }
    LOG_INFO("=== Available ALSA PCM devices ===");
    for (void** h = hints; *h != nullptr; h++) {
        char* name = snd_device_name_get_hint(*h, "NAME");
        char* desc = snd_device_name_get_hint(*h, "DESC");
        if (name) {
            std::string info = "  - " + std::string(name);
            if (desc) {
                info += " : " + std::string(desc);
            }
            LOG_INFO(info);
            free(name);
        }
        if (desc) free(desc);
    }
    snd_device_name_free_hint(hints);
    LOG_INFO("=== End of ALSA devices ===");
}

namespace {
    bool configure_pcm_params(snd_pcm_t* pcm, int sample_rate, int channels, snd_pcm_uframes_t& buffer_size) {
        LOG_INFO("Configuring PCM params: sample_rate=" + std::to_string(sample_rate) + 
                 ", channels=" + std::to_string(channels) + 
                 ", format=S16");
        
        snd_pcm_hw_params_t* params;
        int err;

        snd_pcm_hw_params_malloc(&params);

        err = snd_pcm_hw_params_any(pcm, params);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot initialize hardware parameters: ") + snd_strerror(err));
            snd_pcm_hw_params_free(params);
            return false;
        }

        err = snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot set access type: ") + snd_strerror(err));
            snd_pcm_hw_params_free(params);
            return false;
        }

        err = snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_S16);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot set sample format: ") + snd_strerror(err));
            snd_pcm_hw_params_free(params);
            return false;
        }

        unsigned int rate = sample_rate;
        err = snd_pcm_hw_params_set_rate_near(pcm, params, &rate, 0);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot set sample rate: ") + snd_strerror(err));
            snd_pcm_hw_params_free(params);
            return false;
        }

        err = snd_pcm_hw_params_set_channels(pcm, params, channels);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot set channel count: ") + snd_strerror(err));
            snd_pcm_hw_params_free(params);
            return false;
        }

        snd_pcm_uframes_t requested_buffer_size = 4096;
        err = snd_pcm_hw_params_set_buffer_size_near(pcm, params, &requested_buffer_size);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot set buffer size: ") + snd_strerror(err));
            snd_pcm_hw_params_free(params);
            return false;
        }

        err = snd_pcm_hw_params(pcm, params);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot set hardware parameters: ") + snd_strerror(err));
            snd_pcm_hw_params_free(params);
            return false;
        }

        snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
        
        unsigned int actual_rate;
        snd_pcm_hw_params_get_rate(params, &actual_rate, 0);
        unsigned int actual_channels;
        snd_pcm_hw_params_get_channels(params, &actual_channels);
        
        LOG_INFO("PCM configured: actual_rate=" + std::to_string(actual_rate) + 
                 ", channels=" + std::to_string(actual_channels) +
                 ", buffer_size=" + std::to_string(buffer_size));
        
        snd_pcm_hw_params_free(params);

        err = snd_pcm_prepare(pcm);
        if (err < 0) {
            LOG_ERROR(std::string("Cannot prepare interface: ") + snd_strerror(err));
            return false;
        }

        return true;
    }
}

ALSADevice::ALSADevice() : pcm_capture_(nullptr), pcm_playback_(nullptr),
    sample_rate_(44100), channels_(1), buffer_size_(1024) {
}

ALSADevice::~ALSADevice() {
    close();
}

bool ALSADevice::open_capture(const char* device_name) {
    LOG_INFO(std::string("Attempting to open capture device: ") + device_name);
    int err = snd_pcm_open(&pcm_capture_, device_name, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot open capture device '") + device_name + "': " + snd_strerror(err));
        return false;
    }

    bool result = configure_pcm_params(pcm_capture_, sample_rate_, channels_, buffer_size_);
    if (result) {
        LOG_INFO(std::string("Capture device opened successfully: ") + device_name);
    }
    return result;
}

bool ALSADevice::open_playback(const char* device_name) {
    LOG_INFO(std::string("Attempting to open playback device: ") + device_name);
    int err = snd_pcm_open(&pcm_playback_, device_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (err < 0) {
        LOG_ERROR(std::string("Cannot open playback device '") + device_name + "': " + snd_strerror(err));
        return false;
    }

    snd_pcm_uframes_t playback_buffer_size;
    bool result = configure_pcm_params(pcm_playback_, sample_rate_, channels_, playback_buffer_size);
    if (result) {
        LOG_INFO(std::string("Playback device opened successfully: ") + device_name);
    }
    return result;
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

    std::vector<int16_t> int_buffer(frames);
    snd_pcm_sframes_t result = snd_pcm_readi(pcm_capture_, int_buffer.data(), frames);

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

    for (int i = 0; i < result; i++) {
        buffer[i] = static_cast<float>(int_buffer[i]) / 32768.0f;
    }

    return (int)result;
}

int ALSADevice::playback(const float* buffer, int frames) {
    if (!pcm_playback_) return 0;

    int wait_result = snd_pcm_wait(pcm_playback_, 100);
    if (wait_result < 0) {
        LOG_ERROR(std::string("snd_pcm_wait error: ") + snd_strerror(wait_result));
        snd_pcm_recover(pcm_playback_, wait_result, 0);
        return 0;
    }

    int err = snd_pcm_prepare(pcm_playback_);
    if (err < 0) {
        LOG_ERROR(std::string("snd_pcm_prepare error: ") + snd_strerror(err));
        return 0;
    }

    std::vector<int16_t> int_buffer(frames);
    for (int i = 0; i < frames; i++) {
        float sample = buffer[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        int_buffer[i] = static_cast<int16_t>(sample * 32767.0f);
    }

    snd_pcm_sframes_t result = snd_pcm_writei(pcm_playback_, int_buffer.data(), frames);

    if (result == -EINVAL) {
        LOG_ERROR("playback -EINVAL: frames=" + std::to_string(frames) + 
                  ", sample_rate=" + std::to_string(sample_rate_) + 
                  ", channels=" + std::to_string(channels_));
        return 0;
    }

    if (result < 0) {
        LOG_ERROR(std::string("playback error: ") + snd_strerror(result));
        result = snd_pcm_recover(pcm_playback_, (int)result, 0);
        if (result < 0) {
            LOG_ERROR(std::string("recovery failed: ") + snd_strerror(result));
            return 0;
        }
    }

    return (int)result;
}
