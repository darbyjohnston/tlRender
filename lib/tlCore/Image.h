// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/BBox.h>
#include <tlCore/Memory.h>
#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <half.h>

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace tl
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
            constexpr explicit Size(
                uint16_t w,
                uint16_t h,
                float pixelAspectRatio = 1.F) noexcept;

            uint16_t w = 0;
            uint16_t h = 0;
            float pixelAspectRatio = 1.F;

            //! Is this size valid?
            constexpr bool isValid() const noexcept;

            //! Get the aspect ratio.
            constexpr float getAspect() const noexcept;

            constexpr bool operator == (const Size&) const noexcept;
            constexpr bool operator != (const Size&) const noexcept;
            bool operator < (const Size&) const noexcept;
        };

        //! Get a bounding box with the given aspect ratio that fits within
        //! the given bounding box.
        math::BBox2i getBBox(float aspect, const math::BBox2i&) noexcept;

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

            YUV_420P_U8,
            YUV_422P_U8,
            YUV_444P_U8,

            YUV_420P_U16,
            YUV_422P_U16,
            YUV_444P_U16,

            Count,
            First = None
        };
        TLRENDER_ENUM(PixelType);
        TLRENDER_ENUM_SERIALIZE(PixelType);

        typedef uint8_t   U8_T;
        typedef uint16_t U10_T;
        typedef uint16_t U12_T;
        typedef uint16_t U16_T;
        typedef uint32_t U32_T;
        typedef half     F16_T;
        typedef float    F32_T;

        const math::Range<U8_T>  U8Range(
            std::numeric_limits<U8_T>::min(),
            std::numeric_limits<U8_T>::max());
        const math::Range<U10_T> U10Range(0, 1023);
        const math::Range<U12_T> U12Range(0, 4095);
        const math::Range<U16_T> U16Range(
            std::numeric_limits<U16_T>::min(),
            std::numeric_limits<U16_T>::max());
        const math::Range<U32_T> U32Range(
            std::numeric_limits<U32_T>::min(),
            std::numeric_limits<U32_T>::max());
        const math::Range<F16_T> F16Range(0.F, 1.F);
        const math::Range<F32_T> F32Range(0.F, 1.F);

        //! Video levels.
        enum class VideoLevels
        {
            FullRange,
            LegalRange,

            Count,
            First = FullRange
        };
        TLRENDER_ENUM(VideoLevels);
        TLRENDER_ENUM_SERIALIZE(VideoLevels);

        //! YUV coefficients.
        enum class YUVCoefficients
        {
            REC709,
            BT2020,

            Count,
            First = REC709
        };
        TLRENDER_ENUM(YUVCoefficients);
        TLRENDER_ENUM_SERIALIZE(YUVCoefficients);

        //! Get YUV coefficients.
        math::Vector4f getYUVCoefficients(YUVCoefficients) noexcept;

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
#if defined(TLRENDER_ENDIAN_MSB)
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
            Layout() noexcept;
            Layout(
                const Mirror&  mirror,
                uint8_t        alignment = 1,
                memory::Endian endian    = memory::getEndian()) noexcept;

            Mirror         mirror;
            uint8_t        alignment = 1;
            memory::Endian endian    = memory::getEndian();

            bool operator == (const Layout&) const noexcept;
            bool operator != (const Layout&) const noexcept;
        };

        //! Image information.
        class Info
        {
        public:
            Info();
            explicit Info(const Size&, PixelType);
            explicit Info(uint16_t w, uint16_t h, PixelType);

            std::string     name             = "Default";
            Size            size;
            PixelType       pixelType        = PixelType::None;
            VideoLevels     videoLevels      = VideoLevels::FullRange;
            YUVCoefficients yuvCoefficients  = YUVCoefficients::REC709;
            Layout          layout;

            //! Is the information valid?
            bool isValid() const;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Get the number of bytes required to align data.
        size_t getAlignedByteCount(size_t value, size_t alignment);

        //! Get the number of bytes used to store image data.
        std::size_t getDataByteCount(const Info&);

        //! Image tags.
        typedef std::map<std::string, std::string> Tags;

        //! Image.
        class Image : public std::enable_shared_from_this<Image>
        {
            TLRENDER_NON_COPYABLE(Image);

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

            //! Get the image tags.
            const Tags& getTags() const;

            //! Set the image tags.
            void setTags(const Tags&);

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
            Tags _tags;
            size_t _dataByteCount = 0;
            uint8_t* _data = nullptr;
        };

        void to_json(nlohmann::json&, const Size&);

        void from_json(const nlohmann::json&, Size&);

        std::ostream& operator << (std::ostream&, const Size&);

        std::istream& operator >> (std::istream&, Size&);
    }
}

#include <tlCore/ImageInline.h>
