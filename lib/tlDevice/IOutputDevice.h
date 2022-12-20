// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/DeviceData.h>

#include <tlTimeline/TimelinePlayer.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace device
    {
        //! Base class for output devices.
        class IOutputDevice : public std::enable_shared_from_this<IOutputDevice>
        {
            TLRENDER_NON_COPYABLE(IOutputDevice);

        protected:
            void _init(
                int deviceIndex,
                int displayModeIndex,
                PixelType,
                const std::shared_ptr<system::Context>&);

            IOutputDevice();

        public:
            virtual ~IOutputDevice() = 0;

            //! Get the output device index. A value of -1 is returned if there
            //! is no output device.
            int getDeviceIndex() const;

            //! Get the output device display mode index. A value of -1 is
            //! returned if there is no display mode.
            int getDisplayModeIndex() const;

            //! Get the output device pixel type.
            PixelType getPixelType() const;

            //! Get the output device size.
            const imaging::Size& getSize() const;

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

        protected:
            int _deviceIndex = 0;
            int _displayModeIndex = 0;
            PixelType _pixelType = PixelType::None;
            imaging::Size _size;
            otime::RationalTime _frameRate;
        };
    }
}
