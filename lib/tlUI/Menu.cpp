// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Menu.h>

#include <tlUI/Divider.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct Menu::Private
        {
            std::shared_ptr<VerticalLayout> layout;
        };

        void Menu::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IPopup::_init("tl::ui::Menu", context, parent);
            TLRENDER_P();
            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
        }

        Menu::Menu() :
            _p(new Private)
        {}

        Menu::~Menu()
        {}

        std::shared_ptr<Menu> Menu::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Menu>(new Menu);
            out->_init(context, parent);
            return out;
        }

        void Menu::addAction(const std::shared_ptr<Action>& action)
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                auto button = ListButton::create(context);
                button->setText(action->getText());
                button->setClickedCallback(
                    [action]
                    {
                        action->click();
                    });
                button->setParent(p.layout);
            }
        }

        void Menu::addDivider()
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                auto divider = Divider::create(Orientation::Horizontal, context);
                divider->setParent(p.layout);
            }
        }

        void Menu::clear()
        {
            for (auto child : _children)
            {
                child->setParent(nullptr);
            }
        }
    }
}
