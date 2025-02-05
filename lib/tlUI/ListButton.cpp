// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/ListButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ListButton::Private
        {
            SizeRole labelMarginRole = SizeRole::MarginInside;

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

        void ListButton::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::ListButton", context, parent);
            setButtonRole(ColorRole::None);
            setAcceptsKeyFocus(true);
        }

        ListButton::ListButton() :
            _p(new Private)
        {}

        ListButton::~ListButton()
        {}

        std::shared_ptr<ListButton> ListButton::create(
            const std::shared_ptr<dtk::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<ListButton>(new ListButton);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<ListButton> ListButton::create(
            const std::string& text,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ListButton>(new ListButton);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void ListButton::setLabelMarginRole(SizeRole value)
        {
            DTK_P();
            if (value == p.labelMarginRole)
                return;
            p.labelMarginRole = value;
            p.size.sizeInit = true;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ListButton::setText(const std::string& value)
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

        void ListButton::setFontRole(FontRole value)
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

        void ListButton::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IButton::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, _displayScale);
                p.size.margin2 = event.style->getSizeRole(p.labelMarginRole, _displayScale);
                p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = event.style->getFontRole(_fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(
                    event.style->getFontRole(_fontRole, _displayScale));
                p.size.textSize = event.fontSystem->getSize(_text, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint = dtk::Size2I();
            if (!_text.empty())
            {
                _sizeHint.w = p.size.textSize.w + p.size.margin2 * 2;
                _sizeHint.h = p.size.fontMetrics.lineHeight + p.size.margin * 2;
            }
            if (_iconImage || _checkedIconImage)
            {
                if (!_text.empty())
                {
                    _sizeHint.w += p.size.spacing;
                }
                dtk::V2I size;
                if (_iconImage)
                {
                    size.x = std::max(size.x, _iconImage->getWidth());
                    size.y = std::max(size.y, _iconImage->getHeight());
                }
                if (_checkedIconImage)
                {
                    size.x = std::max(size.x, _checkedIconImage->getWidth());
                    size.y = std::max(size.y, _checkedIconImage->getHeight());
                }
                _sizeHint.w += size.x;
                _sizeHint.h = std::max(_sizeHint.h, size.y);
            }
            _sizeHint.w += p.size.border * 4;
            _sizeHint.h += p.size.border * 4;
        }

        void ListButton::clipEvent(const dtk::Box2I& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            DTK_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void ListButton::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = _geometry;
            const bool enabled = isEnabled();

            // Draw the background and checked state.
            const ColorRole colorRole = _checked ? _checkedRole : _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(colorRole));
            }

            // Draw the pressed and hover states.
            if (_mouse.press && dtk::contains(_geometry, _mouse.pos))
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_mouse.inside)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Hover));
            }

            // Draw the key focus.
            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }

            // Draw the icon.
            const dtk::Box2I g2 = dtk::margin(g, -p.size.border * 2);
            int x = g2.x();
            if (_checked && _checkedIconImage)
            {
                const dtk::Size2I& iconSize = _checkedIconImage->getSize();
                event.render->drawImage(
                    _checkedIconImage,
                    dtk::Box2I(
                        x,
                        g2.y() + g2.h() / 2 - iconSize.h / 2,
                        iconSize.w,
                        iconSize.h),
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
                x += iconSize.w + p.size.spacing;
            }
            else if (_iconImage)
            {
                const dtk::Size2I& iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  dtk::Box2I(
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
                const dtk::V2I pos(
                    x + p.size.margin2,
                    g2.y() + g2.h() / 2 - p.size.textSize.h / 2);
                event.render->drawText(
                    p.draw.glyphs,
                    p.size.fontMetrics,
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
            }
        }

        void ListButton::keyPressEvent(KeyEvent& event)
        {
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case Key::Enter:
                    event.accept = true;
                    takeKeyFocus();
                    if (_pressedCallback)
                    {
                        _pressedCallback();
                    }
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

        void ListButton::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
