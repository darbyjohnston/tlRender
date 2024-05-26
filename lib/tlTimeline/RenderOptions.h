// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Image.h>

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

            //! Offscreen color type.
            image::PixelType offscreenColorType = image::PixelType::RGBA_U8;

            //! Texture cache byte count.
            size_t textureCacheByteCount = memory::gigabyte / 4;

            bool operator == (const RenderOptions&) const;
            bool operator != (const RenderOptions&) const;
        };
    }
}

#include <tlTimeline/RenderOptionsInline.h>
