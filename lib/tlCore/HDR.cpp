// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/HDR.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/String.h>

#include <sstream>

namespace tl
{
    namespace image
    {
        FEATHER_TK_ENUM_IMPL(
            HDR_EOTF,
            "SDR",
            "HDR",
            "ST2084");

        FEATHER_TK_ENUM_IMPL(
            HDRPrimaries,
            "Red",
            "Green",
            "Blue",
            "White");

        void to_json(nlohmann::json& json, const HDRData& value)
        {
            json["EOTF"] = to_string(value.eotf);
            for (size_t i = 0; i < value.primaries.size(); ++i)
            {
                json["Primaries"].push_back(value.primaries[i]);
            }
            json["DisplayMasteringLuminance"] = value.displayMasteringLuminance;
            json["MaxCLL"] = value.maxCLL;
            json["MaxFALL"] = value.maxFALL;
        }

        void from_json(const nlohmann::json& json, HDRData& value)
        {
            from_string(json.at("EOTF").get<std::string>(), value.eotf);
            for (size_t i = 0; i < value.primaries.size(); ++i)
            {
                json.at("Primaries").at(i).get_to(value.primaries[i]);
            }
            json.at("DisplayMasteringLuminance").get_to(value.displayMasteringLuminance);
            json.at("MaxCLL").get_to(value.maxCLL);
            json.at("MaxFALL").get_to(value.maxFALL);
        }
    }
}
