// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Move data.
        struct MoveData
        {
            int fromTrack = 0;
            int fromIndex = 0;
            int toTrack   = 0;
            int toIndex   = 0;
        };

        //! Move the given items.
        otio::SerializableObject::Retainer<otio::Timeline> move(
            const otio::SerializableObject::Retainer<otio::Timeline>&,
            const std::vector<MoveData>&);
    }
}
