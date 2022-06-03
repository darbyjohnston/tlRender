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
                { "redPrimaries", { value.redPrimaries.first, value.redPrimaries.second } },
                { "greenPrimaries", { value.greenPrimaries.first, value.greenPrimaries.second } },
                { "bluePrimaries", { value.bluePrimaries.first, value.bluePrimaries.second } },
                { "whitePrimaries", { value.whitePrimaries.first, value.whitePrimaries.second } },
                { "displayMasteringLuminance", value.displayMasteringLuminance },
                { "maxCLL", value.maxCLL },
                { "maxFALL", value.maxFALL }
            };
        }

        void from_json(const nlohmann::json& json, HDR& value)
        {
            json.at("eotf").get_to(value.eotf);
            json.at("redPrimaries").at(0).get_to(value.redPrimaries.first);
            json.at("redPrimaries").at(1).get_to(value.redPrimaries.second);
            json.at("greenPrimaries").at(0).get_to(value.greenPrimaries.first);
            json.at("greenPrimaries").at(1).get_to(value.greenPrimaries.second);
            json.at("bluePrimaries").at(0).get_to(value.bluePrimaries.first);
            json.at("bluePrimaries").at(1).get_to(value.bluePrimaries.second);
            json.at("whitePrimaries").at(0).get_to(value.whitePrimaries.first);
            json.at("whitePrimaries").at(1).get_to(value.whitePrimaries.second);
            json.at("displayMasteringLuminance").get_to(value.displayMasteringLuminance);
            json.at("maxCLL").get_to(value.maxCLL);
            json.at("maxFALL").get_to(value.maxFALL);
        }
    }
}
