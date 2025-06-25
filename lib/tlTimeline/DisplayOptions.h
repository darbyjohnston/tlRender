// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/Image.h>
#include <feather-tk/core/Matrix.h>
#include <feather-tk/core/RenderOptions.h>

namespace tl
{
    namespace timeline
    {
        //! Color values.
        struct Color
        {
        public:
            bool     enabled    = false;
            feather_tk::V3F add        = feather_tk::V3F(0.F, 0.F, 0.F);
            feather_tk::V3F brightness = feather_tk::V3F(1.F, 1.F, 1.F);
            feather_tk::V3F contrast   = feather_tk::V3F(1.F, 1.F, 1.F);
            feather_tk::V3F saturation = feather_tk::V3F(1.F, 1.F, 1.F);
            float    tint       = 0.F;
            bool     invert     = false;

            bool operator == (const Color&) const;
            bool operator != (const Color&) const;
        };

        //! Get a brightness color matrix.
        feather_tk::M44F brightness(const feather_tk::V3F&);

        //! Get a contrast color matrix.
        feather_tk::M44F contrast(const feather_tk::V3F&);

        //! Get a saturation color matrix.
        feather_tk::M44F saturation(const feather_tk::V3F&);

        //! Get a tint color matrix.
        feather_tk::M44F tint(float);

        //! Get a color matrix.
        feather_tk::M44F color(const Color&);

        //! Levels values.
        struct Levels
        {
            bool  enabled = false;
            float inLow   = 0.F;
            float inHigh  = 1.F;
            float gamma   = 1.F;
            float outLow  = 0.F;
            float outHigh = 1.F;

            bool operator == (const Levels&) const;
            bool operator != (const Levels&) const;
        };

        //! These values match the ones in exrdisplay for comparison and
        //! testing.
        struct EXRDisplay
        {
            bool  enabled  = false;
            float exposure = 0.F;
            float defog    = 0.F;
            float kneeLow  = 0.F;
            float kneeHigh = 5.F;

            bool operator == (const EXRDisplay&) const;
            bool operator != (const EXRDisplay&) const;
        };

        //! Soft clip.
        struct SoftClip
        {
            bool  enabled = false;
            float value   = 0.F;

            bool operator == (const SoftClip&) const;
            bool operator != (const SoftClip&) const;
        };

        //! Display options.
        struct DisplayOptions
        {
            feather_tk::ChannelDisplay channels     = feather_tk::ChannelDisplay::Color;
            feather_tk::ImageMirror    mirror;
            Color               color;
            Levels              levels;
            EXRDisplay          exrDisplay;
            SoftClip            softClip;
            feather_tk::ImageFilters   imageFilters;
            feather_tk::VideoLevels    videoLevels  = feather_tk::VideoLevels::FullRange;

            bool operator == (const DisplayOptions&) const;
            bool operator != (const DisplayOptions&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Color&);
        void to_json(nlohmann::json&, const Levels&);
        void to_json(nlohmann::json&, const EXRDisplay&);
        void to_json(nlohmann::json&, const SoftClip&);
        void to_json(nlohmann::json&, const DisplayOptions&);

        void from_json(const nlohmann::json&, Color&);
        void from_json(const nlohmann::json&, Levels&);
        void from_json(const nlohmann::json&, EXRDisplay&);
        void from_json(const nlohmann::json&, SoftClip&);
        void from_json(const nlohmann::json&, DisplayOptions&);

        ///@}
    }
}
