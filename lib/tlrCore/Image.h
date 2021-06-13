// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <iostream>
#include <memory>
#include <vector>

namespace tlr
{
    namespace imaging
    {
        //! Image dimensions.
        class Size
        {
        public:
            Size();
            explicit Size(uint16_t w, uint16_t h);

            uint16_t w, h;

            //! Is this size valid?
            bool isValid() const;

            //! Get the aspect ratio.
            float getAspect() const;

            bool operator == (const Size&) const;
            bool operator != (const Size&) const;
            bool operator < (const Size&) const;
        };

        //! Image pixel types.
        enum class PixelType
        {
            None,
            L_U8,
            L_U16,
            L_F32,
            LA_U8,
            LA_U16,
            LA_F32,
            RGB_U8,
            RGB_U16,
            RGB_F32,
            RGBA_U8,
            RGBA_U16,
            RGBA_F16,
            RGBA_F32,
            YUV_420P,
            Count
        };
        TLR_ENUM(PixelType);

        //! Get the number of channels for the given pixel type.
        uint8_t getChannelCount(PixelType);

        //! Get the bit-depth for the given pixel type.
        uint8_t getBitDepth(PixelType);

        //! Determine the integer pixel type for a given channel count and bit depth.
        PixelType getIntType(std::size_t channelCount, std::size_t bitDepth);

        //! Determine the floating point pixel type for a given channel count and bit depth.
        PixelType getFloatType(std::size_t channelCount, std::size_t bitDepth);

        //! Image information.
        class Info
        {
        public:
            Info();
            explicit Info(const Size&, PixelType);
            explicit Info(uint16_t w, uint16_t h, PixelType);

            Size size;
            PixelType pixelType;

            //! Is the information valid?
            bool isValid() const;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Get the number of bytes used to store the image data.
        std::size_t getDataByteCount(const Info&);

        //! Image.
        class Image : public std::enable_shared_from_this<Image>
        {
            TLR_NON_COPYABLE(Image);

        protected:
            void _init(const Info&);
            Image();

        public:
            ~Image();

            //! Create a new image.
            static std::shared_ptr<Image> create(const Info&);

            //! Get the image information.
            const Info& getInfo() const;

            //! Get the image size.
            const Size& getSize() const;

            //! Get the image width.
            uint16_t getWidth() const;

            //! Get the image height.
            uint16_t getHeight() const;

            //! Get the aspect ratio.
            float getAspect() const;

            //! Get the image pixel type.
            PixelType getPixelType() const;

            //! Is the image valid?
            bool isValid() const;

            //! Get the image data.
            const uint8_t* getData() const;

            //! Get the image data.
            uint8_t* getData();

            //! Zero the image data.
            void zero();

        private:
            Info _info;
            std::vector<uint8_t> _data;
        };
    }

    TLR_ENUM_SERIALIZE(imaging::PixelType);

    std::ostream& operator << (std::ostream&, const imaging::Size&);
    std::ostream& operator << (std::ostream&, const imaging::Info&);
    
    std::istream& operator >> (std::istream&, imaging::Size&);
}

#include <tlrCore/ImageInline.h>
