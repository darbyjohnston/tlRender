// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);

        //! Convert frames to ranges.
        std::vector<otime::TimeRange> toRanges(std::vector<otime::RationalTime>);

        //! Get the root (highest parent).
        const otio::Composable* getRoot(const otio::Composable*);

        //! Get the parent of the given type.
        template<typename T>
        const T* getParent(const otio::Item*);

        //! Get the duration of all tracks of the same kind.
        otio::optional<otime::RationalTime> getDuration(const otio::Timeline*, const std::string& kind);

        //! Get a list of files to open from the given path.
        std::vector<file::Path> getPaths(
            const std::string&,
            const file::PathOptions&,
            const std::shared_ptr<system::Context>&);
    }
}

#include <tlTimeline/UtilInline.h>
