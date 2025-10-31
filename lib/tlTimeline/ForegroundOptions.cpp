// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/ForegroundOptions.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/String.h>

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

        bool ForegroundOptions::operator == (const ForegroundOptions& other) const
        {
            return
                grid == other.grid &&
                outline == other.outline;
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

        void to_json(nlohmann::json& json, const Outline& in)
        {
            json["Enabled"] = in.enabled;
            json["Width"] = in.width;
            json["Color"] = in.color;
        }

        void to_json(nlohmann::json& json, const ForegroundOptions& in)
        {
            json["Grid"] = in.grid;
            json["Outline"] = in.outline;
        }

        void from_json(const nlohmann::json& json, Grid& out)
        {
            json.at("Enabled").get_to(out.enabled);
            json.at("Size").get_to(out.size);
            json.at("LineWidth").get_to(out.lineWidth);
            json.at("Color").get_to(out.color);
        }

        void from_json(const nlohmann::json& json, Outline& out)
        {
            json.at("Enabled").get_to(out.enabled);
            json.at("Width").get_to(out.width);
            json.at("Color").get_to(out.color);
        }

        void from_json(const nlohmann::json& json, ForegroundOptions& out)
        {
            json.at("Grid").get_to(out.grid);
            json.at("Outline").get_to(out.outline);
        }
    }
}
