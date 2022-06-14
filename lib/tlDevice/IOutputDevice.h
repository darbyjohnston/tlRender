// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/DeviceData.h>

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

            //! Get the output device size.
            const imaging::Size& getSize() const;

            //! Get the output device frame rate.
            const otime::RationalTime& getFrameRate() const;

            //! Get the HDR mode and metadata.
            std::pair<HDRMode, imaging::HDRData> getHDR() const;

            //! Set the HDR mode and metadata.
            void setHDR(HDRMode, const imaging::HDRData&);

            //! Display pixel data.
            virtual void display(const std::shared_ptr<PixelData>&) = 0;

        protected:
            int _deviceIndex = 0;
            int _displayModeIndex = 0;
            imaging::Size _size;
            PixelType _pixelType = PixelType::None;
            otime::RationalTime _frameRate;
            HDRMode _hdrMode = HDRMode::FromFile;
            imaging::HDRData _hdrData;
        };
    }
}
