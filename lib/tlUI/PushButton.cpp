// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/PushButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct PushButton::Private
        {
        };

        void PushButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::PushButton", context, parent);
        }

        PushButton::PushButton() :
            _p(new Private)
        {}

        PushButton::~PushButton()
        {}

        std::shared_ptr<PushButton> PushButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PushButton>(new PushButton);
            out->_init(context, parent);
            return out;
        }

        void PushButton::sizeEvent(const SizeEvent& event)
        {
            IButton::sizeEvent(event);
            TLRENDER_P();

            const int m = event.style->getSizeRole(SizeRole::Margin) * event.contentScale;
            const int m2 = event.style->getSizeRole(SizeRole::MarginSmall) * event.contentScale;

            _sizeHint.x = 0;
            _sizeHint.y = 0;
            if (!_text.empty())
            {
                imaging::FontInfo fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
                _sizeHint.x = event.fontSystem->measure(_text, fontInfo).x + m2 * 2;
                _sizeHint.y = fontMetrics.lineHeight;
            }
            if (_iconImage)
            {
                _sizeHint.x += _iconImage->getWidth();
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(_iconImage->getHeight()));
            }
            _sizeHint.x += m * 2 * 2;
            _sizeHint.y += m2 * 2;
        }

        void PushButton::drawEvent(const DrawEvent& event)
        {
            IButton::drawEvent(event);
            TLRENDER_P();

            const int m = event.style->getSizeRole(SizeRole::Margin) * event.contentScale;
            const int m2 = event.style->getSizeRole(SizeRole::MarginSmall) * event.contentScale;
            const int b = event.style->getSizeRole(SizeRole::Border) * event.contentScale;
            math::BBox2i g = _geometry;

            event.render->drawMesh(
                border(g, b, m / 2),
                event.style->getColorRole(ColorRole::Border));

            math::BBox2i g2 = g.margin(-b);
            const auto mesh = rect(g2, m / 2);
            const ColorRole colorRole = _checked->get() ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawMesh(
                    mesh,
                    event.style->getColorRole(colorRole));
            }

            if (_pressed && _geometry.contains(_cursorPos))
            {
                event.render->drawMesh(
                    mesh,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_inside)
            {
                event.render->drawMesh(
                    mesh,
                    event.style->getColorRole(ColorRole::Hover));
            }

            int x = g.x() + m * 2;
            if (_iconImage)
            {
                const auto iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  math::BBox2i(x, g.y() + m2, iconSize.w, iconSize.h));
                x += iconSize.w;
            }
            
            if (!_text.empty())
            {
                imaging::FontInfo fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
                math::Vector2i textSize = event.fontSystem->measure(_text, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_text, fontInfo),
                    math::Vector2i(
                        x + (g.max.x - m * 2 - x) / 2 - textSize.x / 2,
                        g.y() + g.h() / 2 - textSize.y / 2 + fontMetrics.ascender),
                    event.style->getColorRole(ColorRole::Text));
            }
        }
    }
}
