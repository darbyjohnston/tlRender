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
        DTK_ENUM_IMPL(
            Channels,
            "Color",
            "Red",
            "Green",
            "Blue",
            "Alpha");

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
    }
}
