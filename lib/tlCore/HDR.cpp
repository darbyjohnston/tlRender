// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/HDR.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <sstream>

namespace tl
{
    namespace image
    {
        DTK_ENUM_IMPL(
            HDR_EOTF,
            "SDR",
            "HDR",
            "ST2084");

        DTK_ENUM_IMPL(
            HDRPrimaries,
            "Red",
            "Green",
            "Blue",
            "White");

        void to_json(nlohmann::json& json, const HDRData& value)
        {
            json["eotf"] = to_string(value.eotf);
            for (size_t i = 0; i < value.primaries.size(); ++i)
            {
                json["primaries"].push_back(value.primaries[i]);
            }
            json["displayMasteringLuminance"] = value.displayMasteringLuminance;
            json["maxCLL"] = value.maxCLL;
            json["maxFALL"] = value.maxFALL;
        }

        void from_json(const nlohmann::json& json, HDRData& value)
        {
            from_string(json["eotf"].get<std::string>(), value.eotf);
            for (size_t i = 0; i < value.primaries.size(); ++i)
            {
                json["primaries"].at(i).get_to(value.primaries[i]);
            }
            json["displayMasteringLuminance"].get_to(value.displayMasteringLuminance);
            json["maxCLL"].get_to(value.maxCLL);
            json["maxFALL"].get_to(value.maxFALL);
        }
    }
}
