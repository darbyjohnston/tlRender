// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <dtk/core/Color.h>

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
            dtk::Color4F clearColor;

            //! Color buffer type.
            image::PixelType colorBuffer = image::PixelType::RGBA_U8;

            //! Texture cache byte count.
            size_t textureCacheByteCount = memory::gigabyte / 4;

            bool operator == (const RenderOptions&) const;
            bool operator != (const RenderOptions&) const;
        };
    }
}

#include <tlTimeline/RenderOptionsInline.h>
