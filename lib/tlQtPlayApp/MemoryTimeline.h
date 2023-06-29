// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <tlCore/FileIO.h>

namespace tl
{
    namespace qtplay
    {
        //! For each clip in the timeline, load the associated media into
        //! memory and replace the media references with memory references.
        void createMemoryTimeline(
            otio::Timeline*,
            const std::string& directory,
            const file::PathOptions&);
    }
}
