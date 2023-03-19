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
        };

        void ToolButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::ToolButton", context, parent);
            setBackgroundRole(ColorRole::Button);
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
            TLRENDER_P();
            
            const int m = event.style->getSizeRole(SizeRole::MarginSmall) * event.contentScale;
            const int s = event.style->getSizeRole(SizeRole::SpacingSmall) * event.contentScale;

            _sizeHint.x = 0;
            _sizeHint.y = 0;
            if (!_text.empty())
            {
                imaging::FontInfo fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
                _sizeHint.x = event.fontSystem->measure(_text, fontInfo).x;
                _sizeHint.y = fontMetrics.lineHeight;
            }
            if (_iconImage)
            {
                _sizeHint.x += _iconImage->getWidth();
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(_iconImage->getHeight()));
                if (!_text.empty())
                {
                    _sizeHint.x += s;
                }
            }
            _sizeHint.x += m * 2;
            _sizeHint.y += m * 2;
        }

        void ToolButton::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const int m = event.style->getSizeRole(SizeRole::MarginSmall) * event.contentScale;
            const int s = event.style->getSizeRole(SizeRole::SpacingSmall) * event.contentScale;
            math::BBox2i g = _geometry;

            if (_checked->get())
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Checked));
            }

            if (_pressed && _geometry.contains(_cursorPos))
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_inside)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Hover));
            }

            int x = g.x() + m;
            if (_iconImage)
            {
                event.render->drawImage(
                  _iconImage,
                  math::BBox2i(x, g.y() + m, _iconImage->getWidth(), _iconImage->getHeight()));
                x += _iconImage->getWidth() + s;
            }
            
            imaging::FontInfo fontInfo = _fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            math::Vector2i textSize = event.fontSystem->measure(_text, fontInfo);
            event.render->drawText(
                event.fontSystem->getGlyphs(_text, fontInfo),
                math::Vector2i(
                    x,
                    g.y() + g.h() / 2 - textSize.y / 2 + fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));
        }
    }
}
