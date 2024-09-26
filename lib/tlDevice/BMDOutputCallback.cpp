// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputPrivate.h>

#include <tlDevice/BMDUtil.h>

namespace tl
{
    namespace bmd
    {
        namespace
        {
            const size_t videoFramesMax = 3;
            const size_t audioBufferCount = 2000;
        }

        DLOutputCallback::DLOutputCallback(
            IDeckLinkOutput* dlOutput,
            const math::Size2i& size,
            PixelType pixelType,
            const FrameRate& frameRate,
            int videoFrameDelay,
            const audio::Info& audioInfo) :
            _dlOutput(dlOutput),
            _size(size),
            _pixelType(pixelType),
            _frameRate(frameRate),
            _audioInfo(audioInfo)
        {
            _refCount = 1;

#if defined(_WINDOWS)
            HRESULT r = _videoThread.frameConverter.CoCreateInstance(CLSID_CDeckLinkVideoConversion, nullptr, CLSCTX_ALL);
            if (r != S_OK)
            {
                throw std::runtime_error("Cannot create video frame converter");
            }
#else // _WINDOWS
            _videoThread.frameConverter.p = CreateVideoConversionInstance();
            if (!_videoThread.frameConverter.p)
            {
                throw std::runtime_error("Cannot create video frame converter");
            }
#endif // _WINDOWS

            IDeckLinkProfileAttributes* dlProfileAttributes = nullptr;
            if (dlOutput->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&dlProfileAttributes) == S_OK)
            {
                LONGLONG minVideoPreroll = 0;
                if (dlProfileAttributes->GetInt(BMDDeckLinkMinimumPrerollFrames, &minVideoPreroll) == S_OK)
                {
                    //! \bug Leave the default preroll, lower numbers
                    //! cause stuttering.
                    //videoPreroll = minVideoPreroll;
                }
            }

            _dlOutput->BeginAudioPreroll();
            /*const size_t audioPrerollSamples = videoPreroll / 24.0 * audioInfo.sampleRate;
            std::vector<uint8_t> emptyAudio(
                audioPrerollSamples *
                audioInfo.channelCount *
                audio::getByteCount(audioInfo.dataType), 0);
            uint32_t audioSamplesWritten = 0;
            _dlOutput->ScheduleAudioSamples(
                emptyAudio.data(),
                audioPrerollSamples,
                0,
                0,
                nullptr);*/
            _dlOutput->EndAudioPreroll();

            for (size_t i = 0; i < videoFrameDelay; ++i)
            {
                DLVideoFrameWrapper dlVideoFrame;
                if (_dlOutput->CreateVideoFrame(
                    _size.w,
                    _size.h,
                    getRowByteCount(_size.w, _pixelType),
                    toBMD(_pixelType),
                    bmdFrameFlagDefault,
                    &dlVideoFrame.p) != S_OK)
                {
                    throw std::runtime_error("Cannot create video frame");
                }
                if (_dlOutput->ScheduleVideoFrame(
                    dlVideoFrame.p,
                    _videoThread.frameCount * _frameRate.num,
                    _frameRate.num,
                    _frameRate.den) != S_OK)
                {
                    throw std::runtime_error("Cannot schedule video frame");
                }
                _videoThread.frameCount = _videoThread.frameCount + 1;
            }

            _videoThread.t = std::chrono::steady_clock::now();

            _dlOutput->StartScheduledPlayback(0, _frameRate.den, 1.0);
        }

        void DLOutputCallback::setPlayback(
            timeline::Playback value,
            const otime::RationalTime& time)
        {
            std::unique_lock<std::mutex> lock(_audioMutex.mutex);
            if (value != _audioMutex.playback)
            {
                _dlOutput->FlushBufferedAudioSamples();
                _audioMutex.playback = value;
                _audioMutex.reset = true;
                _audioMutex.start = time;
                _audioMutex.current = time;
            }
        }

        void DLOutputCallback::setCurrentTime(const otime::RationalTime& time)
        {
            std::unique_lock<std::mutex> lock(_audioMutex.mutex);
            _audioMutex.current = time;
        }

        void DLOutputCallback::seek(const otime::RationalTime& time)
        {
            if (time == _seek)
                return;
            _seek = time;
            std::unique_lock<std::mutex> lock(_audioMutex.mutex);
            _audioMutex.reset = true;
            _audioMutex.start = time;
            _audioMutex.current = time;
        }

        void DLOutputCallback::setVideo(const std::shared_ptr<DLVideoFrameWrapper>& value)
        {
            std::unique_lock<std::mutex> lock(_videoMutex.mutex);
            _videoMutex.videoFrames.push_back(value);
            while (_videoMutex.videoFrames.size() > videoFramesMax)
            {
                _videoMutex.videoFrames.pop_front();
            }
        }

        void DLOutputCallback::setVolume(float value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex.mutex);
            _audioMutex.volume = value;
        }

        void DLOutputCallback::setMute(bool value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex.mutex);
            _audioMutex.mute = value;
        }

        void DLOutputCallback::setAudioOffset(double value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex.mutex);
            _audioMutex.audioOffset = value;
        }

        void DLOutputCallback::setAudioData(const std::vector<timeline::AudioData>& value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex.mutex);
            _audioMutex.audioData = value;
        }

        HRESULT DLOutputCallback::QueryInterface(REFIID iid, LPVOID* ppv)
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        ULONG DLOutputCallback::AddRef()
        {
            return ++_refCount;
        }

        ULONG DLOutputCallback::Release()
        {
            const ULONG out = --_refCount;
            if (0 == out)
            {
                delete this;
                return 0;
            }
            return out;
        }

        HRESULT DLOutputCallback::ScheduledFrameCompleted(
            IDeckLinkVideoFrame* dlVideoFrame,
            BMDOutputFrameCompletionResult dlResult)
        {
            {
                std::unique_lock<std::mutex> lock(_videoMutex.mutex);
                if (!_videoMutex.videoFrames.empty())
                {
                    _videoThread.videoFrame = _videoMutex.videoFrames.front();
                    _videoMutex.videoFrames.pop_front();
                }
            }

            if (_videoThread.videoFrame)
            {
                if (_videoThread.videoFrame->p->GetPixelFormat() ==
                    toBMD(_pixelType))
                {
                    dlVideoFrame = _videoThread.videoFrame->p;
                }
                else
                {
                    _videoThread.frameConverter->ConvertFrame(
                        _videoThread.videoFrame->p,
                        dlVideoFrame);
                }
            }

            _dlOutput->ScheduleVideoFrame(
                dlVideoFrame,
                _videoThread.frameCount * _frameRate.num,
                _frameRate.num,
                _frameRate.den);
            //std::cout << "result: " << getOutputFrameCompletionResultLabel(dlResult) << std::endl;
            _videoThread.frameCount += 1;

            const auto t = std::chrono::steady_clock::now();
            const std::chrono::duration<double> diff = t - _videoThread.t;
            //std::cout << "diff: " << diff.count() * 1000 << std::endl;
            _videoThread.t = t;

            return S_OK;
        }

        HRESULT DLOutputCallback::ScheduledPlaybackHasStopped()
        {
            return S_OK;
        }

        HRESULT DLOutputCallback::RenderAudioSamples(BOOL preroll)
        {
            // Get values.
            timeline::Playback playback = timeline::Playback::Stop;
            float volume = 1.F;
            bool mute = false;
            double audioOffset = 0.0;
            std::vector<timeline::AudioData> audioDataList;
            bool reset = false;
            otime::RationalTime start = time::invalidTime;
            otime::RationalTime current = time::invalidTime;
            {
                std::unique_lock<std::mutex> lock(_audioMutex.mutex);
                playback = _audioMutex.playback;
                volume = _audioMutex.volume;
                mute = _audioMutex.mute;
                audioOffset = _audioMutex.audioOffset;
                audioDataList = _audioMutex.audioData;
                reset = _audioMutex.reset;
                _audioMutex.reset = false;
                start = _audioMutex.start;
                current = _audioMutex.current;
            }
            //std::cout << "audio playback: " << playback << std::endl;
            //std::cout << "audio reset: " << reset << std::endl;
            //std::cout << "audio start: " << start << std::endl;

            // Initialize on reset.
            if (reset)
            {
                _audioThread.frame = 0;
                if (_audioThread.resample)
                {
                    _audioThread.resample->flush();
                }
                _dlOutput->FlushBufferedAudioSamples();
            }

            audio::Info inputInfo;
            if (!audioDataList.empty() &&
                !audioDataList[0].layers.empty() &&
                audioDataList[0].layers[0].audio)
            {
                inputInfo = audioDataList[0].layers[0].audio->getInfo();
            }
            if (playback != timeline::Playback::Stop && inputInfo.sampleRate > 0)
            {
                // Create the audio resampler.
                if (!_audioThread.resample ||
                    (_audioThread.resample && _audioThread.resample->getInputInfo() != inputInfo))
                {
                    _audioThread.resample = audio::AudioResample::create(
                        inputInfo,
                        _audioInfo);
                }

                uint32_t bufferedSampleCount = 0;
                _dlOutput->GetBufferedAudioSampleFrameCount(&bufferedSampleCount);
                //std::cout << "bmd buffered samples: " << bufferedSampleCount << std::endl;
                if (bufferedSampleCount < audioBufferCount)
                {
                    // Find the audio data.
                    timeline::AudioData audioData;
                    int64_t t =
                        start.rescaled_to(inputInfo.sampleRate).value() -
                        otime::RationalTime(audioOffset, 1.0).rescaled_to(inputInfo.sampleRate).value();
                    if (timeline::Playback::Forward == playback)
                    {
                        t += _audioThread.frame;
                    }
                    else
                    {
                        t -= _audioThread.frame;
                    }
                    int64_t seconds = t / inputInfo.sampleRate;
                    int64_t offset = t - seconds * inputInfo.sampleRate;
                    int64_t size = audioBufferCount - bufferedSampleCount;
                    if (timeline::Playback::Forward == playback)
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
                    //std::cout << "start: " << start << std::endl;
                    //std::cout << "current: " << current << std::endl;
                    //std::cout << "t: " << t << std::endl;
                    //std::cout << "seconds: " << seconds << std::endl;
                    //std::cout << "offset: " << offset << std::endl;
                    //std::cout << "size: " << size << std::endl;
                    bool found = false;
                    if (size >= 0 && seconds >= 0 && offset >= 0)
                    {
                        for (const auto& i : audioDataList)
                        {
                            if (seconds == static_cast<int64_t>(i.seconds))
                            {
                                audioData = i;
                                found = true;
                                break;
                            }
                        }
                    }
                    //std::cout << "found: " << found << std::endl;
                    //std::cout << std::endl;

                    if (found)
                    {
                        // Mix the audio layers.
                        std::vector<const uint8_t*> audioDataP;
                        for (const auto& layer : audioData.layers)
                        {
                            if (layer.audio && layer.audio->getInfo() == inputInfo)
                            {
                                audioDataP.push_back(layer.audio->getData() + offset * inputInfo.getByteCount());
                            }
                        }
                        auto audio = audio::Audio::create(inputInfo, size);
                        audio::mix(
                            audioDataP.data(),
                            audioDataP.size(),
                            audio->getData(),
                            mute ? 0.F : volume,
                            size,
                            inputInfo.channelCount,
                            inputInfo.dataType);

                        // Reverse the audio if necessary.
                        if (timeline::Playback::Reverse == playback)
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

                        // Resample the audio.
                        auto resampledAudio = _audioThread.resample->process(audio);

                        // Send audio data to the device.
                        _dlOutput->ScheduleAudioSamples(
                            resampledAudio->getData(),
                            resampledAudio->getSampleCount(),
                            0,
                            0,
                            nullptr);

                        // Update the frame counter.
                        _audioThread.frame += size;
                    }
                    else
                    {
                        std::unique_lock<std::mutex> lock(_audioMutex.mutex);
                        _audioMutex.reset = true;
                        _audioMutex.start = current;
                    }
                }
            }

            //BMDTimeScale dlTimeScale = audioSampleRate;
            //BMDTimeValue dlTimeValue = 0;
            //if (_dlOutput->GetScheduledStreamTime(dlTimeScale, &dlTimeValue, nullptr) == S_OK)
            //{
            //    std::cout << "stream time: " << dlTimeValue << std::endl;
            //}

            return S_OK;
        }
    }
}
