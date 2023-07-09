// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Render a screenshot of a widget.
        std::shared_ptr<imaging::Image> screenshot(
            const std::shared_ptr<IWidget>&,
            const imaging::Size& displaySize,
            const std::shared_ptr<Style>&,
            const std::shared_ptr<IconLibrary>&,
            const std::shared_ptr<timeline::IRender>&,
            const std::shared_ptr<imaging::FontSystem>&,
            float displayScale);
    }
}
