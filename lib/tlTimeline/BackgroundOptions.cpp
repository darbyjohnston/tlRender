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

        bool Outline::operator == (const Outline& other) const
        {
            return
                enabled == other.enabled &&
                width == other.width &&
                color == other.color;
        }

        bool Outline::operator != (const Outline& other) const
        {
            return !(*this == other);
        }

        bool BackgroundOptions::operator == (const BackgroundOptions& other) const
        {
            return
                type == other.type &&
                solidColor == other.solidColor &&
                checkersColor == other.checkersColor &&
                checkersSize == other.checkersSize &&
                gradientColor == other.gradientColor &&
                outline == other.outline;
        }

        bool BackgroundOptions::operator != (const BackgroundOptions& other) const
        {
            return !(*this == other);
        }

        void to_json(nlohmann::json& json, const Outline& in)
        {
            json["enabled"] = in.enabled;
            json["width"] = in.width;
            json["color"] = in.color;
        }

        void to_json(nlohmann::json& json, const BackgroundOptions& in)
        {
            json["type"] = to_string(in.type);
            json["solidColor"] = in.solidColor;
            json["checkersColor"].push_back(in.checkersColor.first);
            json["checkersColor"].push_back(in.checkersColor.second);
            json["checkersSize"] = in.checkersSize;
            json["gradientColor"].push_back(in.gradientColor.first);
            json["gradientColor"].push_back(in.gradientColor.second);
            json["outline"] = in.outline;
        }

        void from_json(const nlohmann::json& json, Outline& out)
        {
            json.at("enabled").get_to(out.enabled);
            json.at("width").get_to(out.width);
            json.at("color").get_to(out.color);
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
            json.at("outline").get_to(out.outline);
        }
    }
}
