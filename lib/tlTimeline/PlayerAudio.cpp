// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        int Player::getAudioDevice() const
        {
            return _p->audioDevice->get();
        }

        std::shared_ptr<observer::IValue<int> > Player::observeAudioDevice() const
        {
            return _p->audioDevice;
        }

        void Player::setAudioDevice(int value)
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

            const int device = audioDevice->get();
            if (rtAudio && device != -1)
            {
                auto audioSystem = context->getSystem<audio::System>();
                const auto devices = audioSystem->getDevices();
                if (device >= 0 && device < devices.size())
                {
                    audioInfo = devices[device].outputInfo;
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
                        rtParameters.deviceId = device;
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

        void Player::Private::resetAudioTime()
        {
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.reset = true;
            }
#if defined(TLRENDER_AUDIO)
            if (rtAudio &&
                rtAudio->isStreamRunning())
            {
                try
                {
                    rtAudio->setStreamTime(0.0);
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }
            }
#endif // TLRENDER_AUDIO
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
            otime::RationalTime playbackStartTime = time::invalidTime;
            otime::RationalTime currentTime = time::invalidTime;
            double audioOffset = 0.0;
            {
                std::unique_lock<std::mutex> lock(p->mutex.mutex);
                playback = p->mutex.playback;
                playbackStartTime = p->mutex.playbackStartTime;
                currentTime = p->mutex.currentTime;
                audioOffset = p->mutex.audioOffset;
            }
            double speed = 0.0;
            float volume = 1.F;
            bool mute = false;
            std::chrono::steady_clock::time_point muteTimeout;
            bool reset = false;
            {
                std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                speed = p->audioMutex.speed;
                volume = p->audioMutex.volume;
                mute = p->audioMutex.mute;
                muteTimeout = p->audioMutex.muteTimeout;
                reset = p->audioMutex.reset;
                p->audioMutex.reset = false;
            }
            //std::cout << "playback: " << playback << std::endl;
            //std::cout << "playbackStartTime: " << playbackStartTime << std::endl;
            //std::cout << "reset: " << reset << std::endl;

            // Check if the timers should be initialized.
            if (playback != p->audioThread.playback ||
                speed != p->audioThread.speed ||
                reset)
            {
                p->audioThread.playback = playback;
                p->audioThread.speed = speed;
                {
                    std::unique_lock<std::mutex> lock(p->mutex.mutex);
                    p->mutex.playbackStartTime = currentTime;
                    p->mutex.playbackStartTimer = std::chrono::steady_clock::now();
                }
            }

            // Zero output audio data.
            const audio::Info& outputInfo = p->audioThread.info;
            std::memset(outputBuffer, 0, nFrames * outputInfo.getByteCount());

            const audio::Info& inputInfo = p->ioInfo.audio;
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
                    p->audioThread.rtAudioCurrentFrame = 0;
                }

                // Create the audio resampler.
                if (!p->audioThread.resample ||
                    (p->audioThread.resample && p->audioThread.resample->getInputInfo() != p->ioInfo.audio))
                {
                    p->audioThread.resample = audio::AudioResample::create(
                        p->ioInfo.audio,
                        p->audioThread.info);
                }

                // Calculate the audio frame.
                const int64_t playbackStartFrame =
                    playbackStartTime.rescaled_to(inputInfo.sampleRate).value() -
                    p->timeline->getTimeRange().start_time().rescaled_to(inputInfo.sampleRate).value() -
                    otime::RationalTime(audioOffset, 1.0).rescaled_to(inputInfo.sampleRate).value();
                const int64_t playbackStartFrameOffset = otime::RationalTime(
                    p->audioThread.rtAudioCurrentFrame + audio::getSampleCount(p->audioThread.buffer),
                    p->audioThread.info.sampleRate).rescaled_to(inputInfo.sampleRate).value();

                // Fill the audio buffer.
                const size_t bufferSizeMax = std::max(
                    static_cast<size_t>(nFrames),
                    p->playerOptions.audioBufferFrameCount);
                int64_t fillSize = bufferSizeMax - audio::getSampleCount(p->audioThread.buffer);
                int64_t frame = Playback::Forward == playback ?
                    (playbackStartFrame + playbackStartFrameOffset) :
                    (playbackStartFrame - playbackStartFrameOffset - fillSize);
                int64_t seconds = frame / inputInfo.sampleRate;
                int64_t offset = frame - seconds * inputInfo.sampleRate;
                while (audio::getSampleCount(p->audioThread.buffer) < bufferSizeMax &&
                    p->running)
                {
                    // Get audio from the cache.
                    AudioData audioData;
                    bool found = false;
                    {
                        std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                        auto j = p->audioMutex.audioDataCache.find(seconds);
                        if (j != p->audioMutex.audioDataCache.end())
                        {
                            audioData = j->second;
                            found = true;
                        }
                    }
                    if (!found)
                    {
                        break;
                    }

                    // Mix the audio layers.
                    std::vector<const uint8_t*> audioDataP;
                    for (const auto& layer : audioData.layers)
                    {
                        if (layer.audio && layer.audio->getInfo() == p->ioInfo.audio)
                        {
                            audioDataP.push_back(
                                layer.audio->getData() +
                                offset * inputInfo.getByteCount());
                        }
                    }
                    const int64_t size = std::min(
                        fillSize,
                        static_cast<int64_t>(inputInfo.sampleRate - offset));
                    auto tmp = audio::Audio::create(inputInfo, size);
                    tmp->zero();
                    audio::mix(
                        audioDataP.data(),
                        audioDataP.size(),
                        tmp->getData(),
                        volume,
                        Playback::Reverse == playback,
                        size,
                        p->ioInfo.audio.channelCount,
                        p->ioInfo.audio.dataType);

                    // Resample the audio and add it to the buffer.
                    p->audioThread.buffer.push_back(p->audioThread.resample->process(tmp));

                    fillSize -= size;
                    frame += size;
                    seconds = frame / inputInfo.sampleRate;
                    offset = frame - seconds * inputInfo.sampleRate;
                }

                // Send audio data to RtAudio.
                const auto now = std::chrono::steady_clock::now();
                if (speed == p->timeline->getTimeRange().duration().rate() &&
                    !mute &&
                    now >= muteTimeout &&
                    nFrames <= getSampleCount(p->audioThread.buffer))
                {
                    audio::move(
                        p->audioThread.buffer,
                        reinterpret_cast<uint8_t*>(outputBuffer),
                        nFrames);
                }

                // Update the audio frame.
                p->audioThread.rtAudioCurrentFrame += nFrames;
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
