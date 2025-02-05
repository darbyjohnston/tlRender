// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
                bool sizeInit = true;
                int margin = 0;
                int margin2 = 0;
                int spacing = 0;
                int border = 0;

                bool textInit = true;
                dtk::FontInfo fontInfo;
                dtk::FontMetrics fontMetrics;
                dtk::Size2I textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<dtk::Glyph> > glyphs;
            };
            DrawData draw;
        };

        void PushButton::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::PushButton", context, parent);
            setAcceptsKeyFocus(true);
        }

        PushButton::PushButton() :
            _p(new Private)
        {}

        PushButton::~PushButton()
        {}

        std::shared_ptr<PushButton> PushButton::create(
            const std::shared_ptr<dtk::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<PushButton>(new PushButton);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<PushButton> PushButton::create(
            const std::string& text,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PushButton>(new PushButton);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void PushButton::setText(const std::string& value)
        {
            const bool changed = value != _text;
            IButton::setText(value);
            TLRENDER_P();
            if (changed)
            {
                p.size.textInit = true;
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void PushButton::setFontRole(FontRole value)
        {
            const bool changed = value != _fontRole;
            IButton::setFontRole(value);
            TLRENDER_P();
            if (changed)
            {
                p.size.textInit = true;
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void PushButton::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IButton::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::Margin, _displayScale);
                p.size.margin2 = event.style->getSizeRole(SizeRole::MarginInside, _displayScale);
                p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = event.style->getFontRole(_fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->getSize(_text, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint = dtk::Size2I();
            if (!_text.empty())
            {
                _sizeHint.w = p.size.textSize.w + p.size.margin2 * 2;
                _sizeHint.h = p.size.fontMetrics.lineHeight;
            }
            if (_iconImage)
            {
                _sizeHint.w += _iconImage->getWidth();
                if (!_text.empty())
                {
                    _sizeHint.w += p.size.spacing;
                }
                _sizeHint.h = std::max(_sizeHint.h, _iconImage->getHeight());
            }
            _sizeHint.w +=
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.h +=
                p.size.margin2 * 2 +
                p.size.border * 4;
        }

        void PushButton::clipEvent(const dtk::Box2I& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void PushButton::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            TLRENDER_P();

            const dtk::Box2I& g = _geometry;
            const bool enabled = isEnabled();

            // Draw the border.
            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    event.style->getColorRole(ColorRole::KeyFocus));
            } 
            else
            {
                event.render->drawMesh(
                    border(dtk::margin(g, -p.size.border), p.size.border),
                    event.style->getColorRole(ColorRole::Border));
            }

            // Draw the background and checked state.
            const dtk::Box2I g2 = dtk::margin(g, -p.size.border * 2);
            const auto mesh = rect(g2);
            const ColorRole colorRole = _checked ? _checkedRole : _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawMesh(
                    mesh,
                    event.style->getColorRole(colorRole));
            }

            // Draw the pressed and hover states.
            if (_mouse.press && dtk::contains(_geometry, _mouse.pos))
            {
                event.render->drawMesh(
                    mesh,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawMesh(
                    mesh,
                    event.style->getColorRole(ColorRole::Hover));
            }

            // Draw the icon.
            const dtk::Box2I g3 = dtk::margin(
                g2,
                -p.size.margin,
                -p.size.margin2,
                -p.size.margin,
                -p.size.margin2);
            int x = g3.x();
            if (_iconImage)
            {
                const dtk::Size2I& iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  dtk::Box2I(
                      x,
                      g3.y() + g3.h() / 2 - iconSize.h / 2,
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
                const dtk::V2I pos(
                    x + p.size.margin2,
                    g3.y() + g3.h() / 2 - p.size.textSize.h / 2);
                event.render->drawText(
                    p.draw.glyphs,
                    p.size.fontMetrics,
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
            }
        }

        void PushButton::keyPressEvent(KeyEvent& event)
        {
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
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
        }

        void PushButton::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
