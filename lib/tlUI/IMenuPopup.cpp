// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IMenuPopup.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>

namespace tl
{
    namespace ui
    {
        struct IMenuPopup::Private
        {
            MenuPopupStyle popupStyle = MenuPopupStyle::Menu;
            ColorRole popupRole = ColorRole::Window;
            math::BBox2i buttonGeometry;
            bool open = false;
            std::function<void(void)> closeCallback;
            int border = 0;
        };

        void IMenuPopup::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
        }

        IMenuPopup::IMenuPopup() :
            _p(new Private)
        {}

        IMenuPopup::~IMenuPopup()
        {}

        void IMenuPopup::open(
            const std::shared_ptr<EventLoop>& eventLoop,
            const math::BBox2i& buttonGeometry)
        {
            TLRENDER_P();
            p.buttonGeometry = buttonGeometry;
            p.open = true;
            eventLoop->addWidget(shared_from_this());
        }

        bool IMenuPopup::isOpen() const
        {
            return _p->open;
        }

        void IMenuPopup::close()
        {
            TLRENDER_P();
            if (auto eventLoop = getEventLoop().lock())
            {
                eventLoop->removeWidget(shared_from_this());
            }
            if (p.closeCallback)
            {
                p.closeCallback();
            }
            p.open = false;
        }

        void IMenuPopup::setCloseCallback(const std::function<void(void)>& value)
        {
            _p->closeCallback = value;
        }

        void IMenuPopup::setPopupStyle(MenuPopupStyle value)
        {
            TLRENDER_P();
            p.popupStyle = value;
        }

        void IMenuPopup::setPopupRole(ColorRole value)
        {
            TLRENDER_P();
            if (value == p.popupRole)
                return;
            p.popupRole = value;
            _updates |= Update::Draw;
        }

        void IMenuPopup::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const auto& children = getChildren();
            if (!children.empty())
            {
                math::Vector2i sizeHint = children.front()->getSizeHint();
                sizeHint.x += p.border * 2;
                sizeHint.y += p.border * 2;
                std::list<math::BBox2i> bboxes;
                switch (p.popupStyle)
                {
                case MenuPopupStyle::Menu:
                    bboxes.push_back(math::BBox2i(
                        p.buttonGeometry.min.x,
                        p.buttonGeometry.max.y,
                        sizeHint.x,
                        sizeHint.y));
                    bboxes.push_back(math::BBox2i(
                        p.buttonGeometry.max.x - sizeHint.x + 1,
                        p.buttonGeometry.max.y,
                        sizeHint.x,
                        sizeHint.y));
                    bboxes.push_back(math::BBox2i(
                        p.buttonGeometry.min.x,
                        p.buttonGeometry.min.y - sizeHint.y + 1,
                        sizeHint.x,
                        sizeHint.y));
                    bboxes.push_back(math::BBox2i(
                        p.buttonGeometry.max.x - sizeHint.x + 1,
                        p.buttonGeometry.min.y - sizeHint.y + 1,
                        sizeHint.x,
                        sizeHint.y));
                    break;
                case MenuPopupStyle::SubMenu:
                    bboxes.push_back(math::BBox2i(
                        p.buttonGeometry.max.x + 1,
                        p.buttonGeometry.min.y,
                        sizeHint.x,
                        sizeHint.y));
                    bboxes.push_back(math::BBox2i(
                        p.buttonGeometry.min.x - sizeHint.x + 1,
                        p.buttonGeometry.min.y,
                        sizeHint.x,
                        sizeHint.y));
                    break;
                }
                struct Intersect
                {
                    math::BBox2i original;
                    math::BBox2i intersected;
                };
                std::vector<Intersect> intersect;
                for (const auto& bbox : bboxes)
                {
                    intersect.push_back({ bbox, bbox.intersect(value) });
                }
                std::stable_sort(
                    intersect.begin(),
                    intersect.end(),
                    [](const Intersect& a, const Intersect& b)
                    {
                        return a.intersected.getArea() > b.intersected.getArea();
                    });
                children.front()->setGeometry(intersect.front().original.margin(-p.border));
            }
        }

        void IMenuPopup::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            p.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
        }

        void IMenuPopup::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();
            //event.render->drawRect(
            //    _geometry,
            //    imaging::Color4f(0.F, 0.F, 0.F, .2F));
            const auto& children = getChildren();
            if (!children.empty())
            {
                const math::BBox2i g = children.front()->getGeometry().margin(p.border);
                event.render->drawMesh(
                    border(g, p.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));

                const math::BBox2i g2 = g.margin(-p.border);
                event.render->drawRect(g2, event.style->getColorRole(p.popupRole));
            }
        }
    }
}
