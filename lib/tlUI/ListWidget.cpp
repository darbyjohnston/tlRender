// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/ListWidget.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

#include <dtk/core/String.h>

namespace tl
{
    namespace ui
    {
        struct ListWidget::Private
        {
            std::vector<std::string> items;
            int currentItem = -1;
            std::string search;

            std::shared_ptr<ButtonGroup> buttonGroup;
            std::shared_ptr<VerticalLayout> layout;
            std::shared_ptr<ScrollWidget> scrollWidget;

            std::function<void(int)> callback;
        };

        void ListWidget::_init(
            ButtonGroupType type,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ListWidget", context, parent);
            TLRENDER_P();

            p.buttonGroup = ButtonGroup::create(type, context);

            p.layout = VerticalLayout::create(context);
            p.layout->setSpacingRole(SizeRole::None);

            p.scrollWidget = ScrollWidget::create(context, ScrollType::Both, shared_from_this());
            p.scrollWidget->setWidget(p.layout);

            p.buttonGroup->setCheckedCallback(
                [this](int index, bool value)
                {
                    if (value && _p->callback)
                    {
                        _p->callback(index);
                    }
                });
        }

        ListWidget::ListWidget() :
            _p(new Private)
        {}

        ListWidget::~ListWidget()
        {}

        std::shared_ptr<ListWidget> ListWidget::create(
            ButtonGroupType type,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ListWidget>(new ListWidget);
            out->_init(type, context, parent);
            return out;
        }

        void ListWidget::setItems(const std::vector<std::string>& value)
        {
            TLRENDER_P();
            if (value == p.items)
                return;
            p.items = value;
            p.currentItem = math::clamp(p.currentItem, 0, static_cast<int>(p.items.size()) - 1);
            _widgetUpdate();
            _searchUpdate();
        }

        void ListWidget::setCurrentItem(int value)
        {
            TLRENDER_P();
            if (value == p.currentItem)
                return;
            p.currentItem = value;
            p.buttonGroup->setChecked(p.currentItem, true);
        }

        void ListWidget::setCallback(const std::function<void(int)>& value)
        {
            _p->callback = value;
        }

        void ListWidget::setSearch(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.search)
                return;
            p.search = value;
            _searchUpdate();
        }

        void ListWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->scrollWidget->setGeometry(value);
        }

        void ListWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->scrollWidget->getSizeHint();
        }

        void ListWidget::_widgetUpdate()
        {
            TLRENDER_P();
            p.buttonGroup->clearButtons();
            auto children = p.layout->getChildren();
            for (const auto& child : children)
            {
                child->setParent(nullptr);
            }
            if (auto context = _context.lock())
            {
                for (const auto& item : p.items)
                {
                    auto button = ListButton::create(item, context, p.layout);
                    p.buttonGroup->addButton(button);
                }
            }
            p.buttonGroup->setChecked(p.currentItem, true);
        }

        void ListWidget::_searchUpdate()
        {
            TLRENDER_P();
            size_t i = 0;
            for (const auto& child : p.layout->getChildren())
            {
                if (i < p.items.size())
                {
                    child->setVisible(dtk::contains(
                        p.items[i],
                        p.search,
                        dtk::CaseCompare::Insensitive));
                }
                ++i;
            }
        }
    }
}
