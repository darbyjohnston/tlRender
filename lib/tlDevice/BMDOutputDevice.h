// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDDeviceData.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

namespace tl
{
    namespace device
    {
        //! BMD output device.
        class BMDOutputDevice : public std::enable_shared_from_this<BMDOutputDevice>
        {
            TLRENDER_NON_COPYABLE(BMDOutputDevice);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            BMDOutputDevice();

        public:
            ~BMDOutputDevice();

            //! Create a new BMD output device.
            static std::shared_ptr<BMDOutputDevice> create(const std::shared_ptr<system::Context>&);

            //! Get the output device configuration.
            DeviceConfig getConfig() const;

            //! Observe the output device configuration.
            std::shared_ptr<observer::IValue<DeviceConfig> > observeConfig() const;

            //! Set the output device configuration.
            void setConfig(const DeviceConfig&);

            //! Get whether the output device is enabled.
            bool isEnabled() const;

            //! Observe whether the output device is enabled.
            std::shared_ptr<observer::IValue<bool> > observeEnabled() const;

            //! Set whether the output device is enabled.
            void setEnabled(bool);

            //! Get whether the output device is active.
            bool isActive() const;

            //! Observe whether the output device is active.
            std::shared_ptr<observer::IValue<bool> > observeActive() const;

            //! Get the output device size.
            const math::Size2i& getSize() const;

            //! Observe the output device size.
            std::shared_ptr<observer::IValue<math::Size2i> > observeSize() const;

            //! Get the output device frame rate.
            const otime::RationalTime& getFrameRate() const;

            //! Observe the output device frame rate.
            std::shared_ptr<observer::IValue<otime::RationalTime> > observeFrameRate() const;

            //! Set the view.
            void setView(
                const tl::math::Vector2i& position,
                float                     zoom,
                bool                      frame);

            //! Set the OpenColorIO options.
            void setOCIOOptions(const timeline::OCIOOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<timeline::ImageOptions>&);

            //! Set the display options.
            void setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the HDR mode and metadata.
            void setHDR(device::HDRMode, const image::HDRData&);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the timeline players.
            void setPlayers(const std::vector<std::shared_ptr<timeline::Player> >&);

            //! Tick the output device.
            void tick();

        private:
            void _run();
            void _createDevice(
                const device::DeviceConfig&,
                bool& active,
                math::Size2i& size,
                otime::RationalTime& frameRate);
            void _render(
                const device::DeviceConfig&,
                const timeline::OCIOOptions&,
                const timeline::LUTOptions&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
