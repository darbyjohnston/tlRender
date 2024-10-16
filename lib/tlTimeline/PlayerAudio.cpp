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
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.audioOffset = value;
                }
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioMutex.audioOffset = value;
                }
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
            bool out = false;
#if defined(TLRENDER_RTAUDIO) || defined(TLRENDER_SDL2)
            out = ioInfo.audio.isValid();
#endif
            return out;
        }

        namespace
        {
#if defined(TLRENDER_RTAUDIO)
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
#endif // TLRENDER_RTAUDIO
#if defined(TLRENDER_SDL2)
            SDL_AudioFormat toSDL2(audio::DataType value) noexcept
            {
                SDL_AudioFormat out = 0;
                switch (value)
                {
                case audio::DataType::S8: out = AUDIO_S8; break;
                case audio::DataType::S16: out = AUDIO_S16; break;
                case audio::DataType::S32: out = AUDIO_S32; break;
                case audio::DataType::F32: out = AUDIO_F32; break;
                default: break;
                }
                return out;
            }
#endif // TLRENDER_RTAUDIO
        }

        void Player::Private::audioInit(const std::shared_ptr<system::Context>& context)
        {
#if defined(TLRENDER_RTAUDIO)
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
                auto devices = audioSystem->getDevices();
                if (playerOptions.audioMinPreferredSampleRate)
                {
                    for (auto i = devices.begin(); i != devices.end(); ++i)
                    {
                        for (auto j = devices.begin(); j != devices.end(); ++j)
                        {
                            if (i != j && i->id.name == j->id.name)
                            {
                                i->preferredSampleRate = std::min(
                                    i->preferredSampleRate,
                                    j->preferredSampleRate);
                                i->inputInfo.sampleRate = i->preferredSampleRate;
                                i->outputInfo.sampleRate = i->preferredSampleRate;
                            }
                        }
                    }
                }
                auto i = std::find_if(
                    devices.begin(),
                    devices.end(),
                    [id](const audio::DeviceInfo& value)
                    {
                        return id == value.id;
                    });
                audioInfo = audio::Info();
                if (i == devices.end())
                {
                    id = audioSystem->getDefaultOutputDevice();
                    i = std::find_if(
                        devices.begin(),
                        devices.end(),
                        [id](const audio::DeviceInfo& value)
                        {
                            return id == value.id;
                        });
                }
                if (i != devices.end())
                {
                    audioInfo = i->outputInfo;
                }
                {
                    std::stringstream ss;
                    ss << "Opening audio device: " << id.number << " " << id.name << "\n" <<
                        "  buffer frames: " << playerOptions.audioBufferFrameCount << "\n" <<
                        "  channels: " << audioInfo.channelCount << "\n" <<
                        "  data type: " << audioInfo.dataType << "\n" <<
                        "  sample rate: " << audioInfo.sampleRate;
                    context->log("tl::timeline::Player", ss.str());
                }
                audioInfo.channelCount = getAudioChannelCount(
                    ioInfo.audio,
                    audioInfo);
                if (audioInfo.isValid())
                {
                    // These are OK to modify since the audio thread is stopped.
                    audioMutex.reset = true;
                    audioMutex.start = currentTime->get();
                    audioMutex.frame = 0;
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
                        audioInfo.sampleRate = rtAudio->getStreamSampleRate();
                        {
                            std::stringstream ss;
                            ss << "Audio device sample rate: " << audioInfo.sampleRate;
                            context->log("tl::timeline::Player", ss.str());
                        }
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
#endif // TLRENDER_RTAUDIO
#if defined(TLRENDER_SDL2)
            SDL_CloseAudio();
            SDL_AudioSpec inSpec;
            inSpec.freq = 48000;
            inSpec.format = AUDIO_F32;
            inSpec.channels = 2;
            inSpec.samples = playerOptions.audioBufferFrameCount;
            inSpec.callback = sdl2Callback;
            inSpec.userdata = this;
            SDL_AudioSpec outSpec;
            if (SDL_OpenAudio(&inSpec, &outSpec) >= 0)
            {
                audioInfo.channelCount = outSpec.channels;
                audioInfo.dataType = audio::DataType::F32;
                audioInfo.sampleRate = outSpec.freq;

                // These are OK to modify since the audio thread is stopped.
                audioMutex.reset = true;
                audioMutex.start = currentTime->get();
                audioMutex.frame = 0;
                audioThread.info = audioInfo;
                audioThread.resample.reset();

                SDL_PauseAudio(0);
            }
            else
            {
                std::stringstream ss;
                ss << "Cannot open audio stream: " << SDL_GetError();
                context->log("tl::timeline::Player", ss.str(), log::Type::Error);
            }
#endif // TLRENDER_SDL2
        }

        void Player::Private::audioReset(const otime::RationalTime& time)
        {
            audioMutex.reset = true;
            audioMutex.start = time;
            audioMutex.frame = 0;
        }

        size_t Player::Private::getAudioChannelCount(
            const audio::Info& input,
            const audio::Info& output)
        {
            size_t out = std::min(static_cast<size_t>(2), output.channelCount);
            if (input.channelCount == output.channelCount)
            {
                out = output.channelCount;
            }
            return out;
        }

#if defined(TLRENDER_RTAUDIO) || defined(TLRENDER_SDL2)
#if defined(TLRENDER_RTAUDIO)
        int Player::Private::rtAudioCallback(
            void* outputBuffer,
            void* inputBuffer,
            unsigned int nFrames,
            double streamTime,
            RtAudioStreamStatus status,
            void* userData)
#endif // TLRENDER_RTAUDIO
#if defined(TLRENDER_SDL2)
            void Player::Private::sdl2Callback(
                void* userData,
                Uint8* outputBuffer,
                int len)
#endif // TLRENDER_SDL2
        {
            auto p = reinterpret_cast<Player::Private*>(userData);
#if defined(TLRENDER_SDL2)
            unsigned int nFrames = len / p->audioThread.info.getByteCount();
#endif // TLRENDER_SDL2

            // Get mutex protected values.
            Playback playback = Playback::Stop;
            double speed = 0.0;
            float volume = 1.F;
            bool mute = false;
            std::chrono::steady_clock::time_point muteTimeout;
            double audioOffset = 0.0;
            bool reset = false;
            otime::RationalTime start = time::invalidTime;
            {
                std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                playback = p->audioMutex.playback;
                speed = p->audioMutex.speed;
                volume = p->audioMutex.volume;
                mute = p->audioMutex.mute;
                muteTimeout = p->audioMutex.muteTimeout;
                audioOffset = p->audioMutex.audioOffset;
                reset = p->audioMutex.reset;
                p->audioMutex.reset = false;
                start = p->audioMutex.start;
                if (reset)
                {
                    p->audioMutex.frame = 0;
                }
            }
            //std::cout << "playback: " << playback << std::endl;
            //std::cout << "reset: " << reset << std::endl;
            //std::cout << "start: " << start << std::endl;

            // Zero output audio data.
            const audio::Info& outputInfo = p->audioThread.info;
            std::memset(outputBuffer, 0, nFrames * outputInfo.getByteCount());

            const audio::Info& inputInfo = p->ioInfo.audio;
            if (playback != Playback::Stop && inputInfo.sampleRate > 0)
            {
                // Initialize on reset.
                if (reset)
                {
                    p->audioThread.inputFrame = 0;
                    p->audioThread.outputFrame = 0;
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
                int64_t t =
                    (start - p->timeRange.start_time()).rescaled_to(inputInfo.sampleRate).value() -
                    otime::RationalTime(audioOffset, 1.0).rescaled_to(inputInfo.sampleRate).value();
                if (Playback::Forward == playback)
                {
                    t += p->audioThread.inputFrame;
                }
                else
                {
                    t -= p->audioThread.inputFrame;
                }
                std::vector<AudioData> audioDataList;
                {
                    const int64_t seconds = std::floor(t / static_cast<double>(inputInfo.sampleRate));
                    std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                    for (int64_t i = seconds - 1; i < seconds + 1; ++i)
                    {
                        auto j = p->audioMutex.audioDataCache.find(i);
                        if (j != p->audioMutex.audioDataCache.end())
                        {
                            audioDataList.push_back(j->second);
                        }
                    }
                }
                int64_t size = otio::RationalTime(
                    nFrames * 2 - getSampleCount(p->audioThread.buffer),
                    outputInfo.sampleRate).
                    rescaled_to(inputInfo.sampleRate).value();
                const auto audioList = audioCopy(
                    inputInfo,
                    audioDataList,
                    playback,
                    t,
                    size);

                if (!audioList.empty())
                {
                    // Mix the audio layers.
                    std::vector<const uint8_t*> audioP;
                    for (const auto& i : audioList)
                    {
                        audioP.push_back(i->getData());
                    }
                    auto audio = audio::Audio::create(
                        inputInfo,
                        audioList[0]->getSampleCount());
                    const auto now = std::chrono::steady_clock::now();
                    if (mute ||
                        now < muteTimeout ||
                        speed != p->timeRange.duration().rate())
                    {
                        volume = 0.F;
                    }
                    audio::mix(
                        audioP.data(),
                        audioP.size(),
                        audio->getData(),
                        volume,
                        audioList[0]->getSampleCount(),
                        inputInfo.channelCount,
                        inputInfo.dataType);

                    // Reverse the audio if necessary.
                    if (Playback::Reverse == playback)
                    {
                        auto tmp = audio::Audio::create(inputInfo, audio->getSampleCount());
                        audio::reverse(
                            audio->getData(),
                            tmp->getData(),
                            audio->getSampleCount(),
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

                // Update the frame counters.
                if (!audioList.empty() || p->audioThread.cacheRetryCount > 1)
                {
                    p->audioThread.cacheRetryCount = 0;
                    p->audioThread.inputFrame += !audioList.empty() ?
                        audioList[0]->getSampleCount() :
                        otio::RationalTime(
                            nFrames,
                            outputInfo.sampleRate).
                        rescaled_to(inputInfo.sampleRate).value();
                    p->audioThread.outputFrame += nFrames;
                }
                else
                {
                    p->audioThread.cacheRetryCount += 1;
                }
                const int64_t outputFrame = otio::RationalTime(
                    p->audioThread.outputFrame,
                    outputInfo.sampleRate).
                    rescaled_to(inputInfo.sampleRate).value();
                {
                    std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                    p->audioMutex.frame = outputFrame;
                }
            }

#if defined(TLRENDER_RTAUDIO)
            return 0;
#endif // TLRENDER_RTAUDIO
        }
#endif // TLRENDER_RTAUDIO || TLRENDER_SDL2

#if defined(TLRENDER_RTAUDIO)
        void Player::Private::rtAudioErrorCallback(
            RtAudioError::Type type,
            const std::string& errorText)
        {
            //std::cout << "RtAudio ERROR: " << errorText << std::endl;
        }
#endif // TLRENDER_RTAUDIO
    }
}
