// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDDeviceData.h>

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

        size_t getRowByteCount(int size, PixelType pixelType)
        {
            size_t out = 0;
            switch (pixelType)
            {
            case PixelType::_8BitBGRA:
                out = size * 32 / 8;
                break;
            case PixelType::_10BitRGBXLE:
                out = ((size + 63) / 64) * 256;
                break;
            default: break;
            }
            return out;
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
