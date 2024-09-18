// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        const audio::DeviceID& Player::getAudioDevice() const
        {
            return _p->audioDevice->get();
        }

        std::shared_ptr<observer::IValue<audio::DeviceID> > Player::observeAudioDevice() const
        {
            return _p->audioDevice;
        }

        void Player::setAudioDevice(const audio::DeviceID& value)
        {
            TLRENDER_P();
            if (p.audioDevice->setIfChanged(value))
            {
                if (auto context = getContext().lock())
                {
                    p.audioInit(context);
                }
            }
        }

        float Player::getVolume() const
        {
            return _p->volume->get();
        }

        std::shared_ptr<observer::IValue<float> > Player::observeVolume() const
        {
            return _p->volume;
        }

        void Player::setVolume(float value)
        {
            TLRENDER_P();
            if (p.volume->setIfChanged(math::clamp(value, 0.F, 1.F)))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.volume = value;
            }
        }

        bool Player::isMuted() const
        {
            return _p->mute->get();
        }

        std::shared_ptr<observer::IValue<bool> > Player::observeMute() const
        {
            return _p->mute;
        }

        void Player::setMute(bool value)
        {
            TLRENDER_P();
            if (p.mute->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.mute = value;
            }
        }

        double Player::getAudioOffset() const
        {
            return _p->audioOffset->get();
        }

        std::shared_ptr<observer::IValue<double> > Player::observeAudioOffset() const
        {
            return _p->audioOffset;
        }

        void Player::setAudioOffset(double value)
        {
            TLRENDER_P();
            if (p.audioOffset->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.audioOffset = value;
            }
        }

        const std::vector<AudioData>& Player::getCurrentAudio() const
        {
            return _p->currentAudioData->get();
        }

        std::shared_ptr<observer::IList<AudioData> > Player::observeCurrentAudio() const
        {
            return _p->currentAudioData;
        }

        bool Player::Private::hasAudio() const
        {
            bool out = ioInfo.audio.isValid();
#if !defined(TLRENDER_AUDIO)
            out = false;
#endif
            return out;
        }

        namespace
        {
#if defined(TLRENDER_AUDIO)
            RtAudioFormat toRtAudio(audio::DataType value) noexcept
            {
                RtAudioFormat out = 0;
                switch (value)
                {
                case audio::DataType::S16: out = RTAUDIO_SINT16; break;
                case audio::DataType::S32: out = RTAUDIO_SINT32; break;
                case audio::DataType::F32: out = RTAUDIO_FLOAT32; break;
                case audio::DataType::F64: out = RTAUDIO_FLOAT64; break;
                default: break;
                }
                return out;
            }
#endif // TLRENDER_AUDIO
        }

        void Player::Private::audioInit(const std::shared_ptr<system::Context>& context)
        {
#if defined(TLRENDER_AUDIO)
            if (rtAudio && rtAudio->isStreamOpen())
            {
                try
                {
                    rtAudio->abortStream();
                    rtAudio->closeStream();
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }
            }
            try
            {
                RtAudio::Api rtApi = RtAudio::Api::UNSPECIFIED;
#if defined(__linux__)
                rtApi = RtAudio::Api::LINUX_ALSA;
#endif // __linux__
                rtAudio.reset(new RtAudio(rtApi));
                rtAudio->showWarnings(false);
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot create RtAudio instance: " << e.what();
                context->log("tl::timeline::Player", ss.str(), log::Type::Error);
            }

            if (rtAudio)
            {
                audio::DeviceID id = audioDevice->get();
                auto audioSystem = context->getSystem<audio::System>();
                if (audio::DeviceID() == id)
                {
                    id = audioSystem->getDefaultOutputDevice();
                }
                {
                    std::stringstream ss;
                    ss << "Opening audio device: " << id.number << " " << id.name;
                    context->log("tl::timeline::Player", ss.str());
                }
                const auto devices = audioSystem->getDevices();
                const auto i = std::find_if(
                    devices.begin(),
                    devices.end(),
                    [id](const audio::DeviceInfo& value)
                    {
                        return id == value.id;
                    });
                audioInfo = audio::Info();
                if (i != devices.end())
                {
                    audioInfo = i->outputInfo;
                }
                audioInfo.channelCount = getAudioChannelCount(
                    ioInfo.audio,
                    audioInfo);
                if (audioInfo.channelCount > 0 &&
                    audioInfo.dataType != audio::DataType::None &&
                    audioInfo.sampleRate > 0)
                {
                    // These are OK to modify since the audio thread is stopped.
                    audioMutex.reset = true;
                    audioThread.info = audioInfo;
                    audioThread.resample.reset();

                    try
                    {
                        RtAudio::StreamParameters rtParameters;
                        rtParameters.deviceId = id.number;
                        rtParameters.nChannels = audioInfo.channelCount;
                        unsigned int rtBufferFrames = playerOptions.audioBufferFrameCount;
                        rtAudio->openStream(
                            &rtParameters,
                            nullptr,
                            toRtAudio(audioInfo.dataType),
                            audioInfo.sampleRate,
                            &rtBufferFrames,
                            rtAudioCallback,
                            this,
                            nullptr,
                            rtAudioErrorCallback);
                        rtAudio->startStream();
                    }
                    catch (const std::exception& e)
                    {
                        std::stringstream ss;
                        ss << "Cannot open audio stream: " << e.what();
                        context->log("tl::timeline::Player", ss.str(), log::Type::Error);
                    }
                }
            }
#endif // TLRENDER_AUDIO
        }

        void Player::Private::audioReset(const otime::RationalTime& time)
        {
            audioMutex.reset = true;
            audioMutex.start = (time - timeline->getTimeRange().start_time()).rescaled_to(ioInfo.audio.sampleRate).value();
            audioMutex.frame = 0;
        }

        size_t Player::Private::getAudioChannelCount(
            const audio::Info& input,
            const audio::Info& output)
        {
            size_t out = 2;
            if (input.channelCount == output.channelCount)
            {
                out = output.channelCount;
            }
            return out;
        }

#if defined(TLRENDER_AUDIO)
        int Player::Private::rtAudioCallback(
            void* outputBuffer,
            void* inputBuffer,
            unsigned int nFrames,
            double streamTime,
            RtAudioStreamStatus status,
            void* userData)
        {
            auto p = reinterpret_cast<Player::Private*>(userData);
            
            // Get mutex protected values.
            Playback playback = Playback::Stop;
            otime::RationalTime currentTime = time::invalidTime;
            double audioOffset = 0.0;
            {
                std::unique_lock<std::mutex> lock(p->mutex.mutex);
                playback = p->mutex.playback;
                currentTime = p->mutex.currentTime;
                audioOffset = p->mutex.audioOffset;
            }
            double speed = 0.0;
            float volume = 1.F;
            bool mute = false;
            std::chrono::steady_clock::time_point muteTimeout;
            bool reset = false;
            int64_t start = 0;
            int64_t frame = 0;
            {
                std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                speed = p->audioMutex.speed;
                volume = p->audioMutex.volume;
                mute = p->audioMutex.mute;
                muteTimeout = p->audioMutex.muteTimeout;
                reset = p->audioMutex.reset;
                p->audioMutex.reset = false;
                start = p->audioMutex.start;
                frame = p->audioMutex.frame;
            }
            //std::cout << "playback: " << playback << std::endl;
            //std::cout << "playbackStartTime: " << playbackStartTime << std::endl;
            //std::cout << "reset: " << reset << std::endl;

            // Check if the timers should be initialized.
            const audio::Info& inputInfo = p->ioInfo.audio;
            if (playback != p->audioThread.playback ||
                speed != p->audioThread.speed ||
                reset)
            {
                p->audioThread.playback = playback;
                p->audioThread.speed = speed;
            }

            // Zero output audio data.
            const audio::Info& outputInfo = p->audioThread.info;
            std::memset(outputBuffer, 0, nFrames * outputInfo.getByteCount());

            if (playback != Playback::Stop && inputInfo.sampleRate > 0)
            {
                // Flush the audio resampler and buffer when the RtAudio
                // playback is reset.
                if (reset)
                {
                    if (p->audioThread.resample)
                    {
                        p->audioThread.resample->flush();
                    }
                    p->audioThread.buffer.clear();
                }

                // Create the audio resampler.
                if (!p->audioThread.resample ||
                    (p->audioThread.resample && p->audioThread.resample->getInputInfo() != p->ioInfo.audio))
                {
                    p->audioThread.resample = audio::AudioResample::create(
                        p->ioInfo.audio,
                        p->audioThread.info);
                }

                // Get audio from the cache.
                const int64_t bufferSize = getSampleCount(p->audioThread.buffer);
                int64_t size = otio::RationalTime(
                    nFrames * 2 - bufferSize,
                    outputInfo.sampleRate).
                    rescaled_to(inputInfo.sampleRate).value();
                int64_t t = start;
                if (Playback::Forward == playback)
                {
                    t += frame;
                }
                else
                {
                    t -= frame;
                }
                int64_t seconds = t / inputInfo.sampleRate;
                int64_t offset = t - (seconds * inputInfo.sampleRate);
                if (Playback::Forward == playback)
                {
                    size = std::min(
                        size,
                        static_cast<int64_t>(inputInfo.sampleRate) - offset);
                }
                else
                {
                    const int64_t tmp = t;
                    t -= size;
                    if (t < (seconds * inputInfo.sampleRate))
                    {
                        if (tmp == (seconds * inputInfo.sampleRate))
                        {
                            --seconds;
                            offset = t - (seconds * inputInfo.sampleRate);
                        }
                        else
                        {
                            size = tmp - (seconds * inputInfo.sampleRate);
                            offset = 0;
                        }
                    }
                    else
                    {
                        offset = t - (seconds * inputInfo.sampleRate);
                    }
                }
                AudioData audioData;
                bool found = false;
                if (size >= 0 && seconds >= 0 && offset >= 0)
                {
                    std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                    auto j = p->audioMutex.audioDataCache.find(seconds);
                    if (j != p->audioMutex.audioDataCache.end())
                    {
                        audioData = j->second;
                        found = true;
                    }
                }

                if (found)
                {
                    // Mix the audio layers.
                    std::vector<const uint8_t*> audioLayerP;
                    for (const auto& layer : audioData.layers)
                    {
                        if (layer.audio && layer.audio->getInfo() == p->ioInfo.audio)
                        {
                            audioLayerP.push_back(
                                layer.audio->getData() +
                                offset * inputInfo.getByteCount());
                        }
                    }
                    auto audio = audio::Audio::create(inputInfo, size);
                    audio->zero();
                    const auto now = std::chrono::steady_clock::now();
                    if (mute ||
                        now < muteTimeout ||
                        speed != p->timeline->getTimeRange().duration().rate())
                    {
                        volume = 0.F;
                    }
                    audio::mix(
                        audioLayerP.data(),
                        audioLayerP.size(),
                        audio->getData(),
                        volume,
                        size,
                        inputInfo.channelCount,
                        inputInfo.dataType);

                    // Reverse the audio if necessary.
                    if (Playback::Reverse == playback)
                    {
                        auto tmp = audio::Audio::create(inputInfo, audio->getSampleCount());
                        audio::reverse(
                            audio->getData(),
                            tmp->getData(),
                            size,
                            audio->getChannelCount(),
                            audio->getDataType());
                        audio = tmp;
                    }

                    // Resample the audio and add it to the buffer.
                    p->audioThread.buffer.push_back(p->audioThread.resample->process(audio));
                }

                // Send audio data to RtAudio.
                if (nFrames <= getSampleCount(p->audioThread.buffer))
                {
                    audio::move(
                        p->audioThread.buffer,
                        reinterpret_cast<uint8_t*>(outputBuffer),
                        nFrames);
                }

                // Update the frame counter.
                {
                    std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                    p->audioMutex.frame += size;
                }
            }

            return 0;
        }

        void Player::Private::rtAudioErrorCallback(
            RtAudioError::Type type,
            const std::string& errorText)
        {
            //std::cout << "RtAudio ERROR: " << errorText << std::endl;
        }
#endif // TLRENDER_AUDIO
    }
}
