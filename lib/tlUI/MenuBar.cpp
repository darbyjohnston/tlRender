// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/MenuBar.h>

namespace tl
{
    namespace ui
    {
        struct MenuBar::Private
        {
            std::list<std::shared_ptr<MenuItem> > menuItems;
            
            struct SizeData
            {
                int margin = 0;
                int margin2 = 0;
                int spacing = 0;
                int border = 0;
                imaging::FontInfo fontInfo = imaging::FontInfo("", 0);
                imaging::FontMetrics fontMetrics;
            };
            SizeData size;
        };

        void MenuBar::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::MenuBar", context, parent);
        }

        MenuBar::MenuBar() :
            _p(new Private)
        {}

        MenuBar::~MenuBar()
        {}

        std::shared_ptr<MenuBar> MenuBar::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MenuBar>(new MenuBar);
            out->_init(context, parent);
            return out;
        }
        
        void MenuBar::addMenuItem(const std::shared_ptr<MenuItem>& item)
        {
            TLRENDER_P();
            p.menuItems.push_back(item);
            _updates |= Update::Size;
            _updates |= Update::Draw;            
        }

        void MenuBar::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::Margin, event.displayScale);
            p.size.margin2 = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            
            _sizeHint.x = _sizeHint.y = 0;
        }
    }
}
