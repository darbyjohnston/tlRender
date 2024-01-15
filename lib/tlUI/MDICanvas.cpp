// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/MDICanvas.h>

#include <tlUI/MDIWidget.h>

namespace tl
{
    namespace ui
    {
        struct MDICanvas::Private
        {
            std::list<std::weak_ptr<IWidget> > newWidgets;

            struct SizeData
            {
                bool sizeInit = true;
                int size = 0;
                int spacing = 0;
            };
            SizeData size;

            struct MouseData
            {
                std::shared_ptr<IWidget> widget;
                math::Box2i geom;
            };
            MouseData mouse;
        };

        void MDICanvas::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::MDICanvas", context, parent);
            setBackgroundRole(ColorRole::Base);
            _setMouseHover(true);
            _setMousePress(true);
        }

        MDICanvas::MDICanvas() :
            _p(new Private)
        {}

        MDICanvas::~MDICanvas()
        {}

        std::shared_ptr<MDICanvas> MDICanvas::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MDICanvas>(new MDICanvas);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<MDIWidget> MDICanvas::addWidget(
            const std::string& title,
            const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            std::shared_ptr<MDIWidget> out;
            if (auto context = _context.lock())
            {
                auto out = MDIWidget::create(title, context, shared_from_this());
                out->setWidget(value);
                out->setPressCallback(
                    [this, out](bool value)
                    {
                        if (value)
                        {
                            moveToFront(out);
                            _p->mouse.widget = out;
                            _p->mouse.geom = out->getGeometry();
                        }
                        else
                        {
                            _p->mouse.widget.reset();
                            _p->mouse.geom = math::Box2i();
                        }
                    });
                out->setMoveCallback(
                    [this](const math::Vector2i& move)
                    {
                        if (auto widget = _p->mouse.widget)
                        {
                            const math::Box2i& g = getGeometry();
                            widget->setGeometry(math::Box2i(
                                math::clamp(
                                    _p->mouse.geom.min.x + move.x,
                                    g.min.x,
                                    g.max.x + 1 - _p->mouse.geom.w()),
                                math::clamp(
                                    _p->mouse.geom.min.y + move.y,
                                    g.min.y,
                                    g.max.y + 1 - _p->mouse.geom.h()),
                                _p->mouse.geom.w(),
                                _p->mouse.geom.h()));
                        }
                    });
                out->setResizeCallback(
                    [this](MDIResize value, const math::Vector2i& move)
                    {
                        if (auto widget = _p->mouse.widget)
                        {
                            const math::Size2i sizeHint = widget->getSizeHint();
                            const math::Box2i& g = getGeometry();
                            math::Box2i g2 = _p->mouse.geom;
                            switch (value)
                            {
                            case MDIResize::North:
                                g2.min.y = math::clamp(
                                    g2.min.y + move.y,
                                    g.min.y,
                                    _p->mouse.geom.max.y - sizeHint.h);
                                break;
                            case MDIResize::NorthEast:
                                g2.min.y = math::clamp(
                                    g2.min.y + move.y,
                                    g.min.y,
                                    _p->mouse.geom.max.y - sizeHint.h);
                                g2.max.x = std::min(g2.max.x + move.x, g.max.x);
                                break;
                            case MDIResize::East:
                                g2.max.x = std::min(g2.max.x + move.x, g.max.x);
                                break;
                            case MDIResize::SouthEast:
                                g2.max.x = std::min(g2.max.x + move.x, g.max.x);
                                g2.max.y = std::min(g2.max.y + move.y, g.max.y);
                                break;
                            case MDIResize::South:
                                g2.max.y = std::min(g2.max.y + move.y, g.max.y);
                                break;
                            case MDIResize::SouthWest:
                                g2.min.x = math::clamp(
                                    g2.min.x + move.x,
                                    g.min.x,
                                    _p->mouse.geom.max.x - sizeHint.w);
                                g2.max.y = std::min(g2.max.y + move.y, g.max.y);
                                break;
                            case MDIResize::West:
                                g2.min.x = math::clamp(
                                    g2.min.x + move.x,
                                    g.min.x,
                                    _p->mouse.geom.max.x - sizeHint.w);
                                break;
                            case MDIResize::NorthWest:
                                g2.min.x = math::clamp(
                                    g2.min.x + move.x,
                                    g.min.x,
                                    _p->mouse.geom.max.x - sizeHint.w);
                                g2.min.y = math::clamp(
                                    g2.min.y + move.y,
                                    g.min.y,
                                    _p->mouse.geom.max.y - sizeHint.h);
                                break;
                            default: break;
                            }
                            widget->setGeometry(g2);
                        }
                    });
                p.newWidgets.push_back(out);
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
            return out;
        }

        void MDICanvas::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::Box2i& g = _geometry;
            math::Vector2i pos = g.min + p.size.spacing;
            while (!p.newWidgets.empty())
            {
                if (auto widget = p.newWidgets.front().lock())
                {
                    const math::Size2i& sizeHint = widget->getSizeHint();
                    widget->setGeometry(math::Box2i(
                        pos.x,
                        pos.y,
                        sizeHint.w,
                        sizeHint.h));
                    pos = pos + p.size.spacing;
                }
                p.newWidgets.pop_front();
            }
            for (const auto& child : _children)
            {
                const math::Size2i& sizeHint = child->getSizeHint();
                const math::Box2i& g2 = child->getGeometry();
                child->setGeometry(math::Box2i(
                    g2.min.x,
                    g2.min.y,
                    std::max(g2.w(), sizeHint.w),
                    std::max(g2.h(), sizeHint.h)));
            }
        }

        void MDICanvas::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.size = event.style->getSizeRole(SizeRole::ScrollArea, _displayScale);
                p.size.spacing = event.style->getSizeRole(SizeRole::SpacingLarge, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint.w = _sizeHint.h = p.size.size;
        }

        void MDICanvas::mouseMoveEvent(MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
        }

        void MDICanvas::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
        }

        void MDICanvas::mouseReleaseEvent(MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
        }
    }
}
