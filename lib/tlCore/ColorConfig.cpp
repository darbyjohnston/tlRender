// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/ColorConfig.h>

namespace tl
{
    namespace imaging
    {
        void to_json(nlohmann::json& json, const ColorConfig& value)
        {
            json = nlohmann::json
            {
                { "fileName", value.fileName },
                { "input", value.input },
                { "display", value.display },
                { "view", value.view },
                { "look", value.look }
            };
        }

        void from_json(const nlohmann::json& json, ColorConfig& value)
        {
            json.at("fileName").get_to(value.fileName);
            json.at("input").get_to(value.input);
            json.at("display").get_to(value.display);
            json.at("view").get_to(value.view);
            json.at("look").get_to(value.look);
        }
    }
}
