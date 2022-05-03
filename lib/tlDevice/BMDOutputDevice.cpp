// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDOutputDevice.h>

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
            class DLVideoOutputCallback : public IDeckLinkVideoOutputCallback
            {
            public:
                DLVideoOutputCallback();

                void setCallback(const std::function<void(IDeckLinkVideoFrame*)>&);

                HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame*, BMDOutputFrameCompletionResult) override;
                HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped() override;

                HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppv) override;
                ULONG STDMETHODCALLTYPE AddRef() override;
                ULONG STDMETHODCALLTYPE Release() override;

            private:
                std::atomic<size_t> _refCount;
                std::function<void(IDeckLinkVideoFrame*)> _callback;
            };

            DLVideoOutputCallback::DLVideoOutputCallback() :
                _refCount(1)
            {}

            void DLVideoOutputCallback::setCallback(const std::function<void(IDeckLinkVideoFrame*)>& callback)
            {
                _callback = callback;
            }

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
        }

        struct BMDOutputDevice::Private
        {
            IDeckLink* dl = nullptr;
            IDeckLinkOutput* dlOutput = nullptr;
            DLVideoOutputCallback* dlVideoOutputCallback = nullptr;

            imaging::Size size;
            otime::RationalTime frameRate;
            uint64_t frameCount = 0;
            std::shared_ptr<imaging::Image> image;
            std::mutex mutex;
        };

        void BMDOutputDevice::_init(
            int deviceIndex,
            int displayModeIndex,
            const std::shared_ptr<system::Context>& context)
        {
            IOutputDevice::_init(deviceIndex, displayModeIndex, context);

            TLRENDER_P();

            IDeckLinkIterator* dlIterator = nullptr;
            IDeckLinkDisplayModeIterator* dlDisplayModeIterator = nullptr;
            IDeckLinkDisplayMode* dlDisplayMode = nullptr;
            IDeckLinkMutableVideoFrame* dlVideoFrame = nullptr;
            try
            {
                if (GetDeckLinkIterator(&dlIterator) != S_OK)
                {
                    throw std::runtime_error("Cannot get iterator");
                }

                int count = 0;
                std::string modelName;
                while (dlIterator->Next(&p.dl) == S_OK)
                {
                    if (count == deviceIndex)
                    {
                        dlstring_t dlModelName;
                        p.dl ->GetModelName(&dlModelName);
                        modelName = DlToStdString(dlModelName);
                        DeleteString(dlModelName);

                        break;
                    }

                    p.dl->Release();
                    p.dl = nullptr;

                    ++count;
                }
                if (!p.dl)
                {
                    throw std::runtime_error("Device not found");
                }

                if (p.dl->QueryInterface(IID_IDeckLinkOutput, (void**)&p.dlOutput) != S_OK)
                {
                    throw std::runtime_error("Output device not found");
                }

                p.dlVideoOutputCallback = new DLVideoOutputCallback;
                p.dlVideoOutputCallback->setCallback(
                    [this](IDeckLinkVideoFrame* dlVideoFrame)
                    {
                        std::shared_ptr<imaging::Image> image;
                        {
                            std::unique_lock<std::mutex> lock(_p->mutex);
                            image = _p->image;
                        }
                        if (image)
                        {
                            void* dlFrame = nullptr;
                            dlVideoFrame->GetBytes((void**)&dlFrame);
                            memcpy(dlFrame, image->getData(), image->getDataByteCount());
                        }
                        if (_p->dlOutput->ScheduleVideoFrame(
                            dlVideoFrame,
                            (_p->frameCount * _p->frameRate.value()),
                            _p->frameRate.value(),
                            _p->frameRate.rate()) == S_OK)
                        {
                            _p->frameCount = _p->frameCount + 1;
                        }
                    });

                if (p.dlOutput->SetScheduledFrameCompletionCallback(p.dlVideoOutputCallback) != S_OK)
                {
                    throw std::runtime_error("Cannot set callback");
                }

                if (p.dlOutput->GetDisplayModeIterator(&dlDisplayModeIterator) != S_OK)
                {
                    throw std::runtime_error("Cannot get display mode iterator");
                }
                count = 0;
                while (dlDisplayModeIterator->Next(&dlDisplayMode) == S_OK)
                {
                    if (count == displayModeIndex)
                    {
                        break;
                    }

                    dlDisplayMode->Release();
                    dlDisplayMode = nullptr;

                    ++count;
                }
                if (!dlDisplayMode)
                {
                    throw std::runtime_error("Display mode not found");
                }

                p.size.w = dlDisplayMode->GetWidth();
                p.size.h = dlDisplayMode->GetHeight();
                BMDTimeValue frameDuration;
                BMDTimeScale frameTimescale;
                dlDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
                p.frameRate = otime::RationalTime(frameDuration, frameTimescale);

                context->log(
                    "tl::device::BMDOutputDevice",
                    string::Format("#{0} {1} {2} {3}").
                    arg(deviceIndex).
                    arg(modelName).
                    arg(p.size).
                    arg(p.frameRate));

                if (p.dlOutput->EnableVideoOutput(
                    dlDisplayMode->GetDisplayMode(),
                    bmdVideoOutputFlagDefault) != S_OK)
                {
                    throw std::runtime_error("Cannot enable video output");
                }

                for (int preroll = 0; preroll < 3; ++preroll)
                {
                    if (p.dlOutput->CreateVideoFrame(
                        p.size.w,
                        p.size.h,
                        p.size.w * 4,
                        bmdFormat8BitBGRA,
                        bmdFrameFlagFlipVertical,
                        &dlVideoFrame) != S_OK)
                    {
                        throw std::runtime_error("Cannot create video frame");
                    }
                    if (p.dlOutput->ScheduleVideoFrame(
                        dlVideoFrame,
                        (p.frameCount * p.frameRate.value()),
                        p.frameRate.value(),
                        p.frameRate.rate()) != S_OK)
                    {
                        throw std::runtime_error("Cannot schedule video frame");
                    }
                    dlVideoFrame->Release();
                    dlVideoFrame = nullptr;
                    p.frameCount = p.frameCount + 1;
                }

                p.dlOutput->StartScheduledPlayback(
                    0,
                    p.frameRate.rate(),
                    1.0);
            }
            catch (const std::exception& e)
            {
                context->log(
                    "tl::device::BMDOutputDevice",
                    e.what(),
                    log::Type::Error);
                if (p.dlOutput)
                {
                    p.dlOutput->Release();
                    p.dlOutput = nullptr;
                }
                if (p.dl)
                {
                    p.dl->Release();
                    p.dl = nullptr;
                }
            }

            if (dlVideoFrame)
            {
                dlVideoFrame->Release();
            }
            if (dlDisplayMode)
            {
                dlDisplayMode->Release();
            }
            if (dlDisplayModeIterator)
            {
                dlDisplayModeIterator->Release();
            }
            if (dlIterator)
            {
                dlIterator->Release();
            }
        }

        BMDOutputDevice::BMDOutputDevice() :
            _p(new Private)
        {}

        BMDOutputDevice::~BMDOutputDevice()
        {
            TLRENDER_P();
            if (p.dlVideoOutputCallback)
            {
                p.dlVideoOutputCallback->Release();
                p.dlVideoOutputCallback = nullptr;
            }
            if (p.dlOutput)
            {
                p.dlOutput->StopScheduledPlayback(0, nullptr, 0);
                p.dlOutput->DisableVideoOutput();
                p.dlOutput->SetScheduledFrameCompletionCallback(nullptr);
                p.dlOutput->Release();
                p.dlOutput = nullptr;
            }
            if (p.dl)
            {
                p.dl->Release();
                p.dl = nullptr;
            }
        }

        std::shared_ptr<BMDOutputDevice> BMDOutputDevice::create(
            int deviceIndex,
            int displayModeIndex,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<BMDOutputDevice>(new BMDOutputDevice);
            out->_init(deviceIndex, displayModeIndex, context);
            return out;
        }

        const imaging::Size& BMDOutputDevice::getSize() const
        {
            return _p->size;
        }

        const otime::RationalTime& BMDOutputDevice::getFrameRate() const
        {
            return _p->frameRate;
        }

        void BMDOutputDevice::display(const std::shared_ptr<imaging::Image>& image)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.image = image;
        }
    }
}
