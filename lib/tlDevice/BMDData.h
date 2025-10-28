// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Video.h>

#include <tlCore/HDR.h>
#include <tlCore/Time.h>

#include <ftk/Core/Size.h>

namespace tl
{
    namespace bmd
    {
        //! Display mode.
        struct DisplayMode
        {
            std::string           name;
            ftk::Size2I           size;
            OTIO_NS::RationalTime frameRate;

            bool operator == (const DisplayMode&) const;
        };

        //! Pixel types.
        //!
        //! \bug Disable 10-bit YUV since the BMD conversion function
        //! shows artifacts.
        enum class PixelType
        {
            None,
            _8BitBGRA,
            _8BitYUV,
            _10BitRGB,
            _10BitRGBX,
            _10BitRGBXLE,
            //_10BitYUV,
            _12BitRGB,
            _12BitRGBLE,

            Count,
            First = None
        };
        FTK_ENUM(PixelType);

        //! Get the number of bytes used to store a row of pixel data.
        size_t getRowByteCount(int, PixelType);

        //! Get the number of bytes used to storepixel data.
        size_t getDataByteCount(const ftk::Size2I&, PixelType);

        //! Device information.
        struct DeviceInfo
        {
            std::string              name;
            std::vector<DisplayMode> displayModes;
            std::vector<PixelType>   pixelTypes;
            size_t                   minVideoPreroll  = 0;
            bool                     hdrMetaData      = false;
            size_t                   maxAudioChannels = 0;

            bool operator == (const DeviceInfo&) const;
            bool operator != (const DeviceInfo&) const;
        };

        //! Device options.
        enum class Option
        {
            None,
            _444SDIVideoOutput,

            Count,
            First = None
        };
        FTK_ENUM(Option);

        //! Device boolean options.
        typedef std::map<Option, bool> BoolOptions;

        //! Device configuration.
        struct DeviceConfig
        {
            int         deviceIndex      = -1;
            int         displayModeIndex = -1;
            PixelType   pixelType        = PixelType::None;
            BoolOptions boolOptions;

            bool operator == (const DeviceConfig&) const;
            bool operator != (const DeviceConfig&) const;
        };

        //! HDR mode.
        enum class HDRMode
        {
            None,
            FromFile,
            Custom,

            Count,
            First = None
        };
        FTK_ENUM(HDRMode);

        //! Get HDR data from timeline video data.
        std::shared_ptr<image::HDRData> getHDRData(const timeline::VideoData&);
    }
}

