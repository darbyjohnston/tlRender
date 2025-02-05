// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/ColorSwatch.h>

#include <tlUI/ColorPopup.h>
#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ColorSwatch::Private
        {
            dtk::Color4F color;
            bool editable = false;
            std::function<void(const dtk::Color4F&)> callback;
            SizeRole sizeRole = SizeRole::Swatch;
            std::shared_ptr<ColorPopup> popup;

            struct SizeData
            {
                bool sizeInit = true;
                int size = 0;
                int border = 0;
            };
            SizeData size;
        };

        void ColorSwatch::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ColorSwatch", context, parent);
        }

        ColorSwatch::ColorSwatch() :
            _p(new Private)
        {}

        ColorSwatch::~ColorSwatch()
        {}

        std::shared_ptr<ColorSwatch> ColorSwatch::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ColorSwatch>(new ColorSwatch);
            out->_init(context, parent);
            return out;
        }

        const dtk::Color4F& ColorSwatch::getColor() const
        {
            return _p->color;
        }

        void ColorSwatch::setColor(const dtk::Color4F& value)
        {
            TLRENDER_P();
            if (value == p.color)
                return;
            p.color = value;
            _updates |= Update::Draw;
        }

        void ColorSwatch::setEditable(bool value)
        {
            TLRENDER_P();
            if (value == p.editable)
                return;
            p.editable = value;
            _setMouseHover(p.editable);
            _setMousePress(p.editable);
        }

        void ColorSwatch::setCallback(const std::function<void(const dtk::Color4F&)>& value)
        {
            _p->callback = value;
        }

        void ColorSwatch::setSizeRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.sizeRole)
                return;
            p.sizeRole = value;
            p.size.sizeInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ColorSwatch::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.size = event.style->getSizeRole(p.sizeRole, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            p.size.sizeInit = false;

            _sizeHint.w = _sizeHint.h = p.size.size;
        }

        void ColorSwatch::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();
            const dtk::Box2I& g = _geometry;
            event.render->drawMesh(
                border(g, p.size.border),
                event.style->getColorRole(ColorRole::Border));
            event.render->drawRect(
                dtk::margin(g, -p.size.border),
                p.color);
        }

        void ColorSwatch::mousePressEvent(MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            if (p.editable)
            {
                _showPopup();
            }
        }

        void ColorSwatch::_showPopup()
        {
            TLRENDER_P();
            if (auto context = _context.lock())
            {
                if (!p.popup)
                {
                    p.popup = ColorPopup::create(p.color, context);
                    p.popup->open(getWindow(), getGeometry());
                    p.popup->setCallback(
                        [this](const dtk::Color4F& value)
                        {
                            _p->color = value;
                            _updates |= Update::Draw;
                            if (_p->callback)
                            {
                                _p->callback(value);
                            }
                        });
                    p.popup->setCloseCallback(
                        [this]
                        {
                            _p->popup.reset();
                        });
                }
                else
                {
                    p.popup->close();
                    p.popup.reset();
                }
            }
        }
    }
}
