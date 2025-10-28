// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlDevice/Init.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDSystem.h>
#endif // TLRENDER_BMD

#include <tlTimeline/Init.h>

#include <ftk/Core/Context.h>

namespace tl
{
    namespace device
    {
        void init(const std::shared_ptr<ftk::Context>& context)
        {
            timeline::init(context);
#if defined(TLRENDER_BMD)
            bmd::System::create(context);
#endif // TLRENDER_BMD
        }
    }
}
