// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>

namespace tl
{
    namespace timeline
    {
        //! Rendering options.
        struct RenderOptions
        {
            //! Clear the canvas before rendering.
            bool clear = true;

            //! Clear color.
            image::Color4f clearColor;

            //! Texture buffers size.
            size_t textureBuffersSize = 6;

            bool operator == (const RenderOptions&) const;
            bool operator != (const RenderOptions&) const;
        };
    }
}

#include <tlTimeline/RenderOptionsInline.h>
