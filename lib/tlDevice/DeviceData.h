// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Video.h>

#include <tlCore/HDR.h>
#include <tlCore/Image.h>
#include <tlCore/Size.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace device
    {
        //! Display mode.
        struct DisplayMode
        {
            std::string name;
            math::Size2i size;
            otime::RationalTime frameRate;

            bool operator == (const DisplayMode&) const;
        };

        //! Pixel types.
        enum class PixelType
        {
            None,
            _8BitBGRA,
            _10BitRGBXLE,

            Count,
            First = None
        };
        TLRENDER_ENUM(PixelType);
        TLRENDER_ENUM_SERIALIZE(PixelType);

        //! Get the number of bytes used to store the pixel data.
        size_t getDataByteCount(const math::Size2i&, PixelType);

        //! Pixel data.
        class PixelData : public std::enable_shared_from_this<PixelData>
        {
            TLRENDER_NON_COPYABLE(PixelData);

        protected:
            void _init(
                const math::Size2i&,
                PixelType,
                const otime::RationalTime&);
            PixelData();

        public:
            ~PixelData();

            //! Create new pixel data.
            static std::shared_ptr<PixelData> create(
                const math::Size2i&,
                PixelType,
                const otime::RationalTime&);

            //! Get the pixel data size.
            const math::Size2i& getSize() const;

            //! Get the pixel type.
            PixelType getPixelType() const;

            //! Get the time.
            const otime::RationalTime& getTime() const;

            //! Is the pixel data valid?
            bool isValid() const;

            //! Get the number of bytes used to store the pixel data.
            size_t getDataByteCount() const;

            //! Get the pixel data.
            const uint8_t* getData() const;

            //! Get the pixel data.
            uint8_t* getData();

            //! Zero the pixel data.
            void zero();

            //! Get HDR data.
            const std::shared_ptr<image::HDRData>& getHDRData() const;

            //! Set HDR data.
            void setHDRData(const std::shared_ptr<image::HDRData>&);

        private:
            math::Size2i _size;
            PixelType _pixelType = PixelType::None;
            otime::RationalTime _time;
            size_t _dataByteCount = 0;
            uint8_t* _data = nullptr;
            std::shared_ptr<image::HDRData> _hdrData;
        };

        //! Device information.
        struct DeviceInfo
        {
            std::string name;
            std::vector<DisplayMode> displayModes;
            std::vector<PixelType> pixelTypes;
            size_t minVideoPreroll = 0;
            bool hdrMetaData = false;
            size_t maxAudioChannels = 0;

            bool operator == (const DeviceInfo&) const;
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
        TLRENDER_ENUM(HDRMode);
        TLRENDER_ENUM_SERIALIZE(HDRMode);

        //! Get HDR data from timeline video data.
        std::shared_ptr<image::HDRData> getHDRData(const timeline::VideoData&);
    }
}
