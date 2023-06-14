// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Action.h>

namespace tl
{
    namespace ui
    {
        struct Action::Private
        {
            std::weak_ptr<system::Context> context;
            std::string text;
            Key shortcut = Key::Unknown;
            int shortcutModifiers = 0;
            std::string icon;
            std::function<void(void)> pressedCallback;
            std::function<void(void)> clickedCallback;
        };

        void Action::_init(const std::shared_ptr<system::Context>& context)
        {
        }

        Action::Action() :
            _p(new Private)
        {}

        Action::~Action()
        {}

        std::shared_ptr<Action> Action::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<Action>(new Action);
            out->_init(context);
            return out;
        }

        const std::string& Action::getText() const
        {
            return _p->text;
        }

        void Action::setText(const std::string& value)
        {
            _p->text = value;
        }

        Key Action::getShortcut() const
        {
            return _p->shortcut;
        }

        int Action::getShortcutModifiers() const
        {
            return _p->shortcutModifiers;
        }

        void Action::setShortut(Key key, int modifiers)
        {
            _p->shortcut = key;
            _p->shortcutModifiers = modifiers;
        }

        const std::string& Action::getIcon() const
        {
            return _p->icon;
        }

        void Action::setIcon(const std::string& value)
        {
            _p->icon = value;
        }

        void Action::setPressedCallback(const std::function<void(void)>& value)
        {
            _p->pressedCallback = value;
        }

        void Action::doPressedCallback()
        {
            TLRENDER_P();
            if (p.pressedCallback)
            {
                p.pressedCallback();
            }
        }

        void Action::setClickedCallback(const std::function<void(void)>& value)
        {
            _p->clickedCallback = value;
        }

        void Action::doClickedCallback()
        {
            TLRENDER_P();
            if (p.clickedCallback)
            {
                p.clickedCallback();
            }
        }
    }
}
