// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDData.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <array>
#include <cstring>

namespace tl
{
    namespace bmd
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
            "8BitYUV",
            "10BitRGB",
            "10BitRGBX",
            "10BitRGBXLE",
            //"10BitYUV",
            "12BitRGB",
            "12BitRGBLE");
        TLRENDER_ENUM_SERIALIZE_IMPL(PixelType);

        size_t getRowByteCount(int size, PixelType pixelType)
        {
            size_t out = 0;
            switch (pixelType)
            {
            case PixelType::_8BitBGRA:
                out = size * 32 / 8;
                break;
            case PixelType::_8BitYUV:
                out = size * 16 / 8;
                break;
            case PixelType::_10BitRGB:
            case PixelType::_10BitRGBX:
            case PixelType::_10BitRGBXLE:
                out = ((size + 63) / 64) * 256;
                break;
            //case PixelType::_10BitYUV:
            //    out = ((size + 47) / 48) * 128;
            //    break;
            case PixelType::_12BitRGB:
            case PixelType::_12BitRGBLE:
                out = (size * 36) / 8;
                break;
            default: break;
            }
            return out;
        }

        size_t getDataByteCount(const math::Size2i& size, PixelType pixelType)
        {
            return getRowByteCount(size.w, pixelType) * size.h;
        }

        bool DeviceInfo::operator == (const DeviceInfo& other) const
        {
            return
                name == other.name &&
                displayModes == other.displayModes &&
                pixelTypes == other.pixelTypes &&
                hdrMetaData == other.hdrMetaData;
        }

        bool DeviceInfo::operator != (const DeviceInfo& other) const
        {
            return !(*this == other);
        }
        
        TLRENDER_ENUM_IMPL(
            Option,
            "None",
            "444SDIVideoOutput");
        TLRENDER_ENUM_SERIALIZE_IMPL(Option);

        bool DeviceConfig::operator == (const DeviceConfig& other) const
        {
            return
                deviceIndex == other.deviceIndex &&
                displayModeIndex == other.displayModeIndex &&
                pixelType == other.pixelType &&
                boolOptions == other.boolOptions;
        }

        bool DeviceConfig::operator != (const DeviceConfig& other) const
        {
            return !(*this == other);
        }

        TLRENDER_ENUM_IMPL(
            HDRMode,
            "None",
            "FromFile",
            "Custom");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRMode);

        std::shared_ptr<image::HDRData> getHDRData(const timeline::VideoData& videoData)
        {
            std::shared_ptr<image::HDRData> out;
            for (const auto& layer : videoData.layers)
            {
                if (layer.image)
                {
                    const auto& tags = layer.image->getTags();
                    const auto k = tags.find("hdr");
                    if (k != tags.end())
                    {
                        out = std::shared_ptr<image::HDRData>(new image::HDRData);
                        try
                        {
                            auto json = nlohmann::json::parse(k->second);
                            from_json(json, *out);
                        }
                        catch (const std::exception&)
                        {}
                        break;
                    }
                }
            }
            return out;
        }
    }
}
