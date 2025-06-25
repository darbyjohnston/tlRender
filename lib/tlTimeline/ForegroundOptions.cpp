// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/ForegroundOptions.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/String.h>

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
            json["Enabled"] = in.enabled;
            json["Size"] = in.size;
            json["LineWidth"] = in.lineWidth;
            json["Color"] = in.color;
        }

        void to_json(nlohmann::json& json, const ForegroundOptions& in)
        {
            json["Grid"] = in.grid;
        }

        void from_json(const nlohmann::json& json, Grid& out)
        {
            json.at("Enabled").get_to(out.enabled);
            json.at("Size").get_to(out.size);
            json.at("LineWidth").get_to(out.lineWidth);
            json.at("Color").get_to(out.color);
        }

        void from_json(const nlohmann::json& json, ForegroundOptions& out)
        {
            json.at("Grid").get_to(out.grid);
        }
    }
}
