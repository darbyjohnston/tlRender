// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tuple>

namespace tlr
{
    namespace imaging
    {
        inline Size::Size() :
            w(0), h(0)
        {}

        inline Size::Size(uint16_t w, uint16_t h) :
            w(w), h(h)
        {}

        inline bool Size::isValid() const
        {
            return w > 0 && h > 0;
        }

        inline float Size::getAspect() const
        {
            return h > 0 ? (w / static_cast<float>(h)) : 0;
        }

        inline bool Size::operator == (const Size& other) const
        {
            return w == other.w && h == other.h;
        }

        inline bool Size::operator != (const Size& other) const
        {
            return !(*this == other);
        }

        inline bool Size::operator < (const Size& other) const
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

        inline Info::Info() :
            pixelType(PixelType::None)
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
            return size == other.size &&
                pixelType == other.pixelType;
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

        inline bool Image::isValid() const
        {
            return _info.isValid();
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
