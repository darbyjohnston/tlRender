// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputDevice.h>

#include <tlDevice/BMDUtil.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>

namespace tl
{
    namespace device
    {
        namespace
        {
            const size_t preroll = 3;
            const size_t pixelDataListMax = 3;
        }

        DLVideoOutputCallback::DLVideoOutputCallback(
            const std::function<void(IDeckLinkVideoFrame*)>& callback) :
            _refCount(1),
            _callback(callback)
        {}

        HRESULT DLVideoOutputCallback::QueryInterface(REFIID iid, LPVOID* ppv)
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }

        ULONG DLVideoOutputCallback::AddRef()
        {
            return ++_refCount;
        }

        ULONG DLVideoOutputCallback::Release()
        {
            --_refCount;
            if (0 == _refCount)
            {
                delete this;
                return 0;
            }
            return _refCount;
        }

        HRESULT	DLVideoOutputCallback::ScheduledFrameCompleted(
            IDeckLinkVideoFrame* completedFrame,
            BMDOutputFrameCompletionResult result)
        {
            if (_callback)
            {
                _callback(completedFrame);
            }
            //std::cout << "result: " << getLabel(result) << std::endl;
            return S_OK;
        }

        HRESULT	DLVideoOutputCallback::ScheduledPlaybackHasStopped()
        {
            return S_OK;
        }

        DLWrapper::~DLWrapper()
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
                p->SetScheduledFrameCompletionCallback(nullptr);
                p->Release();
            }
        }

        DLVideoOutputCallbackWrapper::~DLVideoOutputCallbackWrapper()
        {
            if (p)
            {
                p->Release();
            }
        }

        namespace
        {
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
        }

        void BMDOutputDevice::_init(
            int deviceIndex,
            int displayModeIndex,
            PixelType pixelType,
            const std::shared_ptr<system::Context>& context)
        {
            IOutputDevice::_init(deviceIndex, displayModeIndex, pixelType, context);

            DLIteratorWrapper dlIterator;
            if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
            {
                throw std::runtime_error("Cannot get iterator");
            }

            int count = 0;
            std::string modelName;
            while (dlIterator.p->Next(&_dl.p) == S_OK)
            {
                if (count == deviceIndex)
                {
                    dlstring_t dlModelName;
                    _dl.p->GetModelName(&dlModelName);
                    modelName = DlToStdString(dlModelName);
                    DeleteString(dlModelName);

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

            if (_dl.p->QueryInterface(IID_IDeckLinkOutput, (void**)&_dlOutput) != S_OK)
            {
                throw std::runtime_error("Output device not found");
            }

            DLDisplayModeIteratorWrapper dlDisplayModeIterator;
            if (_dlOutput.p->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
            {
                throw std::runtime_error("Cannot get display mode iterator");
            }
            DLDisplayModeWrapper dlDisplayMode;
            count = 0;
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

            context->log(
                "tl::device::BMDOutputDevice",
                string::Format("#{0} {1} {2} {3}").
                arg(deviceIndex).
                arg(modelName).
                arg(_size).
                arg(_frameRate));

            if (_dlOutput.p->EnableVideoOutput(
                dlDisplayMode.p->GetDisplayMode(),
                bmdVideoOutputFlagDefault) != S_OK)
            {
                throw std::runtime_error("Cannot enable video output");
            }

            _dlVideoOutputCallback.p = new DLVideoOutputCallback(
                [this](IDeckLinkVideoFrame* dlVideoFrame)
                {
                    std::shared_ptr<device::PixelData> pixelData;
                    {
                        std::unique_lock<std::mutex> lock(_pixelDataMutex);
                        if (!_pixelData.empty())
                        {
                            _pixelDataTmp = _pixelData.front();
                            _pixelData.pop_front();
                        }
                        pixelData = _pixelDataTmp;
                    }
                    if (pixelData)
                    {
                        //std::cout << "time: " << pixelData->getTime() << std::endl;
                        void* dlFrame = nullptr;
                        dlVideoFrame->GetBytes((void**)&dlFrame);
                        memcpy(dlFrame, pixelData->getData(), pixelData->getDataByteCount());
                    }
                    if (_dlOutput.p->ScheduleVideoFrame(
                        dlVideoFrame,
                        _frameCount * _frameRate.value(),
                        _frameRate.value(),
                        _frameRate.rate()) == S_OK)
                    {
                        _frameCount = _frameCount + 1;
                    }
                });

            if (_dlOutput.p->SetScheduledFrameCompletionCallback(_dlVideoOutputCallback.p) != S_OK)
            {
                throw std::runtime_error("Cannot set callback");
            }

            DLVideoFrameWrapper dlVideoFrame;
            for (int i = 0; i < preroll; ++i)
            {
                if (_dlOutput.p->CreateVideoFrame(
                    _size.w,
                    _size.h,
                    _size.w * 4,
                    toBMD(pixelType),
                    bmdFrameFlagFlipVertical,
                    &dlVideoFrame.p) != S_OK)
                {
                    throw std::runtime_error("Cannot create video frame");
                }
                if (_dlOutput.p->ScheduleVideoFrame(
                    dlVideoFrame.p,
                    _frameCount * _frameRate.value(),
                    _frameRate.value(),
                    _frameRate.rate()) != S_OK)
                {
                    throw std::runtime_error("Cannot schedule video frame");
                }
                dlVideoFrame.p->Release();
                dlVideoFrame.p = nullptr;
                _frameCount = _frameCount + 1;
            }

            _dlOutput.p->StartScheduledPlayback(
                0,
                _frameRate.rate(),
                1.0);
        }

        BMDOutputDevice::BMDOutputDevice()
        {}

        BMDOutputDevice::~BMDOutputDevice()
        {}

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

        void BMDOutputDevice::display(const std::shared_ptr<device::PixelData>& pixelData)
        {
            std::unique_lock<std::mutex> lock(_pixelDataMutex);
            _pixelData.push_back(pixelData);
            while (_pixelData.size() > pixelDataListMax)
            {
                _pixelData.pop_front();
            }
        }
    }
}
