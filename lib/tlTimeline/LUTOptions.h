// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <iostream>
#include <string>
#include <vector>

namespace tl
{
    namespace timeline
    {
        //! LUT order.
        enum class LUTOrder
        {
            PostColorConfig,
            PreColorConfig,

            Count,
            First = PostColorConfig
        };
        TLRENDER_ENUM(LUTOrder);
        TLRENDER_ENUM_SERIALIZE(LUTOrder);

        //! LUT options.
        struct LUTOptions
        {
            std::string fileName;
            LUTOrder order = LUTOrder::First;

            bool operator == (const LUTOptions&) const;
            bool operator != (const LUTOptions&) const;
        };
    }
}

#include <tlTimeline/LUTOptionsInline.h>
