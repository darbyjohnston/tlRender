// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/Init.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Init.h>

#include <tlResource/Resource.h>

#include <dtk/ui/Init.h>
#include <dtk/ui/IconSystem.h>

namespace tl
{
    namespace timelineui
    {
        void init(const std::shared_ptr<dtk::Context>& context)
        {
            tl::timeline::init(context);
            dtk::uiInit(context);
            ThumbnailSystem::create(context);

            auto iconSystem = context->getSystem<dtk::IconSystem>();
            for (const auto& resource : tl::getIconResources())
            {
                iconSystem->add(resource, tl::getIconResource(resource));
            }
        }
    }
}
