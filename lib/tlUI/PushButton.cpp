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
            struct SizeData
            {
                int margin = 0;
                int margin2 = 0;
                int spacing = 0;
                int border = 0;
                imaging::FontInfo fontInfo = imaging::FontInfo("", 0);
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

        void PushButton::setText(const std::string& value)
        {
            const bool changed = value != _text;
            IButton::setText(value);
            TLRENDER_P();
            if (changed)
            {
                p.draw.glyphs.clear();
            }
        }

        void PushButton::setFontRole(FontRole value)
        {
            const bool changed = value != _fontRole;
            IButton::setFontRole(value);
            TLRENDER_P();
            if (changed)
            {
                p.draw.glyphs.clear();
            }
        }

        bool PushButton::acceptsKeyFocus() const
        {
            return true;
        }

        void PushButton::sizeHintEvent(const SizeHintEvent& event)
        {
            IButton::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::Margin, event.displayScale);
            p.size.margin2 = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

            _sizeHint = math::Vector2i();
            if (!_text.empty())
            {
                p.size.fontMetrics = event.getFontMetrics(_fontRole);
                auto fontInfo = event.style->getFontRole(_fontRole, event.displayScale);
                p.size.fontInfo = fontInfo;
                p.size.textSize = event.fontSystem->getSize(_text, fontInfo);

                _sizeHint.x = p.size.textSize.x + p.size.margin2 * 2;
                _sizeHint.y = p.size.fontMetrics.lineHeight;
            }
            if (_iconImage)
            {
                _sizeHint.x += _iconImage->getWidth();
                if (!_text.empty())
                {
                    _sizeHint.x += p.size.spacing;
                }
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(_iconImage->getHeight()));
            }
            _sizeHint.x +=
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.y +=
                p.size.margin2 * 2 +
                p.size.border * 4;
        }

        void PushButton::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            IWidget::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void PushButton::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;
            const bool enabled = isEnabled();

            // Draw the border.
            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::KeyFocus));
            } 
            else
            {
                event.render->drawMesh(
                    border(g.margin(-p.size.border), p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
            }

            // Draw the background and checked state.
            const math::BBox2i g2 = g.margin(-p.size.border * 2);
            const auto mesh = rect(g2);
            const ColorRole colorRole = _checked ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawMesh(
                    mesh,
                    math::Vector2i(),
                    event.style->getColorRole(colorRole));
            }

            // Draw the pressed and hover states.
            if (_pressed && _geometry.contains(_cursorPos))
            {
                event.render->drawMesh(
                    mesh,
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_inside)
            {
                event.render->drawMesh(
                    mesh,
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Hover));
            }

            // Draw the icon.
            int x = g2.x() + p.size.margin;
            if (_iconImage)
            {
                const imaging::Size& iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  math::BBox2i(
                      x,
                      g2.y() + g2.h() / 2 - iconSize.h / 2,
                      iconSize.w,
                      iconSize.h),
                  event.style->getColorRole(enabled ?
                      ColorRole::Text :
                      ColorRole::TextDisabled));
                x += iconSize.w + p.size.spacing;
            }
            
            // Draw the text.
            if (!_text.empty())
            {
                if (p.draw.glyphs.empty())
                {
                    p.draw.glyphs = event.fontSystem->getGlyphs(_text, p.size.fontInfo);
                }
                const math::Vector2i pos(
                    x + p.size.margin2,
                    g2.y() + g2.h() / 2 - p.size.textSize.y / 2 +
                    p.size.fontMetrics.ascender);
                event.render->drawText(
                    p.draw.glyphs,
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
            }
        }

        void PushButton::keyPressEvent(KeyEvent& event)
        {
            switch (event.key)
            {
            case Key::Space:
            case Key::Enter:
                event.accept = true;
                _click();
                break;
            case Key::Escape:
                if (hasKeyFocus())
                {
                    event.accept = true;
                    releaseKeyFocus();
                }
                break;
            default: break;
            }
        }

        void PushButton::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
