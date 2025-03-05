// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Image.h>
#include <dtk/core/Matrix.h>
#include <dtk/core/RenderOptions.h>

namespace tl
{
    namespace timeline
    {
        //! Color values.
        struct Color
        {
        public:
            bool     enabled    = false;
            dtk::V3F add        = dtk::V3F(0.F, 0.F, 0.F);
            dtk::V3F brightness = dtk::V3F(1.F, 1.F, 1.F);
            dtk::V3F contrast   = dtk::V3F(1.F, 1.F, 1.F);
            dtk::V3F saturation = dtk::V3F(1.F, 1.F, 1.F);
            float    tint       = 0.F;
            bool     invert     = false;

            bool operator == (const Color&) const;
            bool operator != (const Color&) const;
        };

        //! Get a brightness color matrix.
        dtk::M44F brightness(const dtk::V3F&);

        //! Get a contrast color matrix.
        dtk::M44F contrast(const dtk::V3F&);

        //! Get a saturation color matrix.
        dtk::M44F saturation(const dtk::V3F&);

        //! Get a tint color matrix.
        dtk::M44F tint(float);

        //! Get a color matrix.
        dtk::M44F color(const Color&);

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
            dtk::ChannelDisplay channels     = dtk::ChannelDisplay::Color;
            dtk::ImageMirror    mirror;
            Color               color;
            Levels              levels;
            EXRDisplay          exrDisplay;
            SoftClip            softClip;
            dtk::ImageFilters   imageFilters;
            dtk::VideoLevels    videoLevels  = dtk::VideoLevels::FullRange;

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
