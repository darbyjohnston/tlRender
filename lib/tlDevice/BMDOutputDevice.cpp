// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputDevice.h>

#include <tlDevice/BMDUtil.h>

#include <tlCore/AudioConvert.h>
#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <list>
#include <mutex>
#include <tuple>

namespace tl
{
    namespace device
    {
        namespace
        {
            const size_t pixelDataMax = 3;
            const size_t audioBufferCount = 10000;

            class DLIteratorWrapper
            {
            public:
                ~DLIteratorWrapper() { if (p) p->Release(); }

                IDeckLinkIterator* p = nullptr;
            };

            class DLDisplayModeIteratorWrapper
            {
            public:
                ~DLDisplayModeIteratorWrapper() { if (p) p->Release(); }

                IDeckLinkDisplayModeIterator* p = nullptr;
            };

            class DLDisplayModeWrapper
            {
            public:
                ~DLDisplayModeWrapper() { if (p) p->Release(); }

                IDeckLinkDisplayMode* p = nullptr;
            };

            class DLVideoFrameWrapper
            {
            public:
                ~DLVideoFrameWrapper() { if (p) p->Release(); }

                IDeckLinkMutableVideoFrame* p = nullptr;
            };

            /*class DLHDRVideoFrame :
                public IDeckLinkVideoFrame,
                public IDeckLinkVideoFrameMetadataExtensions
            {
            public:
                DLHDRVideoFrame(std::shared_ptr<IDeckLinkMutableVideoFrame>& frame, imaging::HDRData& hdrData) :
                    _frame(frame),
                    _hdrData(hdrData),
                    _refCount(1)
                {}

                virtual ~DLHDRVideoFrame() {}

                HRESULT QueryInterface(REFIID iid, LPVOID* ppv) override;
                ULONG AddRef(void) override;
                ULONG Release(void) override;

                long GetWidth(void) override { return _frame->GetWidth(); }
                long GetHeight(void) override { return _frame->GetHeight(); }
                long GetRowBytes(void) override { return _frame->GetRowBytes(); }
                BMDPixelFormat GetPixelFormat(void) override { return _frame->GetPixelFormat(); }
                BMDFrameFlags GetFlags(void) override { return _frame->GetFlags() | bmdFrameContainsHDRMetadata; }
                HRESULT GetBytes(void** buffer) override { return _frame->GetBytes(buffer); }
                HRESULT GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode** timecode) override { return _frame->GetTimecode(format, timecode); }
                HRESULT GetAncillaryData(IDeckLinkVideoFrameAncillary** ancillary) override { return _frame->GetAncillaryData(ancillary); }

                HRESULT GetInt(BMDDeckLinkFrameMetadataID metadataID, int64_t* value) override;
                HRESULT GetFloat(BMDDeckLinkFrameMetadataID metadataID, double* value) override;
                HRESULT GetFlag(BMDDeckLinkFrameMetadataID metadataID, BOOL* value) override;
                HRESULT GetString(BMDDeckLinkFrameMetadataID metadataID, BSTR* value) override;
                HRESULT GetBytes(BMDDeckLinkFrameMetadataID metadataID, void* buffer, uint32_t* bufferSize) override;

                void UpdateHDRMetadata(const imaging::HDRData& metadata) { _hdrData = metadata; }

            private:
                std::shared_ptr<IDeckLinkMutableVideoFrame> _frame;
                imaging::HDRData _hdrData;
                std::atomic<ULONG> _refCount;
            };

            HRESULT DLHDRVideoFrame::QueryInterface(REFIID iid, LPVOID* ppv)
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
        }

        DLWrapper::~DLWrapper()
        {
            if (p)
            {
                p->Release();
            }
        }

        DLConfigWrapper::~DLConfigWrapper()
        {
            if (p)
            {
                p->Release();
            }
        }

        DLOutputWrapper::~DLOutputWrapper()
        {
            if (p)
            {
                p->Release();
            }
        }

        struct DLOutputCallback::Private
        {
            IDeckLinkOutput* dlOutput = nullptr;
            imaging::Size size;
            PixelType pixelType = PixelType::None;
            otime::RationalTime frameRate = time::invalidTime;
            audio::Info audioInfo;

            std::atomic<size_t> refCount;

            struct PixelDataMutexData
            {
                std::list<std::shared_ptr<device::PixelData> > pixelData;
            };
            PixelDataMutexData pixelDataMutexData;
            std::mutex pixelDataMutex;

            struct PixelDataThreadData
            {
                std::shared_ptr<device::PixelData> pixelDataTmp;
                uint64_t frameCount = 0;
            };
            PixelDataThreadData pixelDataThreadData;

            struct AudioMutexData
            {
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime startTime = time::invalidTime;
                otime::RationalTime currentTime = time::invalidTime;
                float volume = 1.F;
                bool mute = false;
                std::vector<timeline::AudioData> audioData;
            };
            AudioMutexData audioMutexData;
            std::mutex audioMutex;

            struct AudioThreadData
            {
                timeline::Playback playback = timeline::Playback::Stop;
                otime::RationalTime startTime = time::invalidTime;
                size_t samplesOffset = 0;
                std::shared_ptr<audio::AudioConvert> convert;
            };
            AudioThreadData audioThreadData;
        };

        DLOutputCallback::DLOutputCallback(
            IDeckLinkOutput* dlOutput,
            const imaging::Size& size,
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
                    p.size.w * 4,
                    toBMD(pixelType),
                    bmdFrameFlagFlipVertical,
                    &dlVideoFrame.p) != S_OK)
                {
                    throw std::runtime_error("Cannot create video frame");
                }
                if (p.dlOutput->ScheduleVideoFrame(
                    dlVideoFrame.p,
                    p.pixelDataThreadData.frameCount * p.frameRate.value(),
                    p.frameRate.value(),
                    p.frameRate.rate()) != S_OK)
                {
                    throw std::runtime_error("Cannot schedule video frame");
                }
                p.pixelDataThreadData.frameCount = p.pixelDataThreadData.frameCount + 1;
            }

            p.dlOutput->StartScheduledPlayback(
                0,
                p.frameRate.rate(),
                1.0);
        }

        void DLOutputCallback::setPlayback(timeline::Playback value, const otime::RationalTime& time)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex);
            if (value != p.audioMutexData.playback)
            {
                p.dlOutput->FlushBufferedAudioSamples();
                p.audioMutexData.playback = value;
                p.audioMutexData.startTime = time;
                p.audioMutexData.currentTime = time;
            }
        }

        void DLOutputCallback::setPixelData(const std::shared_ptr<device::PixelData>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.pixelDataMutex);
                p.pixelDataMutexData.pixelData.push_back(value);
                while (p.pixelDataMutexData.pixelData.size() > pixelDataMax)
                {
                    p.pixelDataMutexData.pixelData.pop_front();
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.audioMutex);
                if (value->getTime() != p.audioMutexData.currentTime)
                {
                    const otime::RationalTime currentTimePlusOne(
                        p.audioMutexData.currentTime.value() + 1.0,
                        p.audioMutexData.currentTime.rate());
                    if (value->getTime() != currentTimePlusOne)
                    {
                        p.audioMutexData.startTime = value->getTime();
                    }
                    p.audioMutexData.currentTime = value->getTime();
                }
            }
        }

        void DLOutputCallback::setAudio(float volume, bool mute)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex);
            p.audioMutexData.volume = volume;
            p.audioMutexData.mute = mute;
        }

        void DLOutputCallback::setAudioData(const std::vector<timeline::AudioData>& value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.audioMutex);
            p.audioMutexData.audioData = value;
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
                std::unique_lock<std::mutex> lock(p.pixelDataMutex);
                if (!p.pixelDataMutexData.pixelData.empty())
                {
                    p.pixelDataThreadData.pixelDataTmp = p.pixelDataMutexData.pixelData.front();
                    p.pixelDataMutexData.pixelData.pop_front();
                }
            }
            if (p.pixelDataThreadData.pixelDataTmp)
            {
                //std::cout << "video time: " <<
                //    p.pixelDataThreadData.pixelDataTmp->getTime().rescaled_to(1.0) << std::endl;
                void* dlFrame = nullptr;
                dlVideoFrame->GetBytes((void**)&dlFrame);
                memcpy(
                    dlFrame,
                    p.pixelDataThreadData.pixelDataTmp->getData(),
                    p.pixelDataThreadData.pixelDataTmp->getDataByteCount());
            }
            p.dlOutput->ScheduleVideoFrame(
                dlVideoFrame,
                p.pixelDataThreadData.frameCount * p.frameRate.value(),
                p.frameRate.value(),
                p.frameRate.rate());
            p.pixelDataThreadData.frameCount += 1;
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
            std::vector<timeline::AudioData> audioDataList;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex);
                if (p.audioMutexData.playback != p.audioThreadData.playback ||
                    p.audioMutexData.startTime != p.audioThreadData.startTime)
                {
                    p.audioThreadData.playback = p.audioMutexData.playback;
                    p.audioThreadData.startTime = p.audioMutexData.startTime;
                    p.audioThreadData.samplesOffset = 0;
                }
                currentTime = p.audioMutexData.currentTime;
                volume = p.audioMutexData.volume;
                mute = p.audioMutexData.mute;
                audioDataList = p.audioMutexData.audioData;
            }
            //std::cout << "audio playback: " << p.audioThreadData.playback << std::endl;
            //std::cout << "audio start time: " << p.audioThreadData.startTime << std::endl;
            //std::cout << "audio samples offset: " << p.audioThreadData.samplesOffset << std::endl;

            // Flush the audio converter and BMD buffer when the playback
            // is reset.
            if (0 == p.audioThreadData.samplesOffset)
            {
                if (p.audioThreadData.convert)
                {
                    p.audioThreadData.convert->flush();
                }
                p.dlOutput->FlushBufferedAudioSamples();
            }

            // Create the audio converter.
            audio::Info inputInfo;
            if (!audioDataList.empty() &&
                !audioDataList[0].layers.empty() &&
                audioDataList[0].layers[0].audio)
            {
                inputInfo = audioDataList[0].layers[0].audio->getInfo();
                if (!p.audioThreadData.convert ||
                    (p.audioThreadData.convert && p.audioThreadData.convert->getInputInfo() != inputInfo))
                {
                    p.audioThreadData.convert = audio::AudioConvert::create(inputInfo, p.audioInfo);
                }
            }

            // Copy audio data to BMD.
            if (timeline::Playback::Forward == p.audioThreadData.playback &&
                p.audioThreadData.convert)
            {
                int64_t frame =
                    p.audioThreadData.startTime.rescaled_to(inputInfo.sampleRate).value() +
                    p.audioThreadData.samplesOffset;
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
                            std::unique_lock<std::mutex> lock(p.audioMutex);
                            p.audioMutexData.startTime = currentTime;
                        }
                        p.audioThreadData.startTime = currentTime;
                        p.audioThreadData.samplesOffset = 0;
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

                    const size_t size = std::min(audioBufferCount, inputInfo.sampleRate - offset);
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

                    auto convertedAudio = p.audioThreadData.convert->convert(tmpAudio);
                    p.dlOutput->ScheduleAudioSamples(
                        convertedAudio->getData(),
                        convertedAudio->getSampleCount(),
                        0,
                        0,
                        nullptr);

                    offset += size;
                    if (offset >= inputInfo.sampleRate)
                    {
                        offset -= inputInfo.sampleRate;
                        seconds += 1;
                    }

                    p.audioThreadData.samplesOffset += size;

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

        DLOutputCallbackWrapper::~DLOutputCallbackWrapper()
        {
            if (p)
            {
                p->Release();
            }
        }

        struct BMDOutputDevice::Private
        {
            DLWrapper dl;
            DLConfigWrapper dlConfig;
            DLOutputWrapper dlOutput;
            audio::Info audioInfo;
            DLOutputCallbackWrapper dlOutputCallback;
        };

        void BMDOutputDevice::_init(
            int deviceIndex,
            int displayModeIndex,
            PixelType pixelType,
            const std::shared_ptr<system::Context>& context)
        {
            IOutputDevice::_init(deviceIndex, displayModeIndex, pixelType, context);

            TLRENDER_P();

            std::string modelName;
            {
                DLIteratorWrapper dlIterator;
                if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
                {
                    throw std::runtime_error("Cannot get iterator");
                }

                int count = 0;
                while (dlIterator.p->Next(&p.dl.p) == S_OK)
                {
                    if (count == deviceIndex)
                    {
#if defined(__APPLE__)
                        CFStringRef dlModelName;
                        p.dl.p->GetModelName(&dlModelName);
                        StringToStdString(dlModelName, modelName);
                        CFRelease(dlModelName);
#else // __APPLE__
                        dlstring_t dlModelName;
                        p.dl.p->GetModelName(&dlModelName);
                        modelName = DlToStdString(dlModelName);
                        DeleteString(dlModelName);
#endif // __APPLE__

                        break;
                    }

                    p.dl.p->Release();
                    p.dl.p = nullptr;

                    ++count;
                }
                if (!p.dl.p)
                {
                    throw std::runtime_error("Device not found");
                }
            }

            if (p.dl.p->QueryInterface(IID_IDeckLinkConfiguration, (void**)&p.dlConfig) != S_OK)
            {
                throw std::runtime_error("Configuration device not found");
            }

            if (p.dl.p->QueryInterface(IID_IDeckLinkOutput, (void**)&p.dlOutput) != S_OK)
            {
                throw std::runtime_error("Output device not found");
            }

            {
                DLDisplayModeIteratorWrapper dlDisplayModeIterator;
                if (p.dlOutput.p->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
                {
                    throw std::runtime_error("Cannot get display mode iterator");
                }
                DLDisplayModeWrapper dlDisplayMode;
                int count = 0;
                while (dlDisplayModeIterator.p->Next(&dlDisplayMode.p) == S_OK)
                {
                    if (count == displayModeIndex)
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

                _size.w = dlDisplayMode.p->GetWidth();
                _size.h = dlDisplayMode.p->GetHeight();
                BMDTimeValue frameDuration;
                BMDTimeScale frameTimescale;
                dlDisplayMode.p->GetFrameRate(&frameDuration, &frameTimescale);
                _frameRate = otime::RationalTime(frameDuration, frameTimescale);
                p.audioInfo.channelCount = 2;
                p.audioInfo.dataType = audio::DataType::S16;
                p.audioInfo.sampleRate = 48000;

                context->log(
                    "tl::device::BMDOutputDevice",
                    string::Format(
                        "\n"
                        "    #{0} {1}\n"
                        "    video: {2} {3}\n"
                        "    audio: {4} {5} {6}").
                    arg(deviceIndex).
                    arg(modelName).
                    arg(_size).
                    arg(_frameRate).
                    arg(p.audioInfo.channelCount).
                    arg(p.audioInfo.dataType).
                    arg(p.audioInfo.sampleRate));

                HRESULT r = p.dlOutput.p->EnableVideoOutput(
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

                r = p.dlOutput.p->EnableAudioOutput(
                    bmdAudioSampleRate48kHz,
                    bmdAudioSampleType16bitInteger,
                    p.audioInfo.channelCount,
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

            p.dlOutputCallback.p = new DLOutputCallback(p.dlOutput.p, _size, _pixelType, _frameRate, p.audioInfo);

            if (p.dlOutput.p->SetScheduledFrameCompletionCallback(p.dlOutputCallback.p) != S_OK)
            {
                throw std::runtime_error("Cannot set video callback");
            }

            if (p.dlOutput.p->SetAudioCallback(p.dlOutputCallback.p) != S_OK)
            {
                throw std::runtime_error("Cannot set audio callback");
            }
        }

        BMDOutputDevice::BMDOutputDevice() :
            _p(new Private)
        {}

        BMDOutputDevice::~BMDOutputDevice()
        {
            TLRENDER_P();
            p.dlOutput.p->StopScheduledPlayback(0, nullptr, 0);
            p.dlOutput.p->DisableVideoOutput();
            p.dlOutput.p->DisableAudioOutput();
        }

        std::shared_ptr<BMDOutputDevice> BMDOutputDevice::create(
            int deviceIndex,
            int displayModeIndex,
            PixelType pixelType,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<BMDOutputDevice>(new BMDOutputDevice);
            out->_init(deviceIndex, displayModeIndex, pixelType, context);
            return out;
        }

        void BMDOutputDevice::setPlayback(timeline::Playback value, const otime::RationalTime& time)
        {
            _p->dlOutputCallback.p->setPlayback(value, time);
        }

        void BMDOutputDevice::setPixelData(const std::shared_ptr<device::PixelData>& value)
        {
            _p->dlOutputCallback.p->setPixelData(value);
        }

        void BMDOutputDevice::setAudio(float volume, bool mute)
        {
            _p->dlOutputCallback.p->setAudio(volume, mute);
        }

        void BMDOutputDevice::setAudioData(const std::vector<timeline::AudioData>& value)
        {
            _p->dlOutputCallback.p->setAudioData(value);
        }
    }
}
