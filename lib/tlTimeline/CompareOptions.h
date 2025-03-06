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
        //! Comparison modes.
        enum class Compare
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
        DTK_ENUM(Compare);

        //! Comparison time modes.
        enum class CompareTime
        {
            Relative,
            Absolute,

            Count,
            First = Relative
        };
        DTK_ENUM(CompareTime);

        //! Comparison options.
        struct CompareOptions
        {
            Compare  compare      = Compare::A;
            dtk::V2F wipeCenter   = dtk::V2F(.5F, .5F);
            float    wipeRotation = 0.F;
            float    overlay      = .5F;

            bool operator == (const CompareOptions&) const;
            bool operator != (const CompareOptions&) const;
        };

        //! Get the boxes for the given compare mode.
        std::vector<dtk::Box2I> getBoxes(Compare, const std::vector<dtk::ImageInfo>&);

        //! Get the boxes for the given compare mode.
        std::vector<dtk::Box2I> getBoxes(Compare, const std::vector<VideoData>&);

        //! Get the render size for the given compare mode.
        dtk::Size2I getRenderSize(Compare, const std::vector<dtk::ImageInfo>&);

        //! Get the render size for the given compare mode.
        dtk::Size2I getRenderSize(Compare, const std::vector<VideoData>&);

        //! Get a compare time.
        OTIO_NS::RationalTime getCompareTime(
            const OTIO_NS::RationalTime& sourceTime,
            const OTIO_NS::TimeRange& sourceTimeRange,
            const OTIO_NS::TimeRange& compareTimeRange,
            CompareTime);

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const CompareOptions&);

        void from_json(const nlohmann::json&, CompareOptions&);

        ///@}
    }
}
