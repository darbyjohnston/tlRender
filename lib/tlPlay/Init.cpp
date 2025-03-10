// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/Init.h>

#include <tlTimelineUI/Init.h>

#include <tlDevice/Init.h>

namespace tl
{
    namespace play
    {
        void init(const std::shared_ptr<dtk::Context>& context)
        {
            tl::timelineui::init(context);
            device::init(context);
        }
    }
}
