// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/HDR.h>

namespace tl
{
    namespace image
    {
        void to_json(nlohmann::json& json, const HDRData& value)
        {
            json = nlohmann::json
            {
                { "eotf", value.eotf },
                { "redPrimaries", value.redPrimaries },
                { "greenPrimaries", value.greenPrimaries },
                { "bluePrimaries", value.bluePrimaries },
                { "whitePrimaries", value.whitePrimaries },
                { "displayMasteringLuminance", value.displayMasteringLuminance },
                { "maxCLL", value.maxCLL },
                { "maxFALL", value.maxFALL }
            };
        }

        void from_json(const nlohmann::json& json, HDRData& value)
        {
            json.at("eotf").get_to(value.eotf);
            json.at("redPrimaries").get_to(value.redPrimaries);
            json.at("greenPrimaries").get_to(value.greenPrimaries);
            json.at("bluePrimaries").get_to(value.bluePrimaries);
            json.at("whitePrimaries").get_to(value.whitePrimaries);
            json.at("displayMasteringLuminance").get_to(value.displayMasteringLuminance);
            json.at("maxCLL").get_to(value.maxCLL);
            json.at("maxFALL").get_to(value.maxFALL);
        }
    }
}
