// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Image.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>

using namespace tlr::core;

namespace tlr
{
    namespace imaging
    {
        math::BBox2f getBBox(float aspect, const imaging::Size& size)
        {
            math::BBox2f out;
            const float sizeAspect = size.getAspect();
            math::BBox2f bbox;
            if (sizeAspect > aspect)
            {
                out = math::BBox2f(
                    size.w / 2.F - (size.h * aspect) / 2.F,
                    0.F,
                    size.h * aspect,
                    size.h);
            }
            else
            {
                out = math::BBox2f(
                    0.F,
                    size.h / 2.F - (size.w / aspect) / 2.F,
                    size.w,
                    size.w / aspect);
            }
            return out;
        }

        std::ostream& operator << (std::ostream& os, const imaging::Size& value)
        {
            os << value.w << "x" << value.h;
            return os;
        }

        std::istream& operator >> (std::istream& is, imaging::Size& out)
        {
            std::string s;
            is >> s;
            auto split = string::split(s, 'x');
            if (split.size() != 2)
            {
                throw ParseError();
            }
            out.w = std::stoi(split[0]);
            out.h = std::stoi(split[1]);
            return is;
        }

        TLR_ENUM_IMPL(
            PixelType,
            "None",

            "L_U8",
            "L_U16",
            "L_U32",
            "L_F16",
            "L_F32",

            "LA_U8",
            "LA_U16",
            "LA_U32",
            "LA_F16",
            "LA_F32",
            
            "RGB_U8",
            "RGB_U10",
            "RGB_U16",
            "RGB_U32",
            "RGB_F16",
            "RGB_F32",

            "RGBA_U8",
            "RGBA_U16",
            "RGBA_U32",
            "RGBA_F16",
            "RGBA_F32",
            
            "YUV_420P");
        TLR_ENUM_SERIALIZE_IMPL(PixelType);

        TLR_ENUM_IMPL(
            YUVRange,
            "Full",
            "Video");
        TLR_ENUM_SERIALIZE_IMPL(YUVRange);

        uint8_t getChannelCount(PixelType value)
        {
            const std::array<uint8_t, static_cast<size_t>(PixelType::Count)> values =
            {
                0,
                1, 1, 1, 1, 1,
                2, 2, 2, 2, 2,
                3, 3, 3, 3, 3, 3,
                4, 4, 4, 4, 4,
                3
            };
            return values[static_cast<size_t>(value)];
        }

        uint8_t getBitDepth(PixelType value)
        {
            const std::array<uint8_t, static_cast<size_t>(PixelType::Count)> values =
            {
                0,
                8, 16, 32, 16, 32,
                8, 16, 32, 16, 32,
                8, 10, 16, 32, 16, 32,
                8, 16, 32, 16, 32,
                0
            };
            return values[static_cast<size_t>(value)];
        }

        PixelType getIntType(std::size_t channelCount, std::size_t bitDepth)
        {
            PixelType out = PixelType::None;
            switch (channelCount)
            {
            case 1:
                switch (bitDepth)
                {
                case 8: out = PixelType::L_U8; break;
                case 16: out = PixelType::L_U16; break;
                case 32: out = PixelType::L_U32; break;
                }
                break;
            case 2:
                switch (bitDepth)
                {
                case 8: out = PixelType::LA_U8; break;
                case 16: out = PixelType::LA_U16; break;
                case 32: out = PixelType::LA_U32; break;
                }
                break;
            case 3:
                switch (bitDepth)
                {
                case 8: out = PixelType::RGB_U8; break;
                case 10: out = PixelType::RGB_U10; break;
                case 16: out = PixelType::RGB_U16; break;
                case 32: out = PixelType::RGB_U32; break;
                }
                break;
            case 4:
                switch (bitDepth)
                {
                case 8: out = PixelType::RGBA_U8; break;
                case 16: out = PixelType::RGBA_U16; break;
                case 32: out = PixelType::RGBA_U32; break;
                }
                break;
            }
            return out;
        }

        PixelType getFloatType(std::size_t channelCount, std::size_t bitDepth)
        {
            PixelType out = PixelType::None;
            switch (channelCount)
            {
            case 1:
                switch (bitDepth)
                {
                case 16: out = PixelType::L_F16; break;
                case 32: out = PixelType::L_F32; break;
                }
                break;
            case 2:
                switch (bitDepth)
                {
                case 16: out = PixelType::LA_F16; break;
                case 32: out = PixelType::LA_F32; break;
                }
                break;
            case 3:
                switch (bitDepth)
                {
                case 16: out = PixelType::RGB_F16; break;
                case 32: out = PixelType::RGB_F32; break;
                }
                break;
            case 4:
                switch (bitDepth)
                {
                case 16: out = PixelType::RGBA_F16; break;
                case 32: out = PixelType::RGBA_F32; break;
                }
                break;
            }
            return out;
        }

        PixelType getClosest(PixelType value, const std::vector<PixelType>& types)
        {
            std::map<size_t, PixelType> diff;
            for (const auto& type : types)
            {
                diff[abs(getChannelCount(value) - getChannelCount(type)) +
                    abs(getBitDepth(value) - getBitDepth(type))] = type;
            }
            return !diff.empty() ? diff.begin()->second : PixelType::None;
        }

        size_t align(size_t value, size_t alignment)
        {
            return (value / alignment * alignment) + (value % alignment != 0 ? alignment : 0);
        }

        std::size_t getDataByteCount(const Info& info)
        {
            std::size_t out = 0;
            const size_t w = info.size.w;
            const size_t h = info.size.h;
            const size_t alignment = info.layout.alignment;
            switch (info.pixelType)
            {
            case PixelType::L_U8:     out = align(w, alignment) * h; break;
            case PixelType::L_U16:    out = align(w * 2, alignment) * h; break;
            case PixelType::L_U32:    out = align(w * 4, alignment) * h; break;
            case PixelType::L_F16:    out = align(w * 2, alignment) * h; break;
            case PixelType::L_F32:    out = align(w * 4, alignment) * h; break;

            case PixelType::LA_U8:    out = align(w * 2, alignment) * h; break;
            case PixelType::LA_U16:   out = align(w * 2 * 2, alignment) * h; break;
            case PixelType::LA_U32:   out = align(w * 2 * 4, alignment) * h; break;
            case PixelType::LA_F16:   out = align(w * 2 * 2, alignment) * h; break;
            case PixelType::LA_F32:   out = align(w * 2 * 4, alignment) * h; break;

            case PixelType::RGB_U8:   out = align(w * 3, alignment) * h; break;
            case PixelType::RGB_U10:  out = align(w * 4, alignment) * h; break;
            case PixelType::RGB_U16:  out = align(w * 3 * 2, alignment) * h; break;
            case PixelType::RGB_U32:  out = align(w * 3 * 4, alignment) * h; break;
            case PixelType::RGB_F16:  out = align(w * 3 * 2, alignment) * h; break;
            case PixelType::RGB_F32:  out = align(w * 3 * 4, alignment) * h; break;

            case PixelType::RGBA_U8:  out = align(w * 4, alignment) * h; break;
            case PixelType::RGBA_U16: out = align(w * 4 * 2, alignment) * h; break;
            case PixelType::RGBA_U32: out = align(w * 4 * 4, alignment) * h; break;
            case PixelType::RGBA_F16: out = align(w * 4 * 2, alignment) * h; break;
            case PixelType::RGBA_F32: out = align(w * 4 * 4, alignment) * h; break;

            case PixelType::YUV_420P:
                //! \todo Is YUV data aligned?
                out = w * h + (w / 2 * h / 2) * 2;
                break;

            default: break;
            }
            return out;
        }

        std::ostream& operator << (std::ostream& os, const imaging::Info& value)
        {
            os << value.size << "," << value.pixelType;
            return os;
        }

        void Image::_init(const Info& info)
        {
            _info = info;
            _dataByteCount = imaging::getDataByteCount(info);
            _data.resize(_dataByteCount);
        }

        Image::Image()
        {}

        Image::~Image()
        {}

        std::shared_ptr<Image> Image::create(const Info& info)
        {
            auto out = std::shared_ptr<Image>(new Image);
            out->_init(info);
            return out;
        }

        void Image::setTags(const std::map<std::string, std::string>& value)
        {
            _tags = value;
        }

        void Image::zero()
        {
            if (!_data.empty())
            {
                std::memset(&_data[0], 0, _dataByteCount);
            }
        }
    }
}
