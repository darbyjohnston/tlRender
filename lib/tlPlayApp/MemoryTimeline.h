// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Timeline.h>

#include <tlCore/FileIO.h>

namespace tl
{
    namespace play
    {
        //! Load media into memory and replace media references with memory
        //! references.
        void createMemoryTimeline(
            otio::Timeline*,
            const std::string& directory,
            const file::PathOptions&);
    }
}
