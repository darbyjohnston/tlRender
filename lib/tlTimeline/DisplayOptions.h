// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Color.h>
#include <tlCore/Image.h>
#include <tlCore/Matrix.h>

namespace tl
{
    namespace timeline
    {
        //! Channels.
        enum class Channels
        {
            Color,
            Red,
            Green,
            Blue,
            Alpha,

            Count,
            First = Color
        };
        TLRENDER_ENUM(Channels);
        TLRENDER_ENUM_SERIALIZE(Channels);

        //! Color values.
        struct Color
        {
        public:
            math::Vector3f add = math::Vector3f(0.F, 0.F, 0.F);
            math::Vector3f brightness = math::Vector3f(1.F, 1.F, 1.F);
            math::Vector3f contrast = math::Vector3f(1.F, 1.F, 1.F);
            math::Vector3f saturation = math::Vector3f(1.F, 1.F, 1.F);
            float tint = 0.F;
            bool invert = false;

            bool operator == (const Color&) const;
            bool operator != (const Color&) const;
        };

        //! Get a brightness color matrix.
        math::Matrix4x4f brightness(const math::Vector3f&);

        //! Get a contrast color matrix.
        math::Matrix4x4f contrast(const math::Vector3f&);

        //! Get a saturation color matrix.
        math::Matrix4x4f saturation(const math::Vector3f&);

        //! Get a tint color matrix.
        math::Matrix4x4f tint(float);

        //! Get a color matrix.
        math::Matrix4x4f color(const Color&);

        //! Levels values.
        struct Levels
        {
            float inLow = 0.F;
            float inHigh = 1.F;
            float gamma = 1.F;
            float outLow = 0.F;
            float outHigh = 1.F;

            bool operator == (const Levels&) const;
            bool operator != (const Levels&) const;
        };

        //! Exposure values.
        struct Exposure
        {
            float exposure = 0.F;
            float defog = 0.F;
            float kneeLow = 0.F;
            float kneeHigh = 5.F;

            bool operator == (const Exposure&) const;
            bool operator != (const Exposure&) const;
        };

        //! Display options.
        struct DisplayOptions
        {
            Channels             channels = Channels::Color;
            imaging::Mirror      mirror;
            bool                 colorEnabled = false;
            Color                color;
            bool                 levelsEnabled = false;
            Levels               levels;
            bool                 exposureEnabled = false;
            Exposure             exposure;
            bool                 softClipEnabled = false;
            float                softClip = 0.F;
            ImageFilters         imageFilters;
            imaging::VideoLevels videoLevels = imaging::VideoLevels::FullRange;

            bool operator == (const DisplayOptions&) const;
            bool operator != (const DisplayOptions&) const;
        };
    }
}

#include <tlTimeline/DisplayOptionsInline.h>
