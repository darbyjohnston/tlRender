// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace view
    {
        //! Draw a border.
        void drawBorder(
            const math::BBox2i&,
            int width,
            const imaging::Color4f&,
            const std::shared_ptr<timeline::IRender>&);
    }
}
