// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDOutputDevice.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include "platform.h"

#if defined(__APPLE__)
typedef int64_t LONGLONG;
#elif defined(__linux__)
typedef bool BOOL;
typedef int64_t LONGLONG;
#endif // __APPLE__

namespace tl
{
    namespace device
    {
        //! Decklink wrapper.
        class DLWrapper
        {
        public:
            ~DLWrapper();

            IDeckLink* p = nullptr;
        };

        //! Decklink configuration wrapper.
        class DLConfigWrapper
        {
        public:
            ~DLConfigWrapper();

            IDeckLinkConfiguration* p = nullptr;
        };

        //! Decklink output wrapper.
        class DLOutputWrapper
        {
        public:
            ~DLOutputWrapper();

            IDeckLinkOutput* p = nullptr;
        };

        //! Decklink output callback.
        class DLOutputCallback :
            public IDeckLinkVideoOutputCallback,
            public IDeckLinkAudioOutputCallback
        {
        public:
            DLOutputCallback(
                IDeckLinkOutput*,
                const math::Size2i& size,
                PixelType pixelType,
                const otime::RationalTime& frameRate,
                const audio::Info& audioInfo);

            void setPlayback(timeline::Playback, const otime::RationalTime&);
            void setPixelData(const std::shared_ptr<device::PixelData>&);
            void setVolume(float);
            void setMute(bool);
            void setAudioOffset(double);
            void setAudioData(const std::vector<timeline::AudioData>&);

            HRESULT STDMETHODCALLTYPE ScheduledFrameCompleted(IDeckLinkVideoFrame*, BMDOutputFrameCompletionResult) override;
            HRESULT STDMETHODCALLTYPE ScheduledPlaybackHasStopped() override;

            HRESULT STDMETHODCALLTYPE RenderAudioSamples(BOOL preroll) override;

            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID* ppv) override;
            ULONG STDMETHODCALLTYPE AddRef() override;
            ULONG STDMETHODCALLTYPE Release() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Decklink output callback wrapper.
        class DLOutputCallbackWrapper
        {
        public:
            ~DLOutputCallbackWrapper();
            
            DLOutputCallback* p = nullptr;
        };
    }
}
