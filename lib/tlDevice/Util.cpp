// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/Util.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDDeviceSystem.h>
#endif // TLRENDER_BMD

#include <tlCore/Context.h>

namespace tl
{
    namespace device
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
#if defined(TLRENDER_BMD)
            if (!context->getSystem<BMDDeviceSystem>())
            {
                context->addSystem(BMDDeviceSystem::create(context));
            }
#endif // TLRENDER_BMD
        }
    }
}
