// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/DisplayOptions.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/String.h>

#include <algorithm>
#include <array>

namespace tl
{
    namespace timeline
    {
        bool Color::operator == (const Color& other) const
        {
            return
                enabled == other.enabled &&
                add == other.add &&
                brightness == other.brightness &&
                contrast == other.contrast &&
                saturation == other.saturation &&
                tint == other.tint &&
                invert == other.invert;
        }

        bool Color::operator != (const Color& other) const
        {
            return !(*this == other);
        }

        feather_tk::M44F brightness(const feather_tk::V3F& value)
        {
            return feather_tk::M44F(
                value.x, 0.F, 0.F, 0.F,
                0.F, value.y, 0.F, 0.F,
                0.F, 0.F, value.z, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        feather_tk::M44F contrast(const feather_tk::V3F& value)
        {
            return
                feather_tk::M44F(
                    1.F, 0.F, 0.F, -.5F,
                    0.F, 1.F, 0.F, -.5F,
                    0.F, 0.F, 1.F, -.5F,
                    0.F, 0.F, 0.F, 1.F) *
                feather_tk::M44F(
                    value.x, 0.F, 0.F, 0.F,
                    0.F, value.y, 0.F, 0.F,
                    0.F, 0.F, value.z, 0.F,
                    0.F, 0.F, 0.F, 1.F) *
                feather_tk::M44F(
                    1.F, 0.F, 0.F, .5F,
                    0.F, 1.F, 0.F, .5F,
                    0.F, 0.F, 1.F, .5F,
                    0.F, 0.F, 0.F, 1.F);
        }

        feather_tk::M44F saturation(const feather_tk::V3F& value)
        {
            const feather_tk::V3F s(
                (1.F - value.x) * .3086F,
                (1.F - value.y) * .6094F,
                (1.F - value.z) * .0820F);
            return feather_tk::M44F(
                s.x + value.x, s.y, s.z, 0.F,
                s.x, s.y + value.y, s.z, 0.F,
                s.x, s.y, s.z + value.z, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        feather_tk::M44F tint(float v)
        {
            const float c = cos(v * feather_tk::pi * 2.F);
            const float c2 = 1.F - c;
            const float c3 = 1.F / 3.F * c2;
            const float s = sin(v * feather_tk::pi * 2.F);
            const float sq = sqrtf(1.F / 3.F);
            return feather_tk::M44F(
                c + c2 / 3.F, c3 - sq * s, c3 + sq * s, 0.F,
                c3 + sq * s, c + c3, c3 - sq * s, 0.F,
                c3 - sq * s, c3 + sq * s, c + c3, 0.F,
                0.F, 0.F, 0.F, 1.F);
        }

        feather_tk::M44F color(const Color& in)
        {
            return
                feather_tk::brightness(in.brightness) *
                feather_tk::contrast(in.contrast) *
                feather_tk::saturation(in.saturation) *
                tint(in.tint);
        }

        bool Levels::operator == (const Levels& other) const
        {
            return
                enabled == other.enabled &&
                inLow == other.inLow &&
                inHigh == other.inHigh &&
                gamma == other.gamma &&
                outLow == other.outLow &&
                outHigh == other.outHigh;
        }

        bool Levels::operator != (const Levels& other) const
        {
            return !(*this == other);
        }

        bool EXRDisplay::operator == (const EXRDisplay& other) const
        {
            return
                enabled == other.enabled &&
                exposure == other.exposure &&
                defog == other.defog &&
                kneeLow == other.kneeLow &&
                kneeHigh == other.kneeHigh;
        }

        bool EXRDisplay::operator != (const EXRDisplay& other) const
        {
            return !(*this == other);
        }

        bool SoftClip::operator == (const SoftClip& other) const
        {
            return
                enabled == other.enabled &&
                value == other.value;
        }

        bool SoftClip::operator != (const SoftClip& other) const
        {
            return !(*this == other);
        }

        bool DisplayOptions::operator == (const DisplayOptions& other) const
        {
            return
                channels == other.channels &&
                mirror == other.mirror &&
                color == other.color &&
                levels == other.levels &&
                exrDisplay == other.exrDisplay &&
                softClip == other.softClip &&
                imageFilters == other.imageFilters &&
                videoLevels == other.videoLevels;
        }

        bool DisplayOptions::operator != (const DisplayOptions& other) const
        {
            return !(*this == other);
        }

        void to_json(nlohmann::json& json, const Color& in)
        {
            json["Enabled"] = in.enabled;
            json["Add"] = in.add;
            json["Brightness"] = in.brightness;
            json["Contrast"] = in.contrast;
            json["Saturation"] = in.saturation;
            json["Tint"] = in.tint;
            json["Invert"] = in.invert;
        }

        void to_json(nlohmann::json& json, const Levels& in)
        {
            json["Enabled"] = in.enabled;
            json["InLow"] = in.inLow;
            json["InHigh"] = in.inHigh;
            json["Gamma"] = in.gamma;
            json["OutLow"] = in.outLow;
            json["OutHigh"] = in.outHigh;
        }

        void to_json(nlohmann::json& json, const EXRDisplay& in)
        {
            json["Enabled"] = in.enabled;
            json["Exposure"] = in.exposure;
            json["Defog"] = in.defog;
            json["KneeLow"] = in.kneeLow;
            json["KneeHigh"] = in.kneeHigh;
        }

        void to_json(nlohmann::json& json, const SoftClip& in)
        {
            json["Enabled"] = in.enabled;
            json["Value"] = in.value;
        }

        void to_json(nlohmann::json& json, const DisplayOptions& in)
        {
            json["Channels"] = to_string(in.channels);
            json["Mirror"] = in.mirror;
            json["Color"] = in.color;
            json["Levels"] = in.levels;
            json["EXRDisplay"] = in.exrDisplay;
            json["SoftClip"] = in.softClip;
            json["ImageFilters"] = in.imageFilters;
            json["VideoLevels"] = to_string(in.videoLevels);
        }

        void from_json(const nlohmann::json& json, Color& out)
        {
            json.at("Enabled").get_to(out.enabled);
            json.at("Add").get_to(out.add);
            json.at("Brightness").get_to(out.brightness);
            json.at("Contrast").get_to(out.contrast);
            json.at("Saturation").get_to(out.saturation);
            json.at("Tint").get_to(out.tint);
            json.at("Invert").get_to(out.invert);
        }

        void from_json(const nlohmann::json& json, Levels& out)
        {
            json.at("Enabled").get_to(out.enabled);
            json.at("InLow").get_to(out.inLow);
            json.at("InHigh").get_to(out.inHigh);
            json.at("Gamma").get_to(out.gamma);
            json.at("OutLow").get_to(out.outLow);
            json.at("OutHigh").get_to(out.outHigh);
        }

        void from_json(const nlohmann::json& json, EXRDisplay& out)
        {
            json.at("Enabled").get_to(out.enabled);
            json.at("Exposure").get_to(out.exposure);
            json.at("Defog").get_to(out.defog);
            json.at("KneeLow").get_to(out.kneeLow);
            json.at("KneeHigh").get_to(out.kneeHigh);
        }

        void from_json(const nlohmann::json& json, SoftClip& out)
        {
            json.at("Enabled").get_to(out.enabled);
            json.at("Value").get_to(out.value);
        }

        void from_json(const nlohmann::json& json, DisplayOptions& out)
        {
            from_string(json.at("Channels").get<std::string>(), out.channels);
            json.at("Mirror").get_to(out.mirror);
            json.at("Color").get_to(out.color);
            json.at("Levels").get_to(out.levels);
            json.at("EXRDisplay").get_to(out.exrDisplay);
            json.at("SoftClip").get_to(out.softClip);
            json.at("ImageFilters").get_to(out.imageFilters);
            from_string(json.at("VideoLevels").get<std::string>(), out.videoLevels);
        }
    }
}
