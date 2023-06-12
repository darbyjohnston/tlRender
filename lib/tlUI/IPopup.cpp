// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IPopup.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>

namespace tl
{
    namespace ui
    {
        struct IPopup::Private
        {
            math::BBox2i buttonGeometry;
            std::function<void(void)> closeCallback;
            int border = 0;
        };

        void IPopup::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
        }

        IPopup::IPopup() :
            _p(new Private)
        {}

        IPopup::~IPopup()
        {}

        void IPopup::open(
            const std::shared_ptr<EventLoop>& eventLoop,
            const math::BBox2i& buttonGeometry)
        {
            TLRENDER_P();
            p.buttonGeometry = buttonGeometry;
            eventLoop->addWidget(shared_from_this());
        }

        void IPopup::close()
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
        }

        void IPopup::setCloseCallback(const std::function<void(void)>& value)
        {
            _p->closeCallback = value;
        }

        void IPopup::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const auto& children = getChildren();
            if (!children.empty())
            {
                math::Vector2i sizeHint = children.front()->getSizeHint();
                sizeHint.x += p.border * 2;
                sizeHint.y += p.border * 2;
                std::vector<math::BBox2i> bboxes;
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
                for (auto& bbox : bboxes)
                {
                    bbox = bbox.intersect(value);
                }
                std::stable_sort(
                    bboxes.begin(),
                    bboxes.end(),
                    [](const math::BBox2i& a, const math::BBox2i& b)
                    {
                        return a.getArea() > b.getArea();
                    });
                children.front()->setGeometry(bboxes.front().margin(-p.border));
            }
        }

        void IPopup::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            p.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
        }

        void IPopup::drawEvent(
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
                event.render->drawRect(g2, event.style->getColorRole(ColorRole::Button));
            }
        }

        void IPopup::enterEvent()
        {}

        void IPopup::leaveEvent()
        {}

        void IPopup::mouseMoveEvent(MouseMoveEvent& event)
        {
            event.accept = true;
        }

        void IPopup::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            close();
        }

        void IPopup::mouseReleaseEvent(MouseClickEvent& event)
        {
            event.accept = true;
        }

        void IPopup::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            switch (event.key)
            {
            case Key::Tab:
                break;
            case Key::Escape:
                event.accept = true;
                close();
                break;
            default:
                event.accept = true;
                break;
            }
        }

        void IPopup::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
