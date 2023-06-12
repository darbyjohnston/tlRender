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

        void Action::setClickedCallback(const std::function<void(void)>& value)
        {
            _p->clickedCallback = value;
        }

        void Action::click()
        {
            TLRENDER_P();
            if (p.clickedCallback)
            {
                p.clickedCallback();
            }
        }
    }
}
