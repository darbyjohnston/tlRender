// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputDevice.h>

#include <tlDevice/BMDUtil.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <iostream>
#include <tuple>

namespace tl
{
    namespace device
    {
        namespace
        {
            const size_t pixelDataMax = 3;
            const size_t audioBufferSize = 1000;
            const size_t audioBufferMax = 48000;

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

        DLOutputCallback::DLOutputCallback(
            IDeckLinkOutput* dlOutput,
            const imaging::Size& size,
            PixelType pixelType,
            const otime::RationalTime& frameRate,
            const audio::Info& audioInfo) :
            _dlOutput(dlOutput),
            _size(size),
            _pixelType(pixelType),
            _frameRate(frameRate),
            _audioInfo(audioInfo),
            _refCount(1)
        {
            _audioThreadData.outputBuffer.resize(audioBufferSize * _audioInfo.getByteCount());

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

            _dlOutput->BeginAudioPreroll();
            /*std::vector<uint8_t> emptyAudio(audioBufferChunkSize * audioChannelCount * audio::getByteCount(audioDataType), 0);
            uint32_t audioSamplesWritten = 0;
            _dlOutput->ScheduleAudioSamples(
                emptyAudio.data(),
                6000,
                0,
                0,
                nullptr);*/
            _dlOutput->EndAudioPreroll();

            for (size_t i = 0; i < videoPreroll; ++i)
            {
                DLVideoFrameWrapper dlVideoFrame;
                if (_dlOutput->CreateVideoFrame(
                    _size.w,
                    _size.h,
                    _size.w * 4,
                    toBMD(pixelType),
                    bmdFrameFlagFlipVertical,
                    &dlVideoFrame.p) != S_OK)
                {
                    throw std::runtime_error("Cannot create video frame");
                }
                if (_dlOutput->ScheduleVideoFrame(
                    dlVideoFrame.p,
                    _pixelDataThreadData.frameCount * _frameRate.value(),
                    _frameRate.value(),
                    _frameRate.rate()) != S_OK)
                {
                    throw std::runtime_error("Cannot schedule video frame");
                }
                _pixelDataThreadData.frameCount = _pixelDataThreadData.frameCount + 1;
            }

            _dlOutput->StartScheduledPlayback(
                0,
                _frameRate.rate(),
                1.0);
        }

        void DLOutputCallback::setPlayback(timeline::Playback value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex);
            if (value != _audioMutexData.playback)
            {
                _dlOutput->FlushBufferedAudioSamples();
                _audioMutexData.playback = value;
            }
        }

        void DLOutputCallback::pixelData(const std::shared_ptr<device::PixelData>& value)
        {
            {
                std::unique_lock<std::mutex> lock(_pixelDataMutex);
                _pixelDataMutexData.pixelData.push_back(value);
                while (_pixelDataMutexData.pixelData.size() > pixelDataMax)
                {
                    _pixelDataMutexData.pixelData.pop_front();
                }
            }
            {
                std::unique_lock<std::mutex> lock(_audioMutex);
                if (value->getTime() != _audioMutexData.currentTime)
                {
                    const otime::RationalTime currentTimePlusOne(
                        _audioMutexData.currentTime.value() + 1.0,
                        _audioMutexData.currentTime.rate());
                    if (value->getTime() != currentTimePlusOne)
                    {
                        _dlOutput->FlushBufferedAudioSamples();
                    }
                    _audioMutexData.currentTime = value->getTime();
                }
            }
        }

        void DLOutputCallback::setVolume(float value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex);
            _audioMutexData.volume = value;
        }

        void DLOutputCallback::setMute(bool value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex);
            _audioMutexData.mute = value;
        }

        void DLOutputCallback::audioData(const std::vector<timeline::AudioData>& value)
        {
            std::unique_lock<std::mutex> lock(_audioMutex);
            _audioMutexData.audioData = value;
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
            ULONG out = --_refCount;
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
                std::unique_lock<std::mutex> lock(_pixelDataMutex);
                if (!_pixelDataMutexData.pixelData.empty())
                {
                    _pixelDataThreadData.pixelDataTmp = _pixelDataMutexData.pixelData.front();
                    _pixelDataMutexData.pixelData.pop_front();
                }
            }
            if (_pixelDataThreadData.pixelDataTmp)
            {
                //std::cout << "video time: " << _pixelDataThreadData.pixelDataTmp->getTime().rescaled_to(1.0) << std::endl;
                void* dlFrame = nullptr;
                dlVideoFrame->GetBytes((void**)&dlFrame);
                memcpy(
                    dlFrame,
                    _pixelDataThreadData.pixelDataTmp->getData(),
                    _pixelDataThreadData.pixelDataTmp->getDataByteCount());
            }
            if (_dlOutput->ScheduleVideoFrame(
                dlVideoFrame,
                _pixelDataThreadData.frameCount * _frameRate.value(),
                _frameRate.value(),
                _frameRate.rate()) == S_OK)
            {
                _pixelDataThreadData.frameCount = _pixelDataThreadData.frameCount + 1;
            }
            //std::cout << "result: " << getOutputFrameCompletionResultLabel(dlResult) << std::endl;
            return S_OK;
        }

        HRESULT DLOutputCallback::ScheduledPlaybackHasStopped()
        {
            return S_OK;
        }

        HRESULT DLOutputCallback::RenderAudioSamples(BOOL preroll)
        {
            float volume = 1.F;
            bool mute = false;
            std::vector<timeline::AudioData> audioData;
            {
                std::unique_lock<std::mutex> lock(_audioMutex);
                if (_audioMutexData.playback != _audioThreadData.playback)
                {
                    _audioThreadData.playback = _audioMutexData.playback;
                    _audioThreadData.startTime = _audioMutexData.currentTime;
                    _audioThreadData.offset = 0;
                }
                if (_audioMutexData.currentTime != _audioThreadData.currentTime)
                {
                    const otime::RationalTime currentTimePlusOne(
                        _audioThreadData.currentTime.value() + 1.0,
                        _audioThreadData.currentTime.rate());
                    if (_audioMutexData.currentTime != currentTimePlusOne)
                    {
                        _audioThreadData.startTime = _audioMutexData.currentTime;
                        _audioThreadData.offset = 0;
                    }
                    _audioThreadData.currentTime = _audioMutexData.currentTime;
                }
                volume = _audioMutexData.volume;
                mute = _audioMutexData.mute;
                audioData = _audioMutexData.audioData;
            }
            //std::cout << "audio playback: " << _audioThreadData.playback << std::endl;
            //std::cout << "audio start time: " << _audioThreadData.startTime << std::endl;

            if (timeline::Playback::Forward == _audioThreadData.playback)
            {
                uint32_t sampleCount = 0;
                if (_dlOutput->GetBufferedAudioSampleFrameCount(&sampleCount) != S_OK)
                {
                    return E_FAIL;
                }
                //std::cout << "audio sample count: " << sampleCount << std::endl;

                if (sampleCount < audioBufferMax)
                {
                    const otime::RationalTime audioTime = _audioThreadData.startTime.rescaled_to(1.0) +
                        otime::RationalTime(_audioThreadData.offset, _audioInfo.sampleRate).rescaled_to(1.0);
                    //std::cout << "audio time: " << audioTime << std::endl;
                    //std::cout << "audio offset: " << _audioThreadData.offset << std::endl;
                    const int64_t audioSeconds = std::floor(audioTime.value());
                    //std::cout << "audio seconds: " << audioSeconds << std::endl;
                    const size_t samplesOffset =
                        otime::RationalTime(audioTime.value() - audioSeconds, 1.0).
                        rescaled_to(_audioInfo.sampleRate).value();
                    //std::cout << "audio samples offset: " << samplesOffset << std::endl;
                    size_t samplesSize = audioBufferSize;
                    if (samplesSize > audioBufferMax - samplesOffset)
                    {
                        samplesSize = audioBufferMax - samplesOffset;
                    }
                    //std::cout << "audio samples size: " << samplesSize << std::endl;

                    memset(
                        _audioThreadData.outputBuffer.data(),
                        0,
                        audioBufferSize * _audioInfo.getByteCount());

                    for (const auto& i : audioData)
                    {
                        //std::cout << "audio data: " << i.seconds << std::endl;
                        if (audioSeconds == i.seconds)
                        {
                            std::vector<const uint8_t*> audioDataP;
                            for (const auto& layer : i.layers)
                            {
                                if (layer.audio && layer.audio->getInfo() == _audioInfo)
                                {
                                    audioDataP.push_back(layer.audio->getData() +
                                        samplesOffset * _audioInfo.getByteCount());
                                }
                            }

                            audio::mix(
                                audioDataP.data(),
                                audioDataP.size(),
                                _audioThreadData.outputBuffer.data(),
                                mute ? 0.F : volume,
                                samplesSize,
                                _audioInfo.channelCount,
                                _audioInfo.dataType);

                            break;
                        }
                    }

                    if (_dlOutput->ScheduleAudioSamples(
                        _audioThreadData.outputBuffer.data(),
                        samplesSize,
                        0,
                        0,
                        nullptr) != S_OK)
                    {
                        return E_FAIL;
                    }

                    _audioThreadData.offset += samplesSize;
                }
            }
            //std::cout << std::endl;

            //BMDTimeScale dlTimeScale = audioSampleRate;
            //BMDTimeValue dlTimeValue = 0;
            //if (_dlOutput->GetScheduledStreamTime(dlTimeScale, &dlTimeValue, nullptr) == S_OK)
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

        void BMDOutputDevice::_init(
            int deviceIndex,
            int displayModeIndex,
            PixelType pixelType,
            const std::shared_ptr<system::Context>& context)
        {
            IOutputDevice::_init(deviceIndex, displayModeIndex, pixelType, context);

            std::string modelName;
            {
                DLIteratorWrapper dlIterator;
                if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
                {
                    throw std::runtime_error("Cannot get iterator");
                }

                int count = 0;
                while (dlIterator.p->Next(&_dl.p) == S_OK)
                {
                    if (count == deviceIndex)
                    {
#if defined(__APPLE__)
                        CFStringRef dlModelName;
                        _dl.p->GetModelName(&dlModelName);
                        StringToStdString(dlModelName, modelName);
                        CFRelease(dlModelName);
#else // __APPLE__
                        dlstring_t dlModelName;
                        _dl.p->GetModelName(&dlModelName);
                        modelName = DlToStdString(dlModelName);
                        DeleteString(dlModelName);
#endif // __APPLE__

                        break;
                    }

                    _dl.p->Release();
                    _dl.p = nullptr;

                    ++count;
                }
                if (!_dl.p)
                {
                    throw std::runtime_error("Device not found");
                }
            }

            if (_dl.p->QueryInterface(IID_IDeckLinkConfiguration, (void**)&_dlConfig) != S_OK)
            {
                throw std::runtime_error("Configuration device not found");
            }

            if (_dl.p->QueryInterface(IID_IDeckLinkOutput, (void**)&_dlOutput) != S_OK)
            {
                throw std::runtime_error("Output device not found");
            }

            {
                DLDisplayModeIteratorWrapper dlDisplayModeIterator;
                if (_dlOutput.p->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
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
                _audioInfo.channelCount = 2;
                _audioInfo.dataType = audio::DataType::S16;
                _audioInfo.sampleRate = 48000;

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
                    arg(_audioInfo.channelCount).
                    arg(_audioInfo.dataType).
                    arg(_audioInfo.sampleRate));

                HRESULT r = _dlOutput.p->EnableVideoOutput(
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

                r = _dlOutput.p->EnableAudioOutput(
                    bmdAudioSampleRate48kHz,
                    bmdAudioSampleType16bitInteger,
                    _audioInfo.channelCount,
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

            _dlOutputCallback.p = new DLOutputCallback(_dlOutput.p, _size, _pixelType, _frameRate, _audioInfo);

            if (_dlOutput.p->SetScheduledFrameCompletionCallback(_dlOutputCallback.p) != S_OK)
            {
                throw std::runtime_error("Cannot set video callback");
            }

            if (_dlOutput.p->SetAudioCallback(_dlOutputCallback.p) != S_OK)
            {
                throw std::runtime_error("Cannot set audio callback");
            }
        }

        BMDOutputDevice::BMDOutputDevice()
        {}

        BMDOutputDevice::~BMDOutputDevice()
        {
            _dlOutput.p->StopScheduledPlayback(0, nullptr, 0);
            _dlOutput.p->DisableVideoOutput();
            _dlOutput.p->DisableAudioOutput();
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

        void BMDOutputDevice::setPlayback(timeline::Playback value)
        {
            _dlOutputCallback.p->setPlayback(value);
        }

        void BMDOutputDevice::pixelData(const std::shared_ptr<device::PixelData>& value)
        {
            _dlOutputCallback.p->pixelData(value);
        }

        void BMDOutputDevice::setVolume(float value)
        {
            _dlOutputCallback.p->setVolume(value);
        }

        void BMDOutputDevice::setMute(bool value)
        {
            _dlOutputCallback.p->setMute(value);
        }

        void BMDOutputDevice::audioData(const std::vector<timeline::AudioData>& value)
        {
            _dlOutputCallback.p->audioData(value);
        }
    }
}
