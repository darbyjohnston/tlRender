// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
        //! LUT operation order.
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

        //! Get the list of LUT format names.
        std::vector<std::string> getLUTFormatNames();

        //! Get the list of LUT format file extensions.
        std::vector<std::string> getLUTFormatExtensions();
    }
}

#include <tlTimeline/LUTOptionsInline.h>
