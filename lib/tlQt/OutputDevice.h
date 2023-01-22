// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/IRender.h>

#include <tlDevice/DeviceData.h>

#include <QImage>
#include <QThread>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace qt
    {
        //! Output device.
        class OutputDevice : public QThread
        {
            Q_OBJECT

        public:
            OutputDevice(
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            ~OutputDevice();

            //! Get the output device index. A value of -1 is returned if there
            //! is no output device.
            int getDeviceIndex() const;

            //! Get the output device display mode index. A value of -1 is
            //! returned if there is no display mode.
            int getDisplayModeIndex() const;

            //! Get the output device pixel type.
            device::PixelType getPixelType() const;

            //! Set the output device. If deviceIndex or displayModeIndex
            //! is set to -1, or pixelType is set to None, the output device
            //! is disabled.
            void setDevice(
                int deviceIndex,
                int displayModeIndex,
                device::PixelType pixelType);

            //! Get whether the output device is enabled.
            bool isDeviceEnabled() const;

            //! Get whether the output device is active.
            bool isDeviceActive() const;

            //! Set the color configuration options.
            void setColorConfigOptions(const timeline::ColorConfigOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<timeline::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the HDR mode and metadata.
            void setHDR(device::HDRMode, const imaging::HDRData&);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the timeline players.
            void setTimelinePlayers(const std::vector<qt::TimelinePlayer*>&);

            //! Set a QImage overlay. The output device takes ownership of
            //! the given QImage. The QImage format must be:
            //! * QImage::Format_RGBA8888
            //! * QImage::Format_ARGB4444_Premultiplied
            //! \todo Temporary
            void setOverlay(QImage*);

        public Q_SLOTS:
            //! Set whether the output device is enabled.
            void setDeviceEnabled(bool);

            //! Set the view.
            void setView(
                const tl::math::Vector2i& position,
                float                     zoom,
                bool                      frame);

            //! Set the audio volume.
            void setVolume(float);

            //! Set the audio mute.
            void setMute(bool);

            //! Set the audio offset.
            void setAudioOffset(double);

        Q_SIGNALS:
            //! This signal is emitted when the output device active state is
            //! changed.
            void deviceActiveChanged(bool);

            //! This signal is emitted when the output device size is changed.
            void sizeChanged(const tl::imaging::Size&);

            //! This signal is emitted when the output device frame rate is
            //! changed.
            void frameRateChanged(const otime::RationalTime&);

        private Q_SLOTS:
            void _playbackCallback(tl::timeline::Playback);
            void _currentTimeCallback(const otime::RationalTime&);
            void _currentVideoCallback(const tl::timeline::VideoData&);
            void _currentAudioCallback(const std::vector<tl::timeline::AudioData>&);

        protected:
            void run() override;

        private:
            bool _isDeviceActive() const;

            TLRENDER_PRIVATE();
        };
    }
}
