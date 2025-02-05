// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Video.h>

#include <dtk/core/Box.h>

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
        DTK_ENUM(CompareMode);

        //! Comparison time mode.
        enum class CompareTimeMode
        {
            Relative,
            Absolute,

            Count,
            First = Relative
        };
        DTK_ENUM(CompareTimeMode);

        //! Comparison options.
        struct CompareOptions
        {
            CompareMode mode         = CompareMode::A;
            dtk::V2F    wipeCenter   = dtk::V2F(.5F, .5F);
            float       wipeRotation = 0.F;
            float       overlay      = .5F;

            bool operator == (const CompareOptions&) const;
            bool operator != (const CompareOptions&) const;
        };

        //! Get the boxes for the given compare mode.
        std::vector<dtk::Box2I> getBoxes(CompareMode, const std::vector<dtk::ImageInfo>&);

        //! Get the boxes for the given compare mode.
        std::vector<dtk::Box2I> getBoxes(CompareMode, const std::vector<VideoData>&);

        //! Get the render size for the given compare mode.
        dtk::Size2I getRenderSize(CompareMode, const std::vector<dtk::ImageInfo>&);

        //! Get the render size for the given compare mode.
        dtk::Size2I getRenderSize(CompareMode, const std::vector<VideoData>&);

        //! Get a compare time.
        OTIO_NS::RationalTime getCompareTime(
            const OTIO_NS::RationalTime& sourceTime,
            const OTIO_NS::TimeRange& sourceTimeRange,
            const OTIO_NS::TimeRange& compareTimeRange,
            CompareTimeMode);
    }
}

#include <tlTimeline/CompareOptionsInline.h>
