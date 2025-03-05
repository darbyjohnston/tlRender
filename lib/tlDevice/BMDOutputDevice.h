// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDData.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

namespace tl
{
    namespace bmd
    {
        //! Frame rate.
        struct FrameRate
        {
            int num = 0;
            int den = 0;

            bool operator == (const FrameRate&) const;
            bool operator != (const FrameRate&) const;
        };

        //! BMD output device.
        class OutputDevice : public std::enable_shared_from_this<OutputDevice>
        {
            DTK_NON_COPYABLE(OutputDevice);

        protected:
            void _init(const std::shared_ptr<dtk::Context>&);

            OutputDevice();

        public:
            ~OutputDevice();

            //! Create a new output device.
            static std::shared_ptr<OutputDevice> create(const std::shared_ptr<dtk::Context>&);

            //! Get the device configuration.
            DeviceConfig getConfig() const;

            //! Observe the device configuration.
            std::shared_ptr<dtk::IObservableValue<DeviceConfig> > observeConfig() const;

            //! Set the device configuration.
            void setConfig(const DeviceConfig&);

            //! Get whether the device is enabled.
            bool isEnabled() const;

            //! Observe whether the device is enabled.
            std::shared_ptr<dtk::IObservableValue<bool> > observeEnabled() const;

            //! Set whether the device is enabled.
            void setEnabled(bool);

            //! Get whether the device is active.
            bool isActive() const;

            //! Observe whether the device is active.
            std::shared_ptr<dtk::IObservableValue<bool> > observeActive() const;

            //! Get the video size.
            const dtk::Size2I& getSize() const;

            //! Observe the video size.
            std::shared_ptr<dtk::IObservableValue<dtk::Size2I> > observeSize() const;

            //! Get the frame rate.
            const FrameRate& getFrameRate() const;

            //! Observe the frame rate.
            std::shared_ptr<dtk::IObservableValue<FrameRate> > observeFrameRate() const;

            //! Get the video frame delay.
            int getVideoFrameDelay() const;

            //! Observe the video frame delay.
            std::shared_ptr<dtk::IObservableValue<int> > observeVideoFrameDelay() const;

            //! Set the view.
            void setView(
                const dtk::V2I& position,
                double          zoom,
                bool            frame);

            //! Set the OpenColorIO options.
            void setOCIOOptions(const timeline::OCIOOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<dtk::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the HDR mode and metadata.
            void setHDR(HDRMode, const image::HDRData&);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Set the foreground options.
            void setForegroundOptions(const timeline::ForegroundOptions&);

            //! Set the overlay.
            void setOverlay(const std::shared_ptr<dtk::Image>&);

            //! Set the audio volume.
            void setVolume(float);

            //! Set the audio mute.
            void setMute(bool);

            //! Set the audio channels mute.
            void setChannelMute(const std::vector<bool>&);

            //! Set the audio sync offset.
            void setAudioOffset(double);

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Tick the output device.
            void tick();

        private:
            void _run();
            void _createDevice(
                const DeviceConfig&,
                bool& active,
                dtk::Size2I& size,
                FrameRate& frameRate,
                int videoFrameDelay);
            void _render(
                const DeviceConfig&,
                const timeline::OCIOOptions&,
                const timeline::LUTOptions&,
                const std::vector<dtk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                const timeline::BackgroundOptions&,
                const timeline::ForegroundOptions&);
            void _read();

            DTK_PRIVATE();
        };
    }
}
