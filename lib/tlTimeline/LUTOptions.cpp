// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlTimeline/LUTOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <array>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(
            LUTOrder,
            "PostColorConfig",
            "PreColorConfig");
        TLRENDER_ENUM_SERIALIZE_IMPL(LUTOrder);
    }
}
