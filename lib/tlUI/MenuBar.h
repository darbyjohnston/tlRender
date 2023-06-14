// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        //! Menu bar.
        class MenuBar : public IWidget
        {
            TLRENDER_NON_COPYABLE(MenuBar);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            MenuBar();

        public:
            ~MenuBar() override;

            //! Create a new widget.
            static std::shared_ptr<MenuBar> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Add a menu.
            void addMenu(
                const std::string& text,
                const std::shared_ptr<IPopup>&);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
