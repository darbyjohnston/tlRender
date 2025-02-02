// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/MenuBar.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct MenuBar::Private
        {
            std::list<std::shared_ptr<Menu> > menus;
            std::list<std::shared_ptr<ListButton> > buttons;
            std::shared_ptr<HorizontalLayout> layout;
        };

        void MenuBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::MenuBar", context, parent);
            TLRENDER_P();
            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
        }

        MenuBar::MenuBar() :
            _p(new Private)
        {}

        MenuBar::~MenuBar()
        {}

        std::shared_ptr<MenuBar> MenuBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MenuBar>(new MenuBar);
            out->_init(context, parent);
            return out;
        }
        
        void MenuBar::addMenu(
            const std::string& text,
            const std::shared_ptr<Menu>& menu)
        {
            TLRENDER_P();
            p.menus.push_back(menu);
            if (auto context = _context.lock())
            {
                auto button = ListButton::create(context);
                button->setText(text);
                p.buttons.push_back(button);
                button->setParent(p.layout);
                auto menuWeak = std::weak_ptr<Menu>(menu);
                button->setHoveredCallback(
                    [this, menuWeak, button](bool value)
                    {
                        if (value)
                        {
                            if (auto menu = menuWeak.lock())
                            {
                                std::shared_ptr<Menu> openMenu;
                                for (auto& i : _p->menus)
                                {
                                    if (i->isOpen())
                                    {
                                        openMenu = i;
                                        break;
                                    }
                                }
                                if (openMenu && menu != openMenu)
                                {
                                    openMenu->close();
                                    button->takeKeyFocus();
                                    menu->open(getWindow(), button->getGeometry());
                                }
                            }
                        }
                    });
                button->setPressedCallback(
                    [this, menuWeak, button]
                    {
                        if (auto menu = menuWeak.lock())
                        {
                            if (!menu->isOpen())
                            {
                                menu->open(getWindow(), button->getGeometry());
                            }
                            else
                            {
                                menu->close();
                            }
                        }
                    });
                menu->setCloseCallback(
                    [button]
                    {
                        button->takeKeyFocus();
                    });
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        bool MenuBar::shortcut(Key shortcut, int modifiers)
        {
            TLRENDER_P();
            bool out = false;
            if (shortcut != Key::Unknown)
            {
                for (const auto& menu : p.menus)
                {
                    if (menu->isEnabled())
                    {
                        out |= menu->shortcut(shortcut, modifiers);
                    }
                }
            }
            return out;
        }

        void MenuBar::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            p.layout->setGeometry(value);
        }

        void MenuBar::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            _sizeHint = p.layout->getSizeHint();
        }
    }
}
