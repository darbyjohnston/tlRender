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
        //! Insert data.
        struct InsertData
        {
            otio::SerializableObject::Retainer<otio::Composable> composable;
            int trackIndex = 0;
            int insertIndex = 0;
        };

        //! Insert the given composables.
        otio::SerializableObject::Retainer<otio::Timeline> insert(
            const otio::SerializableObject::Retainer<otio::Timeline>&,
            const std::vector<InsertData>&);
    }
}
