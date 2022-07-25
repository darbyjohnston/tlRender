// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputDevice.h>

#include <tlDevice/BMDUtil.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include "platform.h"

#include <algorithm>
#include <array>
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
            const size_t preroll = 3;
            const size_t pixelDataListMax = 3;

            class DLVideoOutputCallback : public IDeckLinkVideoOutputCallback
            {
            public:
                DLVideoOutputCallback(const std::function<void(IDeckLinkVideoFrame*)>&);

                HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame*, BMDOutputFrameCompletionResult) override;
                HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped() override;

                HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppv) override;
                ULONG STDMETHODCALLTYPE AddRef() override;
                ULONG STDMETHODCALLTYPE Release() override;

            private:
                std::atomic<size_t> _refCount;
                std::function<void(IDeckLinkVideoFrame*)> _callback;
            };

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

            class DLWrapper
            {
            public:
                ~DLWrapper()
                {
                    if (p)
                    {
                        p->Release();
                    }
                }

                IDeckLink* p = nullptr;
            };

            class DLOutputWrapper
            {
            public:
                ~DLOutputWrapper()
                {
                    if (p)
                    {
                        p->SetScheduledFrameCompletionCallback(nullptr);
                        p->Release();
                    }
                }

                IDeckLinkOutput* p = nullptr;
            };

            class DLVideoOutputCallbackWrapper
            {
            public:
                ~DLVideoOutputCallbackWrapper()
                {
                    if (p)
                    {
                        p->Release();
                    }
                }

                DLVideoOutputCallback* p = nullptr;
            };

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

        struct BMDOutputDevice::Private
        {
            uint64_t frameCount = 0;

            std::list<std::shared_ptr<device::PixelData> > pixelData;
            std::shared_ptr<device::PixelData> pixelDataTmp;
            std::mutex pixelDataMutex;

            DLWrapper dl;
            DLOutputWrapper dlOutput;
            DLVideoOutputCallbackWrapper dlVideoOutputCallback;
        };

        void BMDOutputDevice::_init(
            int deviceIndex,
            int displayModeIndex,
            PixelType pixelType,
            const std::shared_ptr<system::Context>& context)
        {
            IOutputDevice::_init(deviceIndex, displayModeIndex, pixelType, context);

            TLRENDER_P();

            DLIteratorWrapper dlIterator;
            if (GetDeckLinkIterator(&dlIterator.p) != S_OK)
            {
                throw std::runtime_error("Cannot get iterator");
            }

            int count = 0;
            std::string modelName;
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

            if (p.dl.p->QueryInterface(IID_IDeckLinkOutput, (void**)&p.dlOutput) != S_OK)
            {
                throw std::runtime_error("Output device not found");
            }

            DLDisplayModeIteratorWrapper dlDisplayModeIterator;
            if (p.dlOutput.p->GetDisplayModeIterator(&dlDisplayModeIterator.p) != S_OK)
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

            if (p.dlOutput.p->EnableVideoOutput(
                dlDisplayMode.p->GetDisplayMode(),
                bmdVideoOutputFlagDefault) != S_OK)
            {
                throw std::runtime_error("Cannot enable video output");
            }

            p.dlVideoOutputCallback.p = new DLVideoOutputCallback(
                [this](IDeckLinkVideoFrame* dlVideoFrame)
                {
                    TLRENDER_P();
                    std::shared_ptr<device::PixelData> pixelData;
                    {
                        std::unique_lock<std::mutex> lock(p.pixelDataMutex);
                        if (!p.pixelData.empty())
                        {
                            p.pixelDataTmp = p.pixelData.front();
                            p.pixelData.pop_front();
                        }
                        pixelData = p.pixelDataTmp;
                    }
                    if (pixelData)
                    {
                        //std::cout << "time: " << pixelData->getTime() << std::endl;
                        void* dlFrame = nullptr;
                        dlVideoFrame->GetBytes((void**)&dlFrame);
                        memcpy(dlFrame, pixelData->getData(), pixelData->getDataByteCount());
                    }
                    if (p.dlOutput.p->ScheduleVideoFrame(
                        dlVideoFrame,
                        p.frameCount * _frameRate.value(),
                        _frameRate.value(),
                        _frameRate.rate()) == S_OK)
                    {
                        p.frameCount = p.frameCount + 1;
                    }
                });

            if (p.dlOutput.p->SetScheduledFrameCompletionCallback(p.dlVideoOutputCallback.p) != S_OK)
            {
                throw std::runtime_error("Cannot set callback");
            }

            DLVideoFrameWrapper dlVideoFrame;
            for (int i = 0; i < preroll; ++i)
            {
                if (p.dlOutput.p->CreateVideoFrame(
                    _size.w,
                    _size.h,
                    _size.w * 4,
                    toBMD(pixelType),
                    bmdFrameFlagFlipVertical,
                    &dlVideoFrame.p) != S_OK)
                {
                    throw std::runtime_error("Cannot create video frame");
                }
                if (p.dlOutput.p->ScheduleVideoFrame(
                    dlVideoFrame.p,
                    p.frameCount * _frameRate.value(),
                    _frameRate.value(),
                    _frameRate.rate()) != S_OK)
                {
                    throw std::runtime_error("Cannot schedule video frame");
                }
                dlVideoFrame.p->Release();
                dlVideoFrame.p = nullptr;
                p.frameCount = p.frameCount + 1;
            }

            p.dlOutput.p->StartScheduledPlayback(
                0,
                _frameRate.rate(),
                1.0);
        }

        BMDOutputDevice::BMDOutputDevice() :
            _p(new Private)
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
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.pixelDataMutex);
            p.pixelData.push_back(pixelData);
            while (p.pixelData.size() > pixelDataListMax)
            {
                p.pixelData.pop_front();
            }
        }
    }
}
