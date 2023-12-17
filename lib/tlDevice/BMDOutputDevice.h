// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDDeviceData.h>

#include <tlTimeline/Player.h>

#include <tlCore/Size.h>

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

        //! BMD output device.
        class BMDOutputDevice : public std::enable_shared_from_this<BMDOutputDevice>
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
            ~BMDOutputDevice();

            //! Create a new BMD output device.
            static std::shared_ptr<BMDOutputDevice> create(
                int deviceIndex,
                int displayModeIndex,
                PixelType,
                const std::shared_ptr<system::Context>&);

            //! Get the output device index. A value of -1 is returned if there
            //! is no output device.
            int getDeviceIndex() const;

            //! Get the output device display mode index. A value of -1 is
            //! returned if there is no display mode.
            int getDisplayModeIndex() const;

            //! Get the output device pixel type.
            PixelType getPixelType() const;

            //! Get the output device size.
            const math::Size2i& getSize() const;

            //! Get the output device frame rate.
            const otime::RationalTime& getFrameRate() const;

            //! Set the playback information.
            virtual void setPlayback(timeline::Playback, const otime::RationalTime&);

            //! Set the pixel data.
            virtual void setPixelData(const std::shared_ptr<PixelData>&);

            //! Set the audio volume.
            virtual void setVolume(float);

            //! Set the audio mute.
            virtual void setMute(bool);

            //! Set the audio offset.
            virtual void setAudioOffset(double);

            //! Set the audio data.
            virtual void setAudioData(const std::vector<timeline::AudioData>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
