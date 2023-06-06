// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Menu item.
        class MenuItem : public std::enable_shared_from_this<MenuItem>
        {
            TLRENDER_NON_COPYABLE(MenuItem);

        protected:
            void _init(
                const std::string& text,
                const std::shared_ptr<MenuItem>& parent = nullptr);

            MenuItem();

        public:
            ~MenuItem();
            
            static std::shared_ptr<MenuItem> create(
                const std::string& text,
                const std::shared_ptr<MenuItem>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
