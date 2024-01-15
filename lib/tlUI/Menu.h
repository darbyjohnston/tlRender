// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>
#include <tlUI/IMenuPopup.h>

namespace tl
{
    namespace ui
    {
        //! Menu.
        //!
        //! \todo Automatically open/close sub menus.
        class Menu : public IMenuPopup
        {
            TLRENDER_NON_COPYABLE(Menu);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Menu();

        public:
            virtual ~Menu();

            //! Create a new widget.
            static std::shared_ptr<Menu> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Add a menu item.
            void addItem(const std::shared_ptr<Action>&);

            //! Set whether a menu item is checked.
            void setItemChecked(const std::shared_ptr<Action>&, bool);

            //! Set whether a menu item is enabled.
            void setItemEnabled(const std::shared_ptr<Action>&, bool);

            //! Add a sub menu.
            std::shared_ptr<Menu> addSubMenu(const std::string&);

            //! Add a divider.
            void addDivider();

            //! Clear the menu.
            void clear();

            //! Handle keyboard shortcuts.
            bool shortcut(Key, int);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
