// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TabBar.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct TabBar::Private
        {
            std::vector<std::string> tabs;
            int currentTab = -1;
            std::shared_ptr<ButtonGroup> buttonGroup;
            std::vector<std::shared_ptr<ListButton> > buttons;
            std::shared_ptr<HorizontalLayout> layout;
            std::function<void(int)> callback;
        };

        void TabBar::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TabBar", context, parent);
            TLRENDER_P();

            p.buttonGroup = ButtonGroup::create(ButtonGroupType::Radio, context);

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);

            p.buttonGroup->setCheckedCallback(
                [this](int index, bool value)
                {
                    if (value && _p->callback)
                    {
                        _p->callback(index);
                    }
                });
        }

        TabBar::TabBar() :
            _p(new Private)
        {}

        TabBar::~TabBar()
        {}

        std::shared_ptr<TabBar> TabBar::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TabBar>(new TabBar);
            out->_init(context, parent);
            return out;
        }

        void TabBar::setTabs(const std::vector<std::string>& value)
        {
            TLRENDER_P();
            if (value == p.tabs)
                return;
            p.tabs = value;
            p.currentTab = math::clamp(p.currentTab, 0, static_cast<int>(p.tabs.size()) - 1);
            _widgetUpdate();
        }

        void TabBar::addTab(const std::string& value)
        {
            TLRENDER_P();
            p.tabs.push_back(value);
            if (p.currentTab < 0)
            {
                p.currentTab = 0;
            }
            _widgetUpdate();
        }

        void TabBar::clearTabs()
        {
            TLRENDER_P();
            p.tabs.clear();
            p.currentTab = -1;
            _widgetUpdate();
        }

        int TabBar::getCurrentTab() const
        {
            return _p->currentTab;
        }

        void TabBar::setCurrentTab(int value)
        {
            TLRENDER_P();
            const int tmp = math::clamp(value, 0, static_cast<int>(p.tabs.size()) - 1);
            if (tmp == _p->currentTab)
                return;
            _p->currentTab = tmp;
            _p->buttonGroup->setChecked(_p->currentTab, true);
        }

        void TabBar::setCallback(const std::function<void(int)>& value)
        {
            _p->callback = value;
        }

        void TabBar::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void TabBar::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void TabBar::_widgetUpdate()
        {
            TLRENDER_P();
            p.buttonGroup->clearButtons();
            p.buttons.clear();
            auto children = p.layout->getChildren();
            for (const auto& child : children)
            {
                child->setParent(nullptr);
            }
            if (auto context = _context.lock())
            {
                for (const auto& tab : p.tabs)
                {
                    auto button = ListButton::create(tab, context, p.layout);
                    button->setLabelMarginRole(SizeRole::MarginSmall);
                    button->setCheckedRole(ColorRole::Button);
                    p.buttonGroup->addButton(button);
                    p.buttons.push_back(button);
                }
            }
            p.buttonGroup->setChecked(p.currentTab, true);
        }
    }
}
