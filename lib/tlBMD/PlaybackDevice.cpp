// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlBMD/PlaybackDevice.h>

#include <tlCore/Context.h>
#include <tlCore/StringFormat.h>

#include "platform.h"

#include <iostream>
#include <list>
#include <mutex>

namespace tl
{
    namespace bmd
    {
        namespace
        {
            class RenderDelegate : public IDeckLinkVideoOutputCallback
            {
            public:
                void display(const std::shared_ptr<imaging::Image>&);

                void setCallback(const std::function<void(IDeckLinkVideoFrame*)>&);

                HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result) override;
                HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped() override;

                HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppv) override;
                ULONG STDMETHODCALLTYPE AddRef() override;
                ULONG STDMETHODCALLTYPE Release() override;

            private:
                ULONG _refCount = 1;
                std::function<void(IDeckLinkVideoFrame*)> _callback;
            };

            void RenderDelegate::setCallback(const std::function<void(IDeckLinkVideoFrame*)>& callback)
            {
                _callback = callback;
            }

            HRESULT RenderDelegate::QueryInterface(REFIID iid, LPVOID* ppv)
            {
                *ppv = NULL;
                return E_NOINTERFACE;
            }

            ULONG RenderDelegate::AddRef()
            {
                return InterlockedIncrement((LONG*)&_refCount);
            }

            ULONG RenderDelegate::Release()
            {
                ULONG newRefValue;
                newRefValue = InterlockedDecrement((LONG*)&_refCount);
                if (newRefValue == 0)
                {
                    delete this;
                    return 0;
                }
                return newRefValue;
            }

            HRESULT	RenderDelegate::ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result)
            {
                if (_callback)
                {
                    _callback(completedFrame);
                }
                return S_OK;
            }

            HRESULT	RenderDelegate::ScheduledPlaybackHasStopped()
            {
                return S_OK;
            }
        }

        struct PlaybackDevice::Private
        {
            int deviceIndex = -1;
            IDeckLink* dl = nullptr;
            IDeckLinkOutput* dlOutput = nullptr;
            imaging::Size size;
            otime::RationalTime frameRate;
            uint64_t frameCount = 0;
            RenderDelegate* renderDelegate = nullptr;
            std::list<std::shared_ptr<imaging::Image> > images;
            std::mutex mutex;
        };

        void PlaybackDevice::_init(int deviceIndex, const std::shared_ptr<system::Context>& context)
        {
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
                        p.deviceIndex = deviceIndex;

                        dlstring_t dlModelName;
                        p.dl ->GetModelName(&dlModelName);
                        modelName = DlToStdString(dlModelName);
                        DeleteString(dlModelName);

                        break;
                    }

                    ++count;
                }
                if (!p.dl)
                {
                    throw std::runtime_error("No device found");
                }

                if (p.dl->QueryInterface(IID_IDeckLinkOutput, (void**)&p.dlOutput) != S_OK)
                {
                    throw std::runtime_error("No output device found");
                }

                p.renderDelegate = new RenderDelegate;
                p.renderDelegate->setCallback(
                    [this](IDeckLinkVideoFrame* dlVideoFrame)
                    {
                        std::shared_ptr<imaging::Image> image;
                        {
                            std::unique_lock<std::mutex> lock(_p->mutex);
                            if (!_p->images.empty())
                            {
                                image = _p->images.front();
                                _p->images.pop_front();
                            }
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

                if (p.dlOutput->SetScheduledFrameCompletionCallback(p.renderDelegate) != S_OK)
                {
                    throw std::runtime_error("Cannot set callback");
                }

                if (p.dlOutput->GetDisplayModeIterator(&dlDisplayModeIterator) != S_OK)
                {
                    throw std::runtime_error("Cannot get display mode iterator");
                }
                if (dlDisplayModeIterator->Next(&dlDisplayMode) != S_OK)
                {
                    throw std::runtime_error("No display modes");
                }
                p.size.w = dlDisplayMode->GetWidth();
                p.size.h = dlDisplayMode->GetHeight();
                BMDTimeValue frameDuration;
                BMDTimeScale frameTimescale;
                dlDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
                p.frameRate = otime::RationalTime(frameDuration, frameTimescale);

                context->log(
                    "tl::bmd::PlaybackDevice",
                    string::Format("Using device {0}: {1} {2} {3}").
                    arg(deviceIndex).
                    arg(modelName).
                    arg(p.size).
                    arg(p.frameRate));

                if (p.dlOutput->EnableVideoOutput(dlDisplayMode->GetDisplayMode(), bmdVideoOutputFlagDefault) != S_OK)
                {
                    throw std::runtime_error("Cannot enable video output");
                }

                for (int preroll = 0; preroll < 3; ++preroll)
                {
                    if (p.dlOutput->CreateVideoFrame(p.size.w, p.size.h, p.size.w * 4, bmdFormat8BitBGRA, bmdFrameFlagFlipVertical, &dlVideoFrame) != S_OK)
                    {
                        throw std::runtime_error("Cannot create video frame");
                    }
                    if (p.dlOutput->ScheduleVideoFrame(dlVideoFrame, (p.frameCount * p.frameRate.value()), p.frameRate.value(), p.frameRate.rate()) != S_OK)
                    {
                        throw std::runtime_error("Cannot schedule video frame");
                    }
                    dlVideoFrame->Release();
                    dlVideoFrame = nullptr;
                    p.frameCount = p.frameCount + 1;
                }

                p.dlOutput->StartScheduledPlayback(0, 100, 1.0);
            }
            catch (const std::exception& e)
            {
                context->log(
                    "tl::bmd::PlaybackDevice",
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

        PlaybackDevice::PlaybackDevice() :
            _p(new Private)
        {}

        PlaybackDevice::~PlaybackDevice()
        {
            TLRENDER_P();
            delete p.renderDelegate;
            p.renderDelegate = nullptr;
            if (p.dlOutput)
            {
                p.dlOutput->StopScheduledPlayback(0, nullptr, 0);
                p.dlOutput->DisableAudioOutput();
                p.dlOutput->SetScreenPreviewCallback(nullptr);
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

        std::shared_ptr<PlaybackDevice> PlaybackDevice::create(
            int deviceIndex,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<PlaybackDevice>(new PlaybackDevice);
            out->_init(deviceIndex, context);
            return out;
        }

        const imaging::Size& PlaybackDevice::getSize() const
        {
            return _p->size;
        }

        const otime::RationalTime& PlaybackDevice::getFrameRate() const
        {
            return _p->frameRate;
        }

        void PlaybackDevice::display(const std::shared_ptr<imaging::Image>& image)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.images.push_back(image);
        }
    }
}
