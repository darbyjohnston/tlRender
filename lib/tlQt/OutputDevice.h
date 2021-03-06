// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/IRender.h>

#include <tlDevice/DeviceData.h>

#include <tlCore/ColorConfig.h>
#include <tlCore/Image.h>

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

            //! Set the device.
            void setDevice(
                int deviceIndex,
                int displayModeIndex,
                device::PixelType);

            //! Set the color configuration.
            void setColorConfig(const imaging::ColorConfig&);

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

        public Q_SLOTS:
            //! Set the view.
            void setView(
                const tl::math::Vector2i& position,
                float                     zoom,
                bool                      frame);

        private Q_SLOTS:
            void _videoCallback(const tl::timeline::VideoData&);

        protected:
            void run() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
