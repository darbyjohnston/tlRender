// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrTimeline/Util.h>

#include <opentimelineio/timeline.h>

namespace tlr
{
    //! Timeline
    namespace timeline
    {
        //! Read a timeline.
        otio::SerializableObject::Retainer<otio::Timeline> read(
            const std::string&,
            otio::ErrorStatus*);
    }
}
