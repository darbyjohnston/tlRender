// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDData.h>

#include <feather-tk/core/Image.h>
#include <feather-tk/core/ObservableValue.h>

#include <memory>
#include <string>

namespace feather_tk
{
    class Context;
}

namespace tl
{
    namespace bmd
    {
        //! BMD devices model data.
        struct DevicesModelData
        {
            std::vector<std::string> devices;
            int                      deviceIndex = 0;
            std::vector<std::string> displayModes;
            int                      displayModeIndex = 0;
            std::vector<PixelType>   pixelTypes;
            int                      pixelTypeIndex = 0;
            bool                     deviceEnabled = true;
            BoolOptions              boolOptions;
            feather_tk::VideoLevels         videoLevels = feather_tk::VideoLevels::LegalRange;
            HDRMode                  hdrMode = HDRMode::FromFile;
            image::HDRData           hdrData;

            bool operator == (const DevicesModelData&) const;
        };

        //! BMD devices model.
        class DevicesModel : public std::enable_shared_from_this<DevicesModel>
        {
            FEATHER_TK_NON_COPYABLE(DevicesModel);

        protected:
            void _init(const std::shared_ptr<feather_tk::Context>&);

            DevicesModel();

        public:
            ~DevicesModel();

            //! Create a new device model.
            static std::shared_ptr<DevicesModel> create(
                const std::shared_ptr<feather_tk::Context>&);

            //! Observe the model data.
            std::shared_ptr<feather_tk::IObservableValue<DevicesModelData> > observeData() const;

            //! Set the device index.
            void setDeviceIndex(int);

            //! Set the display mode index.
            void setDisplayModeIndex(int);

            //! Set the pixel type index.
            void setPixelTypeIndex(int);

            //! Set whether the device is enabled.
            void setDeviceEnabled(bool);

            //! Set the boolean options.
            void setBoolOptions(const BoolOptions&);

            //! Set the video levels.
            void setVideoLevels(feather_tk::VideoLevels);

            //! Set the HDR mode.
            void setHDRMode(HDRMode);

            //! Set the HDR data.
            void setHDRData(const image::HDRData&);

        private:
            void _update();

            FEATHER_TK_PRIVATE();
        };

        void to_json(nlohmann::json&, const DevicesModelData&);

        void from_json(const nlohmann::json&, DevicesModelData&);
    }
}
