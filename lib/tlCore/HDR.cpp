// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/HDR.h>

namespace tl
{
    namespace imaging
    {
        void to_json(nlohmann::json& json, const HDR& value)
        {
            json = nlohmann::json
            {
                { "eotf", value.eotf },
                { "redPrimaries", { value.redPrimaries.x, value.redPrimaries.y } },
                { "greenPrimaries", { value.greenPrimaries.x, value.greenPrimaries.y } },
                { "bluePrimaries", { value.bluePrimaries.x, value.bluePrimaries.y } },
                { "whitePrimaries", { value.whitePrimaries.x, value.whitePrimaries.y } },
                { "displayMasteringLuminance", value.displayMasteringLuminance },
                { "maxCLL", value.maxCLL },
                { "maxFALL", value.maxFALL }
            };
        }

        void from_json(const nlohmann::json& json, HDR& value)
        {
            json.at("eotf").get_to(value.eotf);
            json.at("redPrimaries").at(0).get_to(value.redPrimaries.x);
            json.at("redPrimaries").at(1).get_to(value.redPrimaries.y);
            json.at("greenPrimaries").at(0).get_to(value.greenPrimaries.x);
            json.at("greenPrimaries").at(1).get_to(value.greenPrimaries.y);
            json.at("bluePrimaries").at(0).get_to(value.bluePrimaries.x);
            json.at("bluePrimaries").at(1).get_to(value.bluePrimaries.y);
            json.at("whitePrimaries").at(0).get_to(value.whitePrimaries.x);
            json.at("whitePrimaries").at(1).get_to(value.whitePrimaries.y);
            json.at("displayMasteringLuminance").get_to(value.displayMasteringLuminance);
            json.at("maxCLL").get_to(value.maxCLL);
            json.at("maxFALL").get_to(value.maxFALL);
        }
    }
}
