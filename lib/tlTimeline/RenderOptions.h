// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once


namespace tl
{
    namespace timeline
    {
        //! Rendering options.
        struct RenderOptions
        {
            //! Clear canvas before rendering.
            bool clear = true;

            bool operator == (const RenderOptions&) const;
            bool operator != (const RenderOptions&) const;
        };
    }
}

#include <tlTimeline/RenderOptionsInline.h>
