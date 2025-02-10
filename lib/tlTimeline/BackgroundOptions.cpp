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
            json["color0"] = in.color0;
            json["color1"] = in.color1;
            json["checkersSize"] = in.checkersSize;
        }

        void from_json(const nlohmann::json& json, BackgroundOptions& out)
        {
            json.at("type").get_to(out.type);
            json.at("color0").get_to(out.color0);
            json.at("color1").get_to(out.color1);
            json.at("checkersSize").get_to(out.checkersSize);
        }
    }
}
