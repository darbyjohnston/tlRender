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

namespace tlr
{
    namespace imaging
    {
        TLR_ENUM_LABEL_IMPL(
            PixelType,
            "None",
            "L_U8",
            "RGB_U8",
            "RGBA_U8",
            "RGBA_F16",
            "YUV_420P");

        PixelType getIntType(std::size_t channelCount, std::size_t bitDepth)
        {
            PixelType out = PixelType::None;
            switch (channelCount)
            {
            case 1:
                switch (bitDepth)
                {
                case 8: out = PixelType::L_U8; break;
                }
                break;
            case 3:
                switch (bitDepth)
                {
                case 8: out = PixelType::RGB_U8; break;
                }
                break;
            case 4:
                switch (bitDepth)
                {
                case 8: out = PixelType::RGBA_U8; break;
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
            case 4:
                switch (bitDepth)
                {
                case 16: out = PixelType::RGBA_F16; break;
                }
                break;
            }
            return out;
        }

        inline std::size_t getDataByteCount(const Info& info)
        {
            std::size_t out = 0;
            switch (info.pixelType)
            {
            case PixelType::L_U8:
                out = info.size.w * info.size.h;
                break;
            case PixelType::RGB_U8:
                out = info.size.w * info.size.h * 3;
                break;
            case PixelType::RGBA_U8:
                out = info.size.w * info.size.h * 4;
                break;
            case PixelType::RGBA_F16:
                out = info.size.w * info.size.h * 4 * 2;
                break;
            case PixelType::YUV_420P:
                out = info.size.w * info.size.h + (info.size.w / 2 * info.size.h / 2) * 2;
                break;
            default:
                break;
            }
            return out;
        }

        void Image::_init(const Info& info)
        {
            _info = info;
            const std::size_t byteCount = getDataByteCount(info);
            _data.resize(byteCount);
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

        void Image::zero()
        {
            if (!_data.empty())
            {
                memset(&_data[0], 0, getDataByteCount(_info));
            }
        }
    }

    TLR_ENUM_SERIALIZE_IMPL(imaging, PixelType);

    std::ostream& operator << (std::ostream& os, const imaging::Size& value)
    {
        os << value.w << "x" << value.h;
        return os;
    }

    std::ostream& operator << (std::ostream& os, const imaging::Info& value)
    {
        os << value.size << "," << value.pixelType;
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
}
