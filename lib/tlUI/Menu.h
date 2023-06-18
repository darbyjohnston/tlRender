// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IMenuPopup.h>

namespace tl
{
    namespace ui
    {
        // Menu item.
        struct MenuItem
        {
            MenuItem();
            MenuItem(
                const std::string&               text,
                const std::function<void(void)>& callback);
            MenuItem(
                const std::string&               text,
                const std::string&               icon,
                const std::function<void(void)>& callback);
            MenuItem(
                const std::string&               text,
                Key                              shortcut,
                int                              shortcutModifiers,
                const std::function<void(void)>& callback);
            MenuItem(
                const std::string&               text,
                const std::string&               icon,
                Key                              shortcut,
                int                              shortcutModifiers,
                const std::function<void(void)>& callback);
            MenuItem(
                const std::string&               text,
                const std::function<void(bool)>& checkedCallback);
            MenuItem(
                const std::string&               text,
                const std::string&               icon,
                const std::function<void(bool)>& checkedCallback);
            MenuItem(
                const std::string&               text,
                Key                              shortcut,
                int                              shortcutModifiers,
                const std::function<void(bool)>& checkedCallback);
            MenuItem(
                const std::string&               text,
                const std::string&               icon,
                Key                              shortcut,
                int                              shortcutModifiers,
                const std::function<void(bool)>& checkedCallback);

            std::string               text;
            std::string               icon;
            Key                       shortcut          = Key::Unknown;
            int                       shortcutModifiers = 0;
            std::function<void(void)> callback;
            bool                      checkable         = false;
            bool                      checked           = false;
            std::function<void(bool)> checkedCallback;
        };

        //! Menu.
        class Menu : public IMenuPopup
        {
            TLRENDER_NON_COPYABLE(Menu);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Menu();

        public:
            ~Menu() override;

            //! Create a new widget.
            static std::shared_ptr<Menu> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Add a menu item.
            void addItem(const std::shared_ptr<MenuItem>&);

            //! Set whether a menu item is checked.
            void setItemChecked(const std::shared_ptr<MenuItem>&, bool);

            //! Set whether a menu item is enabled.
            void setItemEnabled(const std::shared_ptr<MenuItem>&, bool);

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
