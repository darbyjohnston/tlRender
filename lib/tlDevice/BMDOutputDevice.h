// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/IOutputDevice.h>

#include "platform.h"

#include <list>
#include <mutex>

namespace tl
{
    namespace device
    {
        class BMDOutputDevice;

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

        class DLVideoOutputCallbackWrapper
        {
        public:
            DLVideoOutputCallback* p = nullptr;
            ~DLVideoOutputCallbackWrapper();
        };

        class DLWrapper
        {
        public:
            IDeckLink* p = nullptr;
            ~DLWrapper();
        };

        class DLOutputWrapper
        {
        public:
            IDeckLinkOutput* p = nullptr;
            ~DLOutputWrapper();
        };

        //! BMD output device.
        class BMDOutputDevice : public IOutputDevice
        {
            TLRENDER_NON_COPYABLE(BMDOutputDevice);

        protected:
            void _init(
                int deviceIndex,
                int displayModeIndex,
                PixelType,
                const std::shared_ptr<system::Context>&);

            BMDOutputDevice();

        public:
            ~BMDOutputDevice() override;

            //! Create a new BMD output device.
            static std::shared_ptr<BMDOutputDevice> create(
                int deviceIndex,
                int displayModeIndex,
                PixelType,
                const std::shared_ptr<system::Context>&);

            const imaging::Size& getSize() const override;
            const otime::RationalTime& getFrameRate() const override;
            void display(const std::shared_ptr<device::PixelData>&) override;

        private:
            uint64_t _frameCount = 0;
            imaging::Size _size;
            otime::RationalTime _frameRate;

            std::list<std::shared_ptr<device::PixelData> > _pixelData;
            std::shared_ptr<device::PixelData> _pixelDataTmp;
            std::mutex _pixelDataMutex;

            DLWrapper _dl;
            DLOutputWrapper _dlOutput;
            DLVideoOutputCallbackWrapper _dlVideoOutputCallback;
        };
    }
}
