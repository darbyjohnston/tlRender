// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollWidget.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/GridLayout.h>
#include <tlUI/ScrollArea.h>
#include <tlUI/ScrollBar.h>

namespace tl
{
    namespace ui
    {
        struct ScrollWidget::Private
        {
            ScrollType scrollType = ScrollType::Both;
            std::shared_ptr<ScrollArea> scrollArea;
            std::shared_ptr<ScrollBar> horizontalScrollBar;
            std::shared_ptr<ScrollBar> verticalScrollBar;
            std::shared_ptr<GridLayout> layout;
            std::shared_ptr<observer::ValueObserver<math::Vector2i> > scrollSizeObserver;
            std::shared_ptr<observer::ValueObserver<math::Vector2i> > scrollPosObserver;

            struct SizeData
            {
                int border = 0;
            };
            SizeData size;
        };

        void ScrollWidget::_init(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollWidget", context, parent);
            TLRENDER_P();

            p.scrollType = scrollType;

            p.scrollArea = ui::ScrollArea::create(context, scrollType);
            p.scrollArea->setStretch(Stretch::Expanding);

            switch (scrollType)
            {
            case ScrollType::Horizontal:
                p.horizontalScrollBar = ui::ScrollBar::create(Orientation::Horizontal, context);
                break;
            case ScrollType::Vertical:
                p.verticalScrollBar = ui::ScrollBar::create(Orientation::Vertical, context);
                break;
            case ScrollType::Both:
                p.horizontalScrollBar = ui::ScrollBar::create(Orientation::Horizontal, context);
                p.verticalScrollBar = ui::ScrollBar::create(Orientation::Vertical, context);
                break;
            }

            p.layout = GridLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::Border);
            p.layout->setStretch(Stretch::Expanding);
            p.scrollArea->setParent(p.layout);
            p.layout->setGridPos(p.scrollArea, 0, 0);
            if (p.horizontalScrollBar)
            {
                p.horizontalScrollBar->setParent(p.layout);
                p.layout->setGridPos(p.horizontalScrollBar, 1, 0);
            }
            if (p.verticalScrollBar)
            {
                p.verticalScrollBar->setParent(p.layout);
                p.layout->setGridPos(p.verticalScrollBar, 0, 1);
            }

            if (p.horizontalScrollBar)
            {
                p.horizontalScrollBar->setScrollPosCallback(
                    [this](int value)
                    {
                        math::Vector2i scrollPos;
                        scrollPos.x = value;
                        if (_p->verticalScrollBar)
                        {
                            scrollPos.y = _p->verticalScrollBar->getScrollPos();
                        }
                        _p->scrollArea->setScrollPos(scrollPos);
                    });
            }

            if (p.verticalScrollBar)
            {
                p.verticalScrollBar->setScrollPosCallback(
                    [this](int value)
                    {
                        math::Vector2i scrollPos;
                        if (_p->horizontalScrollBar)
                        {
                            scrollPos.x = _p->horizontalScrollBar->getScrollPos();
                        }
                        scrollPos.y = value;
                        _p->scrollArea->setScrollPos(scrollPos);
                    });
            }

            p.scrollSizeObserver = observer::ValueObserver<math::Vector2i>::create(
                p.scrollArea->observeScrollSize(),
                [this](const math::Vector2i& value)
                {
                    if (_p->horizontalScrollBar)
                    {
                        _p->horizontalScrollBar->setScrollSize(value.x);
                    }
                    if (_p->verticalScrollBar)
                    {
                        _p->verticalScrollBar->setScrollSize(value.y);
                    }
                });

            p.scrollPosObserver = observer::ValueObserver<math::Vector2i>::create(
                p.scrollArea->observeScrollPos(),
                [this](const math::Vector2i& value)
                {
                    if (_p->horizontalScrollBar)
                    {
                        _p->horizontalScrollBar->setScrollPos(value.x);
                    }
                    if (_p->verticalScrollBar)
                    {
                        _p->verticalScrollBar->setScrollPos(value.y);
                    }
                });
        }

        ScrollWidget::ScrollWidget() :
            _p(new Private)
        {}

        ScrollWidget::~ScrollWidget()
        {}

        std::shared_ptr<ScrollWidget> ScrollWidget::create(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ScrollWidget>(new ScrollWidget);
            out->_init(context, scrollType, parent);
            return out;
        }

        void ScrollWidget::setWidget(const std::shared_ptr<IWidget>& value)
        {
            value->setParent(_p->scrollArea);
        }

        void ScrollWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            p.layout->setGeometry(value.margin(-p.size.border));
        }

        void ScrollWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

            _sizeHint = p.layout->getSizeHint() + p.size.border * 2;
        }

        void ScrollWidget::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            math::BBox2i g = p.scrollArea->getGeometry().margin(p.size.border);
            event.render->drawMesh(
                border(g, p.size.border),
                math::Vector2i(),
                event.style->getColorRole(ColorRole::Border));

            if (p.horizontalScrollBar)
            {
                g = p.horizontalScrollBar->getGeometry().margin(p.size.border);
                event.render->drawMesh(
                    border(g, p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
            }

            if (p.verticalScrollBar)
            {
                g = p.verticalScrollBar->getGeometry().margin(p.size.border);
                event.render->drawMesh(
                    border(g, p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
            }
        }
    }
}
