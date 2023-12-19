// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputDevicePrivate.h>

#include <tlDevice/BMDUtil.h>

#include <tlCore/AudioResample.h>
#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <list>
#include <mutex>
#include <tuple>

#if defined(_WINDOWS)
#include <atlbase.h>
#endif // _WINDOWS

namespace tl
{
    namespace device
    {
        namespace
        {
            const size_t videoFramesMax = 3;
            //! \todo Should this be the same as
            //! timeline::PlayerOptions().audioBufferFrameCount?
            const size_t audioBufferCount = 3000;
        }

        /*HRESULT DLHDRVideoFrame::QueryInterface(REFIID iid, LPVOID* ppv)
        {
            IID iunknown = IID_IUnknown;
            if (ppv == nullptr)
                return E_INVALIDARG;
            if (memcmp(&iid, &iunknown, sizeof(REFIID)) == 0)
                *ppv = static_cast<IDeckLinkVideoFrame*>(this);
            else if (memcmp(&iid, &IID_IDeckLinkVideoFrame, sizeof(REFIID)) == 0)
                *ppv = static_cast<IDeckLinkVideoFrame*>(this);
            else if (memcmp(&iid, &IID_IDeckLinkVideoFrameMetadataExtensions, sizeof(REFIID)) == 0)
                *ppv = static_cast<IDeckLinkVideoFrameMetadataExtensions*>(this);
            else
            {
                *ppv = nullptr;
                return E_NOINTERFACE;
            }
            AddRef();
            return S_OK;
        }

        ULONG DLHDRVideoFrame::AddRef(void)
        {
            return ++_refCount;
        }

        ULONG DLHDRVideoFrame::Release(void)
        {
            ULONG newRefValue = --_refCount;
            if (newRefValue == 0)
                delete this;
            return newRefValue;
        }

        HRESULT DLHDRVideoFrame::GetInt(BMDDeckLinkFrameMetadataID metadataID, int64_t* value)
        {
            HRESULT result = S_OK;
            switch (metadataID)
            {
            case bmdDeckLinkFrameMetadataHDRElectroOpticalTransferFunc:
                *value = _hdrData.eotf;
                break;
            case bmdDeckLinkFrameMetadataColorspace:
                *value = bmdColorspaceRec2020;
                break;
            default:
                value = nullptr;
                result = E_INVALIDARG;
            }
            return result;
        }

        HRESULT DLHDRVideoFrame::GetFloat(BMDDeckLinkFrameMetadataID metadataID, double* value)
        {
            HRESULT result = S_OK;
            switch (metadataID)
            {
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedX:
                *value = _hdrData.redPrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesRedY:
                *value = _hdrData.redPrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenX:
                *value = _hdrData.greenPrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesGreenY:
                *value = _hdrData.greenPrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueX:
                *value = _hdrData.bluePrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRDisplayPrimariesBlueY:
                *value = _hdrData.bluePrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRWhitePointX:
                *value = _hdrData.whitePrimaries.x;
                break;
            case bmdDeckLinkFrameMetadataHDRWhitePointY:
                *value = _hdrData.whitePrimaries.y;
                break;
            case bmdDeckLinkFrameMetadataHDRMaxDisplayMasteringLuminance:
                *value = _hdrData.displayMasteringLuminance.getMax();
                break;
            case bmdDeckLinkFrameMetadataHDRMinDisplayMasteringLuminance:
                *value = _hdrData.displayMasteringLuminance.getMin();
                break;
            case bmdDeckLinkFrameMetadataHDRMaximumContentLightLevel:
                *value = _hdrData.maxCLL;
                break;
            case bmdDeckLinkFrameMetadataHDRMaximumFrameAverageLightLevel:
                *value = _hdrData.maxFALL;
                break;
            default:
                value = nullptr;
                result = E_INVALIDARG;
            }
            return result;
        }

        HRESULT DLHDRVideoFrame::GetFlag(BMDDeckLinkFrameMetadataID, BOOL* value)
        {
            *value = false;
            return E_INVALIDARG;
        }

        HRESULT DLHDRVideoFrame::GetString(BMDDeckLinkFrameMetadataID, BSTR* value)
        {
            *value = nullptr;
            return E_INVALIDARG;
        }

        HRESULT	DLHDRVideoFrame::GetBytes(BMDDeckLinkFrameMetadataID metadataID, void* buffer, uint32_t* bufferSize)
        {
            *bufferSize = 0;
            return E_INVALIDARG;
        }*/

        struct DLOutputCallback::Private
        {
            IDeckLinkOutput* dlOutput = nullptr;
            math::Size2i size;
            PixelType pixelType = PixelType::None;
            otime::RationalTime frameRate = time::invalidTime;
            audio::Info audioInfo;

            std::atomic<size_t> refCount;

            struct VideoMutex
            {
                std::list<std::shared_ptr<DLVideoFrameWrapper> > videoFrames;
                std::mutex mutex;
            };
            VideoMutex videoMutex;

            struct VideoThread
            {
                std::shared_ptr<DLVideoFrameWrapper> videoFrame;
#if defined(_WINDOWS)
                CComPtr<IDeckLinkVideoConversion> frameConverter;
#endif // _WINDOWS
                uint64_t frameCount = 0;
            };
            VideoThread videoThread;

            struct AudioMutex
            {
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime startTime = time::invalidTime;
                otime::RationalTime currentTime = time::invalidTime;
                float volume = 1.F;
                bool mute = false;
                double audioOffset = 0.0;
                std::vector<timeline::AudioData> audioData;
                std::mutex mutex;
            };
            AudioMutex audioMutex;

            struct AudioThread
            {
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime startTime = time::invalidTime;
                size_t samplesOffset = 0;
                std::shared_ptr<audio::AudioResample> resample;
            };
            AudioThread audioThread;
        };

        DLOutputCallback::DLOutputCallback(
            IDeckLinkOutput* dlOutput,
            const math::Size2i& size,
            PixelType pixelType,
            const otime::RationalTime& frameRate,
            const audio::Info& audioInfo) :
            _p(new Private)
        {
            TLRENDER_P();

            p.dlOutput = dlOutput;
            p.size = size;
            p.pixelType = pixelType;
            p.frameRate = frameRate;
            p.audioInfo = audioInfo;
            p.refCount = 1;

#if defined(_WINDOWS)
            HRESULT r = p.videoThread.frameConverter.CoCreateInstance(CLSID_CDeckLinkVideoConversion, nullptr, CLSCTX_ALL);
            if (r != S_OK)
            {
                throw std::runtime_error("Cannot create video frame converter");
            }
#endif // _WINDOWS

            size_t videoPreroll = 3;
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

            p.dlOutput->BeginAudioPreroll();
            /*std::vector<uint8_t> emptyAudio(
                audioBufferChunkSize * audioChannelCount * audio::getByteCount(audioDataType), 0);
            uint32_t audioSamplesWritten = 0;
            p.dlOutput->ScheduleAudioSamples(
                emptyAudio.data(),
                6000,
                0,
                0,
                nullptr);*/
            p.dlOutput->EndAudioPreroll();

            for (size_t i = 0; i < videoPreroll; ++i)
            {
                DLVideoFrameWrapper dlVideoFrame;
                if (p.dlOutput->CreateVideoFrame(
                    p.size.w,
                    p.size.h,
                    getRowByteCount(p.size.w, pixelType),
                    toBMD(pixelType),
                    bmdFrameFlagFlipVertical,
                    &dlVideoFrame.p) != S_OK)
                {
                    throw std::runtime_error("Cannot create video frame");
                }
                if (p.dlOutput->ScheduleVideoFrame(
                    dlVideoFrame.p,
                    p.videoThread.frameCount * p.frameRate.value(),
                    p.frameRate.value(),
                    p.frameRate.rate()) != S_OK)
                {
                    throw std::runtime_error("Cannot schedule video frame");
                }
                p.videoThread.frameCount = p.videoThread.frameCount + 1;
            }

            p.dlOutput->StartScheduledPlayback(
                0,
                p.frameRate.rate(),
                1.0);
        }

        void DLOutputCallback::setPlayback(timeline::Playback value, const otime::RationalTime& time)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            if (value != p.audioMutex.playback)
            {
                p.dlOutput->FlushBufferedAudioSamples();
                p.audioMutex.playback = value;
                p.audioMutex.startTime = time;
                p.audioMutex.currentTime = time;
            }
        }

        void DLOutputCallback::setVideo(
            const std::shared_ptr<DLVideoFrameWrapper>& value,
            const otime::RationalTime& time)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                p.videoMutex.videoFrames.push_back(value);
                while (p.videoMutex.videoFrames.size() > videoFramesMax)
                {
                    p.videoMutex.videoFrames.pop_front();
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (time != p.audioMutex.currentTime)
                {
                    const otime::RationalTime currentTimePlusOne(
                        p.audioMutex.currentTime.value() + 1.0,
                        p.audioMutex.currentTime.rate());
                    if (time != currentTimePlusOne)
                    {
                        p.audioMutex.startTime = time;
                    }
                    p.audioMutex.currentTime = time;
                }
            }
        }

        void DLOutputCallback::setVolume(float value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.volume = value;
        }

        void DLOutputCallback::setMute(bool value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.mute = value;
        }

        void DLOutputCallback::setAudioOffset(double value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.audioOffset = value;
        }

        void DLOutputCallback::setAudioData(const std::vector<timeline::AudioData>& value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
            p.audioMutex.audioData = value;
        }

        HRESULT DLOutputCallback::QueryInterface(REFIID iid, LPVOID* ppv)
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        ULONG DLOutputCallback::AddRef()
        {
            return ++_p->refCount;
        }

        ULONG DLOutputCallback::Release()
        {
            ULONG out = --_p->refCount;
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
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.videoFrames.empty())
                {
                    p.videoThread.videoFrame = p.videoMutex.videoFrames.front();
                    p.videoMutex.videoFrames.pop_front();
                }
            }
            if (p.videoThread.videoFrame)
            {
                p.videoThread.frameConverter->ConvertFrame(
                    p.videoThread.videoFrame->p,
                    dlVideoFrame);
            }
            p.dlOutput->ScheduleVideoFrame(
                dlVideoFrame,
                p.videoThread.frameCount * p.frameRate.value(),
                p.frameRate.value(),
                p.frameRate.rate());
            p.videoThread.frameCount += 1;
            //std::cout << "result: " << getOutputFrameCompletionResultLabel(dlResult) << std::endl;
            return S_OK;
        }

        HRESULT DLOutputCallback::ScheduledPlaybackHasStopped()
        {
            return S_OK;
        }

        HRESULT DLOutputCallback::RenderAudioSamples(BOOL preroll)
        {
            TLRENDER_P();

            // Get values.
            otime::RationalTime currentTime = time::invalidTime;
            float volume = 1.F;
            bool mute = false;
            double audioOffset = 0.0;
            std::vector<timeline::AudioData> audioDataList;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (p.audioMutex.playback != p.audioThread.playback ||
                    p.audioMutex.startTime != p.audioThread.startTime)
                {
                    p.audioThread.playback = p.audioMutex.playback;
                    p.audioThread.startTime = p.audioMutex.startTime;
                    p.audioThread.samplesOffset = 0;
                }
                currentTime = p.audioMutex.currentTime;
                volume = p.audioMutex.volume;
                mute = p.audioMutex.mute;
                audioOffset = p.audioMutex.audioOffset;
                audioDataList = p.audioMutex.audioData;
            }
            //std::cout << "audio playback: " << p.audioThread.playback << std::endl;
            //std::cout << "audio start time: " << p.audioThread.startTime << std::endl;
            //std::cout << "audio samples offset: " << p.audioThread.samplesOffset << std::endl;

            // Flush the audio resampler and BMD buffer when the playback
            // is reset.
            if (0 == p.audioThread.samplesOffset)
            {
                if (p.audioThread.resample)
                {
                    p.audioThread.resample->flush();
                }
                p.dlOutput->FlushBufferedAudioSamples();
            }

            // Create the audio resampler.
            audio::Info inputInfo;
            if (!audioDataList.empty() &&
                !audioDataList[0].layers.empty() &&
                audioDataList[0].layers[0].audio)
            {
                inputInfo = audioDataList[0].layers[0].audio->getInfo();
                if (!p.audioThread.resample ||
                    (p.audioThread.resample && p.audioThread.resample->getInputInfo() != inputInfo))
                {
                    p.audioThread.resample = audio::AudioResample::create(inputInfo, p.audioInfo);
                }
            }

            // Copy audio data to BMD.
            if (timeline::Playback::Forward == p.audioThread.playback &&
                p.audioThread.resample)
            {
                int64_t frame =
                    p.audioThread.startTime.rescaled_to(inputInfo.sampleRate).value() -
                    otime::RationalTime(audioOffset, 1.0).rescaled_to(inputInfo.sampleRate).value() +
                    p.audioThread.samplesOffset;
                int64_t seconds = inputInfo.sampleRate > 0 ? (frame / inputInfo.sampleRate) : 0;
                int64_t offset = frame - seconds * inputInfo.sampleRate;

                uint32_t bufferedSampleCount = 0;
                p.dlOutput->GetBufferedAudioSampleFrameCount(&bufferedSampleCount);
                //std::cout << "bmd buffered sample count: " << bufferedSampleCount << std::endl;
                while (bufferedSampleCount < audioBufferCount)
                {
                    //std::cout << "frame: " << frame << std::endl;
                    //std::cout << "seconds: " << seconds << std::endl;
                    //std::cout << "offset: " << offset << std::endl;
                    timeline::AudioData audioData;
                    for (const auto& i : audioDataList)
                    {
                        if (seconds == i.seconds)
                        {
                            audioData = i;
                            break;
                        }
                    }
                    if (audioData.layers.empty())
                    {
                        {
                            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                            p.audioMutex.startTime = currentTime;
                        }
                        p.audioThread.startTime = currentTime;
                        p.audioThread.samplesOffset = 0;
                        break;
                    }
                    std::vector<const uint8_t*> audioDataP;
                    for (const auto& layer : audioData.layers)
                    {
                        if (layer.audio && layer.audio->getInfo() == inputInfo)
                        {
                            audioDataP.push_back(layer.audio->getData() + offset * inputInfo.getByteCount());
                        }
                    }

                    const size_t size = std::min(
                        audioBufferCount,
                        inputInfo.sampleRate - static_cast<size_t>(offset));
                    //std::cout << "size: " << size << " " << std::endl;
                    auto tmpAudio = audio::Audio::create(inputInfo, size);
                    audio::mix(
                        audioDataP.data(),
                        audioDataP.size(),
                        tmpAudio->getData(),
                        mute ? 0.F : volume,
                        size,
                        inputInfo.channelCount,
                        inputInfo.dataType);

                    auto resampledAudio = p.audioThread.resample->process(tmpAudio);
                    p.dlOutput->ScheduleAudioSamples(
                        resampledAudio->getData(),
                        resampledAudio->getSampleCount(),
                        0,
                        0,
                        nullptr);

                    offset += size;
                    if (offset >= inputInfo.sampleRate)
                    {
                        offset -= inputInfo.sampleRate;
                        seconds += 1;
                    }

                    p.audioThread.samplesOffset += size;

                    HRESULT result = p.dlOutput->GetBufferedAudioSampleFrameCount(&bufferedSampleCount);
                    if (result != S_OK)
                    {
                        break;
                    }

                    //std::cout << std::endl;
                }
            }

            //BMDTimeScale dlTimeScale = audioSampleRate;
            //BMDTimeValue dlTimeValue = 0;
            //if (p.dlOutput->GetScheduledStreamTime(dlTimeScale, &dlTimeValue, nullptr) == S_OK)
            //{
            //    std::cout << "stream time: " << dlTimeValue << std::endl;
            //}

            return S_OK;
        }

        struct BMDOutputDevice::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::Value<DeviceConfig> > config;
            std::shared_ptr<observer::Value<bool> > enabled;
            std::shared_ptr<observer::Value<bool> > active;
            std::shared_ptr<observer::Value<math::Size2i> > size;
            std::shared_ptr<observer::Value<otime::RationalTime> > frameRate;

            struct DL
            {
                ~DL()
                {
                    dlOutput.p->StopScheduledPlayback(0, nullptr, 0);
                    dlOutput.p->DisableVideoOutput();
                    dlOutput.p->DisableAudioOutput();
                }

                DLWrapper dl;
                DLConfigWrapper dlConfig;
                DLOutputWrapper dlOutput;
                audio::Info audioInfo;
                DLOutputCallbackWrapper dlOutputCallback;
            };
            std::unique_ptr<DL> dl;
        };

        void BMDOutputDevice::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
            p.config = observer::Value<DeviceConfig>::create();
            p.enabled = observer::Value<bool>::create(false);
            p.active = observer::Value<bool>::create(false);
            p.size = observer::Value<math::Size2i>::create();
            p.frameRate = observer::Value<otime::RationalTime>::create(time::invalidTime);
        }

        BMDOutputDevice::BMDOutputDevice() :
            _p(new Private)
        {}

        BMDOutputDevice::~BMDOutputDevice()
        {}

        std::shared_ptr<BMDOutputDevice> BMDOutputDevice::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<BMDOutputDevice>(new BMDOutputDevice);
            out->_init(context);
            return out;
        }

        DeviceConfig BMDOutputDevice::getConfig() const
        {
            return _p->config->get();
        }

        std::shared_ptr<observer::IValue<DeviceConfig> > BMDOutputDevice::observeConfig() const
        {
            return _p->config;
        }

        void BMDOutputDevice::setConfig(const DeviceConfig& value)
        {
            TLRENDER_P();
            if (p.config->setIfChanged(value))
            {
                p.dl.reset(p.enabled->get() ? new Private::DL : nullptr);
                _deviceUpdate();
            }
        }

        bool BMDOutputDevice::isEnabled() const
        {
            return _p->enabled->get();
        }

        std::shared_ptr<observer::IValue<bool> > BMDOutputDevice::observeEnabled() const
        {
            return _p->enabled;
        }

        void BMDOutputDevice::setEnabled(bool value)
        {
            TLRENDER_P();
            if (p.enabled->setIfChanged(value))
            {
                p.dl.reset(value ? new Private::DL : nullptr);
                _deviceUpdate();
            }
        }

        bool BMDOutputDevice::isActive() const
        {
            return _p->active->get();
        }

        std::shared_ptr<observer::IValue<bool> > BMDOutputDevice::observeActive() const
        {
            return _p->active;
        }

        const math::Size2i& BMDOutputDevice::getSize() const
        {
            return _p->size->get();
        }

        std::shared_ptr<observer::IValue<math::Size2i> > BMDOutputDevice::observeSize() const
        {
            return _p->size;
        }

        const otime::RationalTime& BMDOutputDevice::getFrameRate() const
        {
            return _p->frameRate->get();
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> > BMDOutputDevice::observeFrameRate() const
        {
            return _p->frameRate;
        }

        void BMDOutputDevice::setView(
            const tl::math::Vector2i& position,
            float                     zoom,
            bool                      frame)
        {

        }

        void BMDOutputDevice::setOCIOOptions(const timeline::OCIOOptions&)
        {

        }

        void BMDOutputDevice::setLUTOptions(const timeline::LUTOptions&)
        {

        }

        void BMDOutputDevice::setImageOptions(const std::vector<timeline::ImageOptions>&)
        {

        }

        void BMDOutputDevice::setDisplayOptions(const std::vector<timeline::DisplayOptions>&)
        {

        }

        void BMDOutputDevice::setHDR(device::HDRMode, const image::HDRData&)
        {

        }

        void BMDOutputDevice::setCompareOptions(const timeline::CompareOptions&)
        {

        }

        void BMDOutputDevice::setPlayers(const std::vector<std::shared_ptr<timeline::Player> >&)
        {

        }

        void BMDOutputDevice::_deviceUpdate()
        {
            TLRENDER_P();
            math::Size2i size;
            otime::RationalTime frameRate = time::invalidTime;
            if (p.dl)
            {
                try
                {
                    const DeviceConfig& config = p.config->get();
                    std::string modelName;
                    {
                        DLIteratorWrapper dlIterator;
                        if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
                        {
                            throw std::runtime_error("Cannot get iterator");
                        }

                        int count = 0;
                        while (dlIterator.p->Next(&p.dl->dl.p) == S_OK)
                        {
                            if (count == config.deviceIndex)
                            {
#if defined(__APPLE__)
                                CFStringRef dlModelName;
                                p.dl.p->GetModelName(&dlModelName);
                                StringToStdString(dlModelName, modelName);
                                CFRelease(dlModelName);
#else // __APPLE__
                                dlstring_t dlModelName;
                                p.dl->dl.p->GetModelName(&dlModelName);
                                modelName = DlToStdString(dlModelName);
                                DeleteString(dlModelName);
#endif // __APPLE__

                                break;
                            }

                            p.dl->dl.p->Release();
                            p.dl->dl.p = nullptr;

                            ++count;
                        }
                        if (!p.dl->dl.p)
                        {
                            throw std::runtime_error("Device not found");
                        }
                    }

                    if (p.dl->dl.p->QueryInterface(IID_IDeckLinkConfiguration, (void**)&p.dl->dlConfig) != S_OK)
                    {
                        throw std::runtime_error("Configuration device not found");
                    }
                    for (const auto& option : config.boolOptions)
                    {
                        switch (option.first)
                        {
                        case Option::_444SDIVideoOutput:
                            p.dl->dlConfig.p->SetFlag(bmdDeckLinkConfig444SDIVideoOutput, option.second);
                            break;
                        default: break;
                        }
                    }
                    /*if (0)
                    {
                        BOOL value = 0;
                        p.dl->dlConfig.p->GetFlag(bmdDeckLinkConfig444SDIVideoOutput, &value);
                    }*/

                    if (p.dl->dl.p->QueryInterface(IID_IDeckLinkOutput, (void**)&p.dl->dlOutput) != S_OK)
                    {
                        throw std::runtime_error("Output device not found");
                    }

                    {
                        DLDisplayModeIteratorWrapper dlDisplayModeIterator;
                        if (p.dl->dlOutput.p->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
                        {
                            throw std::runtime_error("Cannot get display mode iterator");
                        }
                        DLDisplayModeWrapper dlDisplayMode;
                        int count = 0;
                        while (dlDisplayModeIterator.p->Next(&dlDisplayMode.p) == S_OK)
                        {
                            if (count == config.displayModeIndex)
                            {
                                break;
                            }

                            dlDisplayMode.p->Release();
                            dlDisplayMode.p = nullptr;

                            ++count;
                        }
                        if (!dlDisplayMode.p)
                        {
                            throw std::runtime_error("Display mode not found");
                        }

                        size.w = dlDisplayMode.p->GetWidth();
                        size.h = dlDisplayMode.p->GetHeight();
                        BMDTimeValue frameDuration;
                        BMDTimeScale frameTimescale;
                        dlDisplayMode.p->GetFrameRate(&frameDuration, &frameTimescale);
                        frameRate = otime::RationalTime(frameDuration, frameTimescale);
                        p.dl->audioInfo.channelCount = 2;
                        p.dl->audioInfo.dataType = audio::DataType::S16;
                        p.dl->audioInfo.sampleRate = 48000;

                        if (auto context = p.context.lock())
                        {
                            context->log(
                                "tl::device::BMDOutputDevice",
                                string::Format(
                                    "\n"
                                    "    #{0} {1}\n"
                                    "    video: {2} {3}\n"
                                    "    audio: {4} {5} {6}").
                                arg(config.deviceIndex).
                                arg(modelName).
                                arg(size).
                                arg(frameRate).
                                arg(p.dl->audioInfo.channelCount).
                                arg(p.dl->audioInfo.dataType).
                                arg(p.dl->audioInfo.sampleRate));
                        }

                        HRESULT r = p.dl->dlOutput.p->EnableVideoOutput(
                            dlDisplayMode.p->GetDisplayMode(),
                            bmdVideoOutputFlagDefault);
                        switch (r)
                        {
                        case S_OK:
                            break;
                        case E_ACCESSDENIED:
                            throw std::runtime_error("Unable to access the hardware");
                        default:
                            throw std::runtime_error("Cannot enable video output");
                        }

                        r = p.dl->dlOutput.p->EnableAudioOutput(
                            bmdAudioSampleRate48kHz,
                            bmdAudioSampleType16bitInteger,
                            p.dl->audioInfo.channelCount,
                            bmdAudioOutputStreamContinuous);
                        switch (r)
                        {
                        case S_OK:
                            break;
                        case E_INVALIDARG:
                            throw std::runtime_error("Invalid number of channels requested");
                        case E_ACCESSDENIED:
                            throw std::runtime_error("Unable to access the hardware");
                        default:
                            throw std::runtime_error("Cannot enable audio output");
                        }
                    }

                    p.dl->dlOutputCallback.p = new DLOutputCallback(
                        p.dl->dlOutput.p,
                        size,
                        config.pixelType,
                        frameRate,
                        p.dl->audioInfo);

                    if (p.dl->dlOutput.p->SetScheduledFrameCompletionCallback(p.dl->dlOutputCallback.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot set video callback");
                    }

                    if (p.dl->dlOutput.p->SetAudioCallback(p.dl->dlOutputCallback.p) != S_OK)
                    {
                        throw std::runtime_error("Cannot set audio callback");
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::device::BMDOutputDevice",
                            e.what(),
                            log::Type::Error);
                    }
                }
            }
            p.active->setIfChanged(p.dl&& p.dl->dlOutputCallback.p);
            p.size->setIfChanged(size);
            p.frameRate->setIfChanged(frameRate);
        }
    }
}
