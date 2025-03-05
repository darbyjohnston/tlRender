// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/DisplayOptions.h>

#include <dtk/core/Error.h>
#include <dtk/core/String.h>

#include <algorithm>
#include <array>

namespace tl
{
    namespace timeline
    {
        dtk::M44F brightness(const dtk::V3F& value)
        {
            return dtk::M44F(
                value.x, 0.F, 0.F, 0.F,
                0.F, value.y, 0.F, 0.F,
                0.F, 0.F, value.z, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        dtk::M44F contrast(const dtk::V3F& value)
        {
            return
                dtk::M44F(
                    1.F, 0.F, 0.F, -.5F,
                    0.F, 1.F, 0.F, -.5F,
                    0.F, 0.F, 1.F, -.5F,
                    0.F, 0.F, 0.F, 1.F) *
                dtk::M44F(
                    value.x, 0.F, 0.F, 0.F,
                    0.F, value.y, 0.F, 0.F,
                    0.F, 0.F, value.z, 0.F,
                    0.F, 0.F, 0.F, 1.F) *
                dtk::M44F(
                    1.F, 0.F, 0.F, .5F,
                    0.F, 1.F, 0.F, .5F,
                    0.F, 0.F, 1.F, .5F,
                    0.F, 0.F, 0.F, 1.F);
        }

        dtk::M44F saturation(const dtk::V3F& value)
        {
            const dtk::V3F s(
                (1.F - value.x) * .3086F,
                (1.F - value.y) * .6094F,
                (1.F - value.z) * .0820F);
            return dtk::M44F(
                s.x + value.x, s.y, s.z, 0.F,
                s.x, s.y + value.y, s.z, 0.F,
                s.x, s.y, s.z + value.z, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        dtk::M44F tint(float v)
        {
            const float c = cos(v * dtk::pi * 2.F);
            const float c2 = 1.F - c;
            const float c3 = 1.F / 3.F * c2;
            const float s = sin(v * dtk::pi * 2.F);
            const float sq = sqrtf(1.F / 3.F);
            return dtk::M44F(
                c + c2 / 3.F, c3 - sq * s, c3 + sq * s, 0.F,
                c3 + sq * s, c + c3, c3 - sq * s, 0.F,
                c3 - sq * s, c3 + sq * s, c + c3, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        dtk::M44F color(const Color& in)
        {
            return
                brightness(in.brightness) *
                contrast(in.contrast) *
                saturation(in.saturation) *
                tint(in.tint);
        }

        void to_json(nlohmann::json& json, const Color& in)
        {
            json["enabled"] = in.enabled;
            json["add"] = in.add;
            json["brightness"] = in.brightness;
            json["contrast"] = in.contrast;
            json["saturation"] = in.saturation;
            json["tint"] = in.tint;
            json["invert"] = in.invert;
        }

        void to_json(nlohmann::json& json, const Levels& in)
        {
            json["enabled"] = in.enabled;
            json["inLow"] = in.inLow;
            json["inHigh"] = in.inHigh;
            json["gamma"] = in.gamma;
            json["outLow"] = in.outLow;
            json["outHigh"] = in.outHigh;
        }

        void to_json(nlohmann::json& json, const EXRDisplay& in)
        {
            json["enabled"] = in.enabled;
            json["exposure"] = in.exposure;
            json["defog"] = in.defog;
            json["kneeLow"] = in.kneeLow;
            json["kneeHigh"] = in.kneeHigh;
        }

        void to_json(nlohmann::json& json, const SoftClip& in)
        {
            json["enabled"] = in.enabled;
            json["value"] = in.value;
        }

        void to_json(nlohmann::json& json, const DisplayOptions& in)
        {
            json["channels"] = to_string(in.channels);
            json["color"] = in.color;
            json["levels"] = in.levels;
            json["exrDisplay"] = in.exrDisplay;
            json["softClip"] = in.softClip;
            json["imageFilters"] = in.imageFilters;
            json["videoLevels"] = to_string(in.videoLevels);
        }

        void from_json(const nlohmann::json& json, Color& out)
        {
            json.at("enabled").get_to(out.enabled);
            json.at("add").get_to(out.add);
            json.at("brightness").get_to(out.brightness);
            json.at("contrast").get_to(out.contrast);
            json.at("saturation").get_to(out.saturation);
            json.at("tint").get_to(out.tint);
            json.at("invert").get_to(out.invert);
        }

        void from_json(const nlohmann::json& json, Levels& out)
        {
            json.at("enabled").get_to(out.enabled);
            json.at("inLow").get_to(out.inLow);
            json.at("inHigh").get_to(out.inHigh);
            json.at("gamma").get_to(out.gamma);
            json.at("outLow").get_to(out.outLow);
            json.at("outHigh").get_to(out.outHigh);
        }

        void from_json(const nlohmann::json& json, EXRDisplay& out)
        {
            json.at("enabled").get_to(out.enabled);
            json.at("exposure").get_to(out.exposure);
            json.at("defog").get_to(out.defog);
            json.at("kneeLow").get_to(out.kneeLow);
            json.at("kneeHigh").get_to(out.kneeHigh);
        }

        void from_json(const nlohmann::json& json, SoftClip& out)
        {
            json.at("enabled").get_to(out.enabled);
            json.at("value").get_to(out.value);
        }

        void from_json(const nlohmann::json& json, DisplayOptions& out)
        {
            from_string(json.at("channels").get<std::string>(), out.channels);
            json.at("color").get_to(out.color);
            json.at("levels").get_to(out.levels);
            json.at("exrDisplay").get_to(out.exrDisplay);
            json.at("softClip").get_to(out.softClip);
            json.at("imageFilters").get_to(out.imageFilters);
            from_string(json.at("videoLevels").get<std::string>(), out.videoLevels);
        }
    }
}
