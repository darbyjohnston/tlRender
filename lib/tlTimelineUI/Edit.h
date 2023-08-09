// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

#include <opentimelineio/item.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace timelineui
    {
        otio::SerializableObject::Retainer<otio::Timeline> insert(
            const otio::Timeline*,
            const otio::Item*,
            int trackIndex,
            int insertIndex);
    }
}
