// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ToolButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ToolButton::Private
        {
            struct Size
            {
                math::Vector2i textSize;
                int margin = 0;
                int border = 0;
            };
            Size size;
        };

        void ToolButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::ToolButton", context, parent);
        }

        ToolButton::ToolButton() :
            _p(new Private)
        {}

        ToolButton::~ToolButton()
        {}

        std::shared_ptr<ToolButton> ToolButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolButton>(new ToolButton);
            out->_init(context, parent);
            return out;
        }

        void ToolButton::sizeEvent(const SizeEvent& event)
        {
            IButton::sizeEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginInside) * event.contentScale;
            p.size.border = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            _sizeHint = math::Vector2i();
            if (!_text.empty())
            {
                const auto fontInfo = event.getFontInfo(_fontRole);
                const auto fontMetrics = event.getFontMetrics(_fontRole);
                p.size.textSize = event.fontSystem->measure(_text, fontInfo);

                _sizeHint.x = event.fontSystem->measure(_text, fontInfo).x + p.size.margin * 2;
                _sizeHint.y = fontMetrics.lineHeight;
            }
            if (_iconImage)
            {
                _sizeHint.x += _iconImage->getWidth();
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(_iconImage->getHeight()));
            }
            _sizeHint.x += p.size.margin * 2;
            _sizeHint.y += p.size.margin * 2;
        }

        void ToolButton::drawEvent(const DrawEvent& event)
        {
            IButton::drawEvent(event);
            TLRENDER_P();

            math::BBox2i g = _geometry;

            event.render->drawMesh(
                border(g, p.size.border),
                math::Vector2i(),
                event.style->getColorRole(ColorRole::Border));

            math::BBox2i g2 = g.margin(-p.size.border);
            const ColorRole colorRole = _checked->get() ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(colorRole));
            }

            if (_pressed && _geometry.contains(_cursorPos))
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_inside)
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Hover));
            }

            int x = g.x() + p.size.margin;
            if (_iconImage)
            {
                const auto iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  math::BBox2i(x, g.y() + p.size.margin, iconSize.w, iconSize.h));
                x += _iconImage->getWidth();
            }
            
            if (!_text.empty())
            {
                const auto fontInfo = event.getFontInfo(_fontRole);
                const auto fontMetrics = event.getFontMetrics(_fontRole);

                math::Vector2i pos(
                    x + p.size.margin,
                    g.y() + g.h() / 2 - p.size.textSize.y / 2 +
                        fontMetrics.ascender);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_text, fontInfo),
                    pos,
                    event.style->getColorRole(ColorRole::Text));
            }
        }
    }
}
