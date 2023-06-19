// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IDialog.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>

namespace tl
{
    namespace ui
    {
        struct IDialog::Private
        {
            bool open = false;
            std::function<void(void)> closeCallback;
            
            struct SizeData
            {
                int margin = 0;
                int margin2 = 0;
                int border = 0;
            };
            SizeData size;
        };

        void IDialog::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
        }

        IDialog::IDialog() :
            _p(new Private)
        {}

        IDialog::~IDialog()
        {}

        void IDialog::open(const std::shared_ptr<EventLoop>& eventLoop)
        {
            TLRENDER_P();
            p.open = true;
            eventLoop->addWidget(shared_from_this());
        }

        bool IDialog::isOpen() const
        {
            return _p->open;
        }

        void IDialog::close()
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

        void IDialog::setCloseCallback(const std::function<void(void)>& value)
        {
            _p->closeCallback = value;
        }

        void IDialog::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const auto& children = getChildren();
            children.front()->setGeometry(value.margin(-p.size.margin));
        }

        void IDialog::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            p.size.margin = event.style->getSizeRole(SizeRole::MarginDialog, event.displayScale);
            p.size.margin2 = event.style->getSizeRole(SizeRole::Margin, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
        }

        void IDialog::drawEvent(
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
                const math::BBox2i g = children.front()->getGeometry();
                const math::BBox2i g2(
                    g.min.x - p.size.margin2,
                    g.min.y,
                    g.w() + p.size.margin2 * 2,
                    g.h() + p.size.margin2);
                event.render->drawColorMesh(
                    shadow(g2, p.size.margin2),
                    math::Vector2i(),
                    imaging::Color4f(1.F, 1.F, 1.F));

                event.render->drawMesh(
                    border(g.margin(p.size.border), p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
                
                event.render->drawRect(g, event.style->getColorRole(ColorRole::Window));
            }
        }
    }
}