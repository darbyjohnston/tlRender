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
        //! Image Dimensions
        struct Size
        {
            Size();
            explicit Size(uint16_t w, uint16_t h);

            uint16_t w = 0;
            uint16_t h = 0;

            //! Is this size valid?
            bool isValid() const;

            //! Get the aspect ratio.
            float getAspect() const;

            bool operator == (const Size&) const;
            bool operator != (const Size&) const;
            bool operator < (const Size&) const;
        };

        //! Pixel Type
        enum class PixelType
        {
            None,
            L_U8,
            RGB_U8,
            RGBA_U8,
            Count
        };
        TLR_ENUM_LABEL(PixelType);

        //! Get the number of bytes used to store a pixel type.
        std::size_t getByteCount(PixelType);

        //! Get an integer pixel type for a given channel count and bit depth.
        PixelType getIntType(size_t channelCount, size_t bitDepth);

        //! Image Information
        struct Info
        {
            Info();
            explicit Info(const Size&, PixelType);
            explicit Info(uint16_t w, uint16_t h, PixelType);

            Size size;
            PixelType pixelType = PixelType::None;

            //! Is the information valid?
            bool isValid() const;

            bool operator == (const Info&) const;
        };

        //! Get the number of bytes used to store a scanline.
        std::size_t getScanlineByteCount(const Info&);

        //! Get the number of bytes used to store the image data.
        std::size_t getDataByteCount(const Info&);

        //! Image
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
            const std::vector<uint8_t>& getData() const;

            //! Get the image data.
            uint8_t* getData(uint16_t);

            //! Get the image data.
            const uint8_t* getData(uint16_t) const;

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

#include <tlrAV/ImageInline.h>
