// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/BackgroundOptions.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <sstream>

namespace tl
{
    namespace timeline
    {
        DTK_ENUM_IMPL(
            Background,
            "Solid",
            "Checkers",
            "Gradient");

        void to_json(nlohmann::json& json, const BackgroundOptions& in)
        {
            json["type"] = to_string(in.type);
            json["solidColor"] = in.solidColor;
            json["checkersColor"].push_back(in.checkersColor.first);
            json["checkersColor"].push_back(in.checkersColor.second);
            json["checkersSize"] = in.checkersSize;
            json["gradientColor"].push_back(in.gradientColor.first);
            json["gradientColor"].push_back(in.gradientColor.second);
        }

        void from_json(const nlohmann::json& json, BackgroundOptions& out)
        {
            from_string(json.at("type").get<std::string>(), out.type);
            json.at("solidColor").get_to(out.solidColor);
            json.at("checkersColor")[0].get_to(out.checkersColor.first);
            json.at("checkersColor")[1].get_to(out.checkersColor.second);
            json.at("checkersSize").get_to(out.checkersSize);
            json.at("gradientColor")[0].get_to(out.gradientColor.first);
            json.at("gradientColor")[1].get_to(out.gradientColor.second);
        }
    }
}
