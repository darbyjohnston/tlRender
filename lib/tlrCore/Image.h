// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/BBox.h>
#include <tlrCore/Memory.h>
#include <tlrCore/Range.h>
#include <tlrCore/Util.h>

#include <half.h>

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tlr
{
    namespace imaging
    {
        //! \name Sizes
        ///@{

        //! Image size.
        class Size
        {
        public:
            constexpr Size() noexcept;
            constexpr explicit Size(uint16_t w, uint16_t h) noexcept;

            uint16_t w, h;

            //! Is this size valid?
            constexpr bool isValid() const noexcept;

            //! Get the aspect ratio.
            constexpr float getAspect() const noexcept;

            constexpr bool operator == (const Size&) const noexcept;
            constexpr bool operator != (const Size&) const noexcept;
            constexpr bool operator < (const Size&) const noexcept;
        };

        //! Get a bounding box with the given aspect ratio that fits the
        //! given size.
        math::BBox2f getBBox(float aspect, const imaging::Size&) noexcept;

        std::ostream& operator << (std::ostream&, const imaging::Size&);
    
        std::istream& operator >> (std::istream&, imaging::Size&);
        
        ///@}

        //! \name Pixel Types
        ///@{

        //! Image pixel types.
        enum class PixelType
        {
            None,

            L_U8,
            L_U16,
            L_U32,
            L_F16,
            L_F32,

            LA_U8,
            LA_U16,
            LA_U32,
            LA_F16,
            LA_F32,

            RGB_U8,
            RGB_U10,
            RGB_U16,
            RGB_U32,
            RGB_F16,
            RGB_F32,

            RGBA_U8,
            RGBA_U16,
            RGBA_U32,
            RGBA_F16,
            RGBA_F32,
            
            YUV_420P,

            Count,
            First = None
        };
        TLR_ENUM(PixelType);
        TLR_ENUM_SERIALIZE(PixelType);

        typedef uint8_t   U8_T;
        typedef uint16_t U10_T;
        typedef uint16_t U12_T;
        typedef uint16_t U16_T;
        typedef uint32_t U32_T;
        typedef half     F16_T;
        typedef float    F32_T;

        const math::Range<U8_T>  U8Range (std::numeric_limits<U8_T>::min(), std::numeric_limits<U8_T>::max());
        const math::Range<U10_T> U10Range(0, 1023);
        const math::Range<U12_T> U12Range(0, 4095);
        const math::Range<U16_T> U16Range(std::numeric_limits<U16_T>::min(), std::numeric_limits<U16_T>::max());
        const math::Range<U32_T> U32Range(std::numeric_limits<U32_T>::min(), std::numeric_limits<U32_T>::max());
        const math::Range<F16_T> F16Range(0.F, 1.F);
        const math::Range<F32_T> F32Range(0.F, 1.F);

        //! YUV value range.
        //! 
        //! References:
        //! - https://trac.ffmpeg.org/wiki/colorspace
        //! - https://web.archive.org/web/20180423091842/http://www.equasys.de/colorconversion.html
        enum class YUVRange
        {
            Full,  //!< 0-255
            Video, //!< 16-240 (Y) and 16-235 (Cb/Cr)

            Count,
            First = Full
        };
        TLR_ENUM(YUVRange);
        TLR_ENUM_SERIALIZE(YUVRange);

        //! 10-bit MSB pixel data.
        struct U10_MSB
        {
            uint32_t r : 10;
            uint32_t g : 10;
            uint32_t b : 10;
            uint32_t pad : 2;

            constexpr bool operator == (const U10_MSB&) const noexcept;
            constexpr bool operator != (const U10_MSB&) const noexcept;
        };

        //! 10-bit LSB pixel data.
        struct U10_LSB
        {
            uint32_t pad : 2;
            uint32_t b : 10;
            uint32_t g : 10;
            uint32_t r : 10;

            constexpr bool operator == (const U10_LSB&) const noexcept;
            constexpr bool operator != (const U10_LSB&) const noexcept;
        };
#if defined(TLR_ENDIAN_MSB)
        typedef U10_MSB U10;
#else
        typedef U10_LSB U10;
#endif

        //! Get the number of channels for the given pixel type.
        uint8_t getChannelCount(PixelType) noexcept;

        //! Get the bit-depth for the given pixel type.
        uint8_t getBitDepth(PixelType) noexcept;

        //! Determine the integer pixel type for a given channel count and bit depth.
        PixelType getIntType(std::size_t channelCount, std::size_t bitDepth) noexcept;

        //! Determine the floating point pixel type for a given channel count and bit depth.
        PixelType getFloatType(std::size_t channelCount, std::size_t bitDepth) noexcept;

        //! Get the closest pixel type for the given pixel type.
        PixelType getClosest(PixelType, const std::vector<PixelType>&);

        ///@}

        //! Image mirroring.
        class Mirror
        {
        public:
            constexpr Mirror() noexcept;
            constexpr Mirror(bool x, bool y) noexcept;

            bool x = false;
            bool y = false;

            constexpr bool operator == (const Mirror&) const noexcept;
            constexpr bool operator != (const Mirror&) const noexcept;
        };

        //! Image data layout.
        class Layout
        {
        public:
            constexpr Layout() noexcept;
            constexpr Layout(
                const Mirror&   mirror,
                uint8_t         alignment = 1,
                memory::Endian  endian    = memory::getEndian()) noexcept;

            Mirror         mirror;
            uint8_t        alignment = 1;
            memory::Endian endian    = memory::getEndian();

            constexpr bool operator == (const Layout&) const noexcept;
            constexpr bool operator != (const Layout&) const noexcept;
        };

        //! Align a number of bytes.
        size_t align(size_t value, size_t alignment);

        //! Image information.
        class Info
        {
        public:
            Info();
            explicit Info(const Size&, PixelType);
            explicit Info(uint16_t w, uint16_t h, PixelType);

            std::string name             = "Default";
            Size        size;
            float       pixelAspectRatio = 1.F;
            PixelType   pixelType        = PixelType::None;
            YUVRange    yuvRange         = YUVRange::Full;
            Layout      layout;

            //! Is the information valid?
            bool isValid() const;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Get the number of bytes used to store the image data.
        std::size_t getDataByteCount(const Info&);

        std::ostream& operator << (std::ostream&, const Info&);

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

            //! Get the image tags.
            const std::map<std::string, std::string>& getTags() const;

            //! Set the image tags.
            void setTags(const std::map<std::string, std::string>&);

            //! Is the image valid?
            bool isValid() const;

            //! Get the number of bytes used to store the image data.
            size_t getDataByteCount() const;

            //! Get the image data.
            const uint8_t* getData() const;

            //! Get the image data.
            uint8_t* getData();

            //! Zero the image data.
            void zero();

        private:
            Info _info;
            std::map<std::string, std::string> _tags;
            size_t _dataByteCount = 0;
            std::vector<uint8_t> _data;
        };
    }
}

#include <tlrCore/ImageInline.h>
