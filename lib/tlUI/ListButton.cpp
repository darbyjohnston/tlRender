// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ListButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ListButton::Private
        {
            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > glyphs;
            };
            DrawData draw;
        };

        void ListButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::ListButton", context, parent);
            setButtonRole(ColorRole::None);
        }

        ListButton::ListButton() :
            _p(new Private)
        {}

        ListButton::~ListButton()
        {}

        std::shared_ptr<ListButton> ListButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ListButton>(new ListButton);
            out->_init(context, parent);
            return out;
        }

        bool ListButton::acceptsKeyFocus() const
        {
            return true;
        }

        void ListButton::sizeHintEvent(const SizeHintEvent& event)
        {
            IButton::sizeHintEvent(event);
            TLRENDER_P();
            
            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

            _sizeHint = math::Vector2i();
            if (!_text.empty())
            {
                p.size.fontMetrics = event.getFontMetrics(_fontRole);
                const auto fontInfo = event.style->getFontRole(_fontRole, event.displayScale);
                p.size.textSize = event.fontSystem->measure(_text, fontInfo);

                _sizeHint.x = event.fontSystem->measure(_text, fontInfo).x;
                _sizeHint.y = p.size.fontMetrics.lineHeight;
            }
            if (_iconImage)
            {
                _sizeHint.x += _iconImage->getWidth();
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(_iconImage->getHeight()));
                if (!_text.empty())
                {
                    _sizeHint.x += p.size.spacing;
                }
            }
            _sizeHint.x += p.size.margin * 2 + p.size.border * 2;
            _sizeHint.y += p.size.margin * 2 + p.size.border * 2;
        }

        void ListButton::clipEvent(bool clipped, const ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipped, event);
            TLRENDER_P();
            if (changed)
            {
                if (clipped)
                {
                    p.draw.glyphs.clear();
                }
                else if (!_text.empty())
                {
                    const auto fontInfo = event.style->getFontRole(_fontRole, event.displayScale);
                    p.draw.glyphs = event.fontSystem->getGlyphs(_text, fontInfo);
                }
            }
        }

        void ListButton::drawEvent(const DrawEvent& event)
        {
            IButton::drawEvent(event);
            TLRENDER_P();

            const math::BBox2i g = _geometry;

            if (event.focusWidget == shared_from_this())
            {
                event.render->drawMesh(
                    border(g, p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }

            const math::BBox2i g2 = g.margin(-p.size.border);
            const ColorRole colorRole = _checked ?
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

            int x = g2.x() + p.size.margin;
            if (_iconImage)
            {
                const auto iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  math::BBox2i(
                      x,
                      g2.y() + g2.h() / 2 - iconSize.h / 2,
                      iconSize.w,
                      iconSize.h));
                x += iconSize.w + p.size.spacing;
            }
            
            if (!_text.empty())
            {
                const math::Vector2i pos(
                    x,
                    g2.y() + g2.h() / 2 - p.size.textSize.y / 2 +
                    p.size.fontMetrics.ascender);
                event.render->drawText(
                    p.draw.glyphs,
                    pos,
                    event.style->getColorRole(ColorRole::Text));
            }
        }

        void ListButton::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            switch (event.key)
            {
            case Key::Space:
            case Key::Enter:
                event.accept = true;
                _click();
                break;
            }
        }

        void ListButton::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
