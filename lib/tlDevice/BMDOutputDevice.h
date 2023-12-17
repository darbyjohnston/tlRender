// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDDeviceData.h>

#include <tlTimeline/Player.h>

#include <tlCore/Size.h>

namespace tl
{
    namespace device
    {
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
