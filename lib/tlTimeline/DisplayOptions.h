// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/Image.h>
#include <ftk/Core/Matrix.h>
#include <ftk/Core/RenderOptions.h>

namespace tl
{
    namespace timeline
    {
        //! Color values.
        struct Color
        {
        public:
            bool            enabled    = false;
            ftk::V3F add        = ftk::V3F(0.F, 0.F, 0.F);
            ftk::V3F brightness = ftk::V3F(1.F, 1.F, 1.F);
            ftk::V3F contrast   = ftk::V3F(1.F, 1.F, 1.F);
            ftk::V3F saturation = ftk::V3F(1.F, 1.F, 1.F);
            float           tint       = 0.F;
            bool            invert     = false;

            bool operator == (const Color&) const;
            bool operator != (const Color&) const;
        };

        //! Get a brightness color matrix.
        ftk::M44F brightness(const ftk::V3F&);

        //! Get a contrast color matrix.
        ftk::M44F contrast(const ftk::V3F&);

        //! Get a saturation color matrix.
        ftk::M44F saturation(const ftk::V3F&);

        //! Get a tint color matrix.
        ftk::M44F tint(float);

        //! Get a color matrix.
        ftk::M44F color(const Color&);

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
            ftk::ChannelDisplay channels     = ftk::ChannelDisplay::Color;
            ftk::ImageMirror    mirror;
            Color               color;
            Levels              levels;
            EXRDisplay          exrDisplay;
            SoftClip            softClip;
            ftk::ImageFilters   imageFilters;
            ftk::VideoLevels    videoLevels  = ftk::VideoLevels::FullRange;

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
