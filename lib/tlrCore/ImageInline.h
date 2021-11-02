// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tuple>

namespace tlr
{
    namespace imaging
    {
        constexpr Size::Size() noexcept :
            w(0), h(0)
        {}

        constexpr Size::Size(uint16_t w, uint16_t h) noexcept :
            w(w), h(h)
        {}

        constexpr bool Size::isValid() const noexcept
        {
            return w > 0 && h > 0;
        }

        constexpr float Size::getAspect() const noexcept
        {
            return h > 0 ? (w / static_cast<float>(h)) : 0;
        }

        constexpr bool Size::operator == (const Size& other) const noexcept
        {
            return w == other.w && h == other.h;
        }

        constexpr bool Size::operator != (const Size& other) const noexcept
        {
            return !(*this == other);
        }

        constexpr bool Size::operator < (const Size& other) const noexcept
        {
            return std::tie(w, h) < std::tie(other.w, other.h);
        }

        constexpr bool U10_MSB::operator == (const U10_MSB& value) const noexcept
        {
            return
                value.r == r &&
                value.g == g &&
                value.b == b;
        }

        constexpr bool U10_MSB::operator != (const U10_MSB& value) const noexcept
        {
            return !(*this == value);
        }

        constexpr bool U10_LSB::operator == (const U10_LSB& value) const noexcept
        {
            return
                value.r == r &&
                value.g == g &&
                value.b == b;
        }

        constexpr bool U10_LSB::operator != (const U10_LSB& value) const noexcept
        {
            return !(*this == value);
        }

        constexpr Mirror::Mirror() noexcept
        {}

        constexpr Mirror::Mirror(bool x, bool y) noexcept :
            x(x),
            y(y)
        {}

        constexpr bool Mirror::operator == (const Mirror& other) const noexcept
        {
            return other.x == x && other.y == y;
        }

        constexpr bool Mirror::operator != (const Mirror& other) const noexcept
        {
            return !(other == *this);
        }

        inline Layout::Layout() noexcept
        {}

        inline Layout::Layout(const Mirror& mirror, uint8_t alignment, memory::Endian endian) noexcept :
            mirror(mirror),
            alignment(alignment),
            endian(endian)
        {}

        inline bool Layout::operator == (const Layout& other) const noexcept
        {
            return other.mirror == mirror && other.alignment == alignment && other.endian == endian;
        }

        inline bool Layout::operator != (const Layout& other) const noexcept
        {
            return !(other == *this);
        }

        inline Info::Info()
        {}

        inline Info::Info(const Size& size, PixelType pixelType) :
            size(size),
            pixelType(pixelType)
        {}

        inline Info::Info(uint16_t w, uint16_t h, PixelType pixelType) :
            size(w, h),
            pixelType(pixelType)
        {}

        inline bool Info::isValid() const
        {
            return size.isValid() && pixelType != PixelType::None;
        }

        inline bool Info::operator == (const Info& other) const
        {
            return name == other.name &&
                size == other.size &&
                pixelAspectRatio == other.pixelAspectRatio &&
                pixelType == other.pixelType &&
                yuvRange == other.yuvRange &&
                layout == other.layout;
        }

        inline bool Info::operator != (const Info& other) const
        {
            return !(*this == other);
        }

        inline const Info& Image::getInfo() const
        {
            return _info;
        }

        inline const Size& Image::getSize() const
        {
            return _info.size;
        }

        inline uint16_t Image::getWidth() const
        {
            return _info.size.w;
        }

        inline uint16_t Image::getHeight() const
        {
            return _info.size.h;
        }

        inline float Image::getAspect() const
        {
            return _info.size.getAspect();
        }

        inline PixelType Image::getPixelType() const
        {
            return _info.pixelType;
        }

        inline const std::map<std::string, std::string>& Image::getTags() const
        {
            return _tags;
        }

        inline bool Image::isValid() const
        {
            return _info.isValid();
        }

        inline size_t Image::getDataByteCount() const
        {
            return _dataByteCount;
        }

        inline const uint8_t* Image::getData() const
        {
            return _data.data();
        }

        inline uint8_t* Image::getData()
        {
            return _data.data();
        }
    }
}
