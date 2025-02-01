// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/HDR.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

namespace tl
{
    namespace image
    {
        TLRENDER_ENUM_IMPL(
            HDR_EOTF,
            "SDR",
            "HDR",
            "ST2084");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDR_EOTF);

        TLRENDER_ENUM_IMPL(
            HDRPrimaries,
            "Red",
            "Green",
            "Blue",
            "White");
        TLRENDER_ENUM_SERIALIZE_IMPL(HDRPrimaries);

        void to_json(nlohmann::json& json, const HDRData& value)
        {
            json = nlohmann::json
            {
                { "eotf", value.eotf },
                { "primaries", value.primaries },
                { "displayMasteringLuminance", value.displayMasteringLuminance },
                { "maxCLL", value.maxCLL },
                { "maxFALL", value.maxFALL }
            };
        }

        void from_json(const nlohmann::json& json, HDRData& value)
        {
            json.at("eotf").get_to(value.eotf);
            json.at("primaries").get_to(value.primaries);
            json.at("displayMasteringLuminance").get_to(value.displayMasteringLuminance);
            json.at("maxCLL").get_to(value.maxCLL);
            json.at("maxFALL").get_to(value.maxFALL);
        }
    }
}
