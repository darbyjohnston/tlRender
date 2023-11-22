// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Base class for windows.
        class IWindow : public IWidget
        {
            TLRENDER_NON_COPYABLE(IWindow);

        protected:
            IWindow();

        public:
            virtual ~IWindow() = 0;
        };
    }
}
