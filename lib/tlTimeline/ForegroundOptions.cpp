// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/ForegroundOptions.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <sstream>

namespace tl
{
    namespace timeline
    {
        bool Grid::operator == (const Grid& other) const
        {
            return
                enabled == other.enabled &&
                size == other.size &&
                lineWidth == other.lineWidth &&
                color == other.color;
        }

        bool Grid::operator != (const Grid& other) const
        {
            return !(*this == other);
        }

        bool ForegroundOptions::operator == (const ForegroundOptions& other) const
        {
            return
                grid == other.grid;
        }

        bool ForegroundOptions::operator != (const ForegroundOptions& other) const
        {
            return !(*this == other);
        }

        void to_json(nlohmann::json& json, const Grid& in)
        {
            json["enabled"] = in.enabled;
            json["size"] = in.size;
            json["lineWidth"] = in.lineWidth;
            json["color"] = in.color;
        }

        void to_json(nlohmann::json& json, const ForegroundOptions& in)
        {
            json["grid"] = in.grid;
        }

        void from_json(const nlohmann::json& json, Grid& out)
        {
            json.at("enabled").get_to(out.enabled);
            json.at("size").get_to(out.size);
            json.at("lineWidth").get_to(out.lineWidth);
            json.at("color").get_to(out.color);
        }

        void from_json(const nlohmann::json& json, ForegroundOptions& out)
        {
            json.at("grid").get_to(out.grid);
        }
    }
}
