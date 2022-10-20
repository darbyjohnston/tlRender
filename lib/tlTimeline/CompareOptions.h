// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

namespace tl
{
    namespace timeline
    {
        //! Comparison mode.
        enum class CompareMode
        {
            A,
            B,
            Wipe,
            Overlay,
            Difference,
            Horizontal,
            Vertical,
            Tile,

            Count,
            First = A
        };
        TLRENDER_ENUM(CompareMode);
        TLRENDER_ENUM_SERIALIZE(CompareMode);

        //! Comparison options.
        struct CompareOptions
        {
            CompareMode mode = CompareMode::A;
            math::Vector2f wipeCenter = math::Vector2f(.5F, .5F);
            float wipeRotation = 0.F;
            float overlay = .5F;

            bool operator == (const CompareOptions&) const;
            bool operator != (const CompareOptions&) const;
        };

        //! Get the bounding boxes for the given compare mode and sizes.
        std::vector<math::BBox2i> tiles(CompareMode, const std::vector<imaging::Size>&);

        //! Get the render size for the given compare mode and sizes.
        imaging::Size getRenderSize(CompareMode, const std::vector<imaging::Size>&);
    }
}

#include <tlTimeline/CompareOptionsInline.h>
