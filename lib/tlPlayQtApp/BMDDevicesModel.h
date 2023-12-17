// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDDeviceData.h>

#include <tlCore/Image.h>
#include <tlCore/ValueObserver.h>

#include <memory>
#include <string>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play_qt
    {
        //! BMD devices model data.
        struct BMDDevicesModelData
        {
            std::vector<std::string> devices;
            int deviceIndex = 0;
            std::vector<std::string> displayModes;
            int displayModeIndex = 0;
            std::vector<device::PixelType> pixelTypes;
            int pixelTypeIndex = 0;
            bool deviceEnabled = true;
            image::VideoLevels videoLevels = image::VideoLevels::LegalRange;
            device::HDRMode hdrMode = device::HDRMode::FromFile;
            image::HDRData hdrData;

            bool operator == (const BMDDevicesModelData&) const;
        };

        //! BMD devices model.
        class BMDDevicesModel : public std::enable_shared_from_this<BMDDevicesModel>
        {
            TLRENDER_NON_COPYABLE(BMDDevicesModel);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&);

            BMDDevicesModel();

        public:
            ~BMDDevicesModel();

            //! Create a new device model.
            static std::shared_ptr<BMDDevicesModel> create(
                const std::shared_ptr<system::Context>&);

            //! Observe the model data.
            std::shared_ptr<observer::IValue<BMDDevicesModelData> > observeData() const;

            //! Set the device index.
            void setDeviceIndex(int);

            //! Set the display mode index.
            void setDisplayModeIndex(int);

            //! Set the pixel type index.
            void setPixelTypeIndex(int);

            //! Set whether the device is enabled.
            void setDeviceEnabled(bool);

            //! Set the video levels.
            void setVideoLevels(image::VideoLevels);

            //! Set the HDR mode.
            void setHDRMode(device::HDRMode);

            //! Set the HDR data.
            void setHDRData(const image::HDRData&);

        private:
            void _update();

            TLRENDER_PRIVATE();
        };
    }
}
