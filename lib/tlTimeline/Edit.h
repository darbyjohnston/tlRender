// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timeline
    {
        //! Copy the given timeline.
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> copy(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&);

        //! Move items data.
        struct MoveData
        {
            int fromTrack = 0;
            int fromIndex = 0;
            int toTrack   = 0;
            int toIndex   = 0;
        };

        //! Move items in the timeline.
        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> move(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            const std::vector<MoveData>&);
    }
}
