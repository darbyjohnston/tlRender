// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlDevice/DeviceData.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <array>
#include <cstring>

namespace tl
{
    namespace device
    {
        bool DisplayMode::operator == (const DisplayMode& other) const
        {
            return
                name == other.name &&
                size == other.size &&
                frameRate == other.frameRate;
        }

        TLRENDER_ENUM_IMPL(
            PixelType,
            "None",
            "8BitBGRA",
            "10BitRGBXLE");
        TLRENDER_ENUM_SERIALIZE_IMPL(PixelType);

        size_t getDataByteCount(const imaging::Size& size, PixelType pixelType)
        {
            size_t out = 0;
            switch (pixelType)
            {
            case PixelType::_8BitBGRA:
                out = (size.w * 32 / 8) * size.h;
                break;
            case PixelType::_10BitRGBXLE:
                out = ((size.w + 63) / 64) * 256 * size.h;
                break;
            default: break;
            }
            return out;
        }

        void PixelData::_init(
            const imaging::Size& size,
            PixelType pixelType,
            const otime::RationalTime& time)
        {
            _size = size;
            _pixelType = pixelType;
            _time = time;
            _dataByteCount = device::getDataByteCount(_size, _pixelType);
            _data = new uint8_t[_dataByteCount];
        }

        PixelData::PixelData()
        {}

        PixelData::~PixelData()
        {
            delete[] _data;
        }

        std::shared_ptr<PixelData> PixelData::create(
            const imaging::Size& size,
            PixelType pixelType,
            const otime::RationalTime& time)
        {
            auto out = std::shared_ptr<PixelData>(new PixelData);
            out->_init(size, pixelType, time);
            return out;
        }

        const imaging::Size& PixelData::getSize() const
        {
            return _size;
        }

        PixelType PixelData::getPixelType() const
        {
            return _pixelType;
        }

        const otime::RationalTime& PixelData::getTime() const
        {
            return _time;
        }

        bool PixelData::isValid() const
        {
            return _size.isValid() && _pixelType != PixelType::None;
        }

        size_t PixelData::getDataByteCount() const
        {
            return _dataByteCount;
        }

        const uint8_t* PixelData::getData() const
        {
            return _data;
        }

        uint8_t* PixelData::getData()
        {
            return _data;
        }

        void PixelData::zero()
        {
            std::memset(_data, 0, _dataByteCount);
        }

        const std::shared_ptr<imaging::HDRData>& PixelData::getHDRData() const
        {
            return _hdrData;
        }

        void PixelData::setHDRData(const std::shared_ptr<imaging::HDRData>& value)
        {
            _hdrData = value;
        }

        bool DeviceInfo::operator == (const DeviceInfo& other) const
        {
            return
                name == other.name &&
                displayModes == other.displayModes &&
                pixelTypes == other.pixelTypes &&
                hdrMetaData == other.hdrMetaData;
        }

        TLRENDER_ENUM_IMPL(
            HDRMode,
            "None",
            "FromFile",
            "Custom");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRMode);

        std::shared_ptr<imaging::HDRData> getHDRData(const timeline::VideoData& videoData)
        {
            std::shared_ptr<imaging::HDRData> out;
            for (const auto& layer : videoData.layers)
            {
                if (layer.image)
                {
                    const auto& tags = layer.image->getTags();
                    const auto k = tags.find("hdr");
                    if (k != tags.end())
                    {
                        out = std::shared_ptr<imaging::HDRData>(new imaging::HDRData);
                        auto json = nlohmann::json::parse(k->second);
                        from_json(json, *out);
                        break;
                    }
                }
            }
            return out;
        }
    }
}
