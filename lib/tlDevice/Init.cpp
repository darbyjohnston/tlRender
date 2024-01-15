// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlDevice/Init.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDSystem.h>
#endif // TLRENDER_BMD

#include <tlTimeline/Init.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace device
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            timeline::init(context);
#if defined(TLRENDER_BMD)
            if (!context->getSystem<BMDSystem>())
            {
                context->addSystem(BMDSystem::create(context));
            }
#endif // TLRENDER_BMD
        }
    }
}
