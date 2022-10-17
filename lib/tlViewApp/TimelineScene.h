// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlViewApp/TimelineItem.h>

#include <tlTimeline/IRender.h>

#include <opentimelineio/timeline.h>

namespace tl
{
    namespace view
    {
        //! Create a scene.
        std::shared_ptr<TimelineItem> createScene(otio::Timeline*);

        //! Draw a scene.
        void drawScene(
            const std::shared_ptr<TimelineItem>&,
            const std::shared_ptr<imaging::FontSystem>&,
            const std::shared_ptr<timeline::IRender>&);
    }
}
