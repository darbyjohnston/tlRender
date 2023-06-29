// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/DeviceData.h>

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
        //! Devices model data.
        struct DevicesModelData
        {
            std::vector<std::string> devices;
            int deviceIndex = 0;
            std::vector<std::string> displayModes;
            int displayModeIndex = 0;
            std::vector<device::PixelType> pixelTypes;
            int pixelTypeIndex = 0;
            bool deviceEnabled = true;
            imaging::VideoLevels videoLevels = imaging::VideoLevels::LegalRange;
            device::HDRMode hdrMode = device::HDRMode::FromFile;
            imaging::HDRData hdrData;

            bool operator == (const DevicesModelData&) const;
        };

        //! Devices model.
        class DevicesModel : public std::enable_shared_from_this<DevicesModel>
        {
            TLRENDER_NON_COPYABLE(DevicesModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            DevicesModel();

        public:
            ~DevicesModel();

            //! Create a new device model.
            static std::shared_ptr<DevicesModel> create(const std::shared_ptr<system::Context>&);

            //! Observe the model data.
            std::shared_ptr<observer::IValue<DevicesModelData> > observeData() const;

            //! Set the device index.
            void setDeviceIndex(int);

            //! Set the display mode index.
            void setDisplayModeIndex(int);

            //! Set the pixel type index.
            void setPixelTypeIndex(int);

            //! Set whether the device is enabled.
            void setDeviceEnabled(bool);

            //! Set the video levels.
            void setVideoLevels(imaging::VideoLevels);

            //! Set the HDR mode.
            void setHDRMode(device::HDRMode);

            //! Set the HDR data.
            void setHDRData(const imaging::HDRData&);

        private:
            void _update();

            TLRENDER_PRIVATE();
        };
    }
}
