// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/ToolButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ToolButton::Private
        {
            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
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

        void ToolButton::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::ToolButton", context, parent);
            setAcceptsKeyFocus(true);
            _buttonRole = ColorRole::None;
        }

        ToolButton::ToolButton() :
            _p(new Private)
        {}

        ToolButton::~ToolButton()
        {}

        std::shared_ptr<ToolButton> ToolButton::create(
            const std::shared_ptr<dtk::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<ToolButton>(new ToolButton);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<ToolButton> ToolButton::create(
            const std::string& text,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolButton>(new ToolButton);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void ToolButton::setText(const std::string& value)
        {
            const bool changed = value != _text;
            IButton::setText(value);
            DTK_P();
            if (changed)
            {
                p.size.textInit = true;
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void ToolButton::setFontRole(FontRole value)
        {
            const bool changed = value != _fontRole;
            IButton::setFontRole(value);
            DTK_P();
            if (changed)
            {
                p.size.textInit = true;
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
        }

        void ToolButton::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IButton::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, _displayScale);
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
                _sizeHint.w = p.size.textSize.w + p.size.margin * 2;
                _sizeHint.h = p.size.fontMetrics.lineHeight;
                if (_icon.empty())
                {
                    const int max = std::max(_sizeHint.w, _sizeHint.h);
                    _sizeHint.w = max;
                    _sizeHint.h = _sizeHint.h;
                }
            }
            std::shared_ptr<dtk::Image> iconImage = _iconImage;
            if (_checked && _checkedIconImage)
            {
                iconImage = _checkedIconImage;
            }
            if (iconImage)
            {
                _sizeHint.w += iconImage->getWidth();
                if (!_text.empty())
                {
                    _sizeHint.w += p.size.spacing;
                }
                _sizeHint.h = std::max(
                    _sizeHint.h,
                    static_cast<int>(iconImage->getHeight()));
            }
            _sizeHint.w +=
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.h +=
                p.size.margin * 2 +
                p.size.border * 4;
        }

        void ToolButton::clipEvent(const dtk::Box2I& clipRect, bool clipped)
        {
            IButton::clipEvent(clipRect, clipped);
            DTK_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void ToolButton::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = _geometry;
            const bool enabled = isEnabled();

            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }

            const dtk::Box2I g2 = dtk::margin(g, -p.size.border * 2);
            const ColorRole colorRole = _checked ? _checkedRole : _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(colorRole));
            }

            if (_mouse.press && dtk::contains(_geometry, _mouse.pos))
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g2,
                    event.style->getColorRole(ColorRole::Hover));
            }

            const dtk::Box2I g3 = dtk::margin(g2, -p.size.margin);
            int x = g3.x();
            std::shared_ptr<dtk::Image> iconImage = _iconImage;
            if (_checked && _checkedIconImage)
            {
                iconImage = _checkedIconImage;
            }
            if (iconImage)
            {
                const dtk::Size2I& iconSize = iconImage->getSize();
                event.render->drawImage(
                  iconImage,
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
            
            if (!_text.empty())
            {
                if (p.draw.glyphs.empty())
                {
                    p.draw.glyphs = event.fontSystem->getGlyphs(_text, p.size.fontInfo);
                }
                const int x2 = !_icon.empty() ?
                    (x + p.size.margin) :
                    (g3.x() + g3.w() / 2 - p.size.textSize.w / 2);
                const dtk::V2I pos(
                    x2,
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

        void ToolButton::keyPressEvent(KeyEvent& event)
        {
            DTK_P();
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

        void ToolButton::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
