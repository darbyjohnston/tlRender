// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>

namespace tl
{
    namespace ui
    {
        //! Action.
        class Action : public std::enable_shared_from_this<Action>
        {
            TLRENDER_NON_COPYABLE(Action);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            Action();

        public:
            ~Action();

            //! Create a new action.
            static std::shared_ptr<Action> create(
                const std::shared_ptr<system::Context>&);

            //! Get the text.
            const std::string& getText() const;

            //! Set the text.
            void setText(const std::string&);

            //! Get the keyboard shortcut.
            Key getShortcut() const;

            //! Get the keyboard shortcut modifiers.
            int getShortcutModifiers() const;

            //! Set the keyboard shortcut.
            void setShortut(Key, int modifiers = 0);

            //! Get the icon.
            const std::string& getIcon() const;

            //! Set the icon.
            void setIcon(const std::string&);

            //! Set the pressed callback.
            void setPressedCallback(const std::function<void(void)>&);

            //! Activate the pressed callback.
            void doPressedCallback();

            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            //! Activate the clicked callback.
            void doClickedCallback();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
