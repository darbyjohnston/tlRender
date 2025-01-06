// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

namespace tl
{
    namespace ui
    {
        class Action;

        //! Menu bar.
        class MenuBar : public IWidget
        {
            TLRENDER_NON_COPYABLE(MenuBar);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            MenuBar();

        public:
            virtual ~MenuBar();

            //! Create a new widget.
            static std::shared_ptr<MenuBar> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Add a menu.
            void addMenu(
                const std::string& text,
                const std::shared_ptr<Menu>&);

            //! Handle keyboard shortcuts.
            bool shortcut(Key, int);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
