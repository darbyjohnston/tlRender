// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/CheckBox.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct CheckBox::Private
        {
            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                int checkBox = 0;
                bool textInit = true;
                math::Size2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > glyphs;
            };
            DrawData draw;
        };

        void CheckBox::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::CheckBox", context, parent);
            setCheckable(true);
            setAcceptsKeyFocus(true);
            _buttonRole = ColorRole::None;
        }

        CheckBox::CheckBox() :
            _p(new Private)
        {}

        CheckBox::~CheckBox()
        {}

        std::shared_ptr<CheckBox> CheckBox::create(
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<CheckBox>(new CheckBox);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<CheckBox> CheckBox::create(
            const std::string& text,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CheckBox>(new CheckBox);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void CheckBox::setText(const std::string& value)
        {
            const bool changed = value != _text;
            IButton::setText(value);
            TLRENDER_P();
            if (changed)
            {
                p.size.textInit = true;
                p.draw.glyphs.clear();
            }
        }

        void CheckBox::setFontRole(FontRole value)
        {
            const bool changed = value != _fontRole;
            IButton::setFontRole(value);
            TLRENDER_P();
            if (changed)
            {
                p.size.textInit = true;
                p.draw.glyphs.clear();
            }
        }

        void CheckBox::sizeHintEvent(const SizeHintEvent& event)
        {
            IButton::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginInside, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

            p.size.fontMetrics = event.getFontMetrics(_fontRole);
            const auto fontInfo = event.style->getFontRole(_fontRole, event.displayScale);
            if (fontInfo != p.size.fontInfo || p.size.textInit)
            {
                p.size.fontInfo = fontInfo;
                p.size.textInit = false;
                p.size.textSize = event.fontSystem->getSize(_text, fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.checkBox = p.size.fontMetrics.lineHeight * .8F;

            _sizeHint.w =
                p.size.checkBox +
                p.size.spacing +
                p.size.textSize.w + p.size.margin * 2 +
                p.size.margin * 2 +
                p.size.border * 4;
            _sizeHint.h =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2 +
                p.size.border * 4;
        }

        void CheckBox::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IButton::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void CheckBox::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IButton::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            const bool enabled = isEnabled();

            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }

            const math::Box2i g2 = g.margin(-p.size.border * 2);
            if (_mouse.press && _geometry.contains(_mouse.pos))
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

            const math::Box2i g3 = g2.margin(-p.size.margin);
            const math::Box2i checkBox(
                g3.x(),
                g3.y() + g3.h() / 2 - p.size.checkBox / 2,
                p.size.checkBox,
                p.size.checkBox);
            event.render->drawMesh(
                border(checkBox, p.size.border),
                math::Vector2i(),
                event.style->getColorRole(ColorRole::Border));
            event.render->drawRect(
                checkBox.margin(-p.size.border),
                event.style->getColorRole(_checked ? ColorRole::Checked : ColorRole::Base ));

            if (!_text.empty())
            {
                if (p.draw.glyphs.empty())
                {
                    p.draw.glyphs = event.fontSystem->getGlyphs(_text, p.size.fontInfo);
                }
                const math::Vector2i pos(
                    g3.x() + p.size.checkBox + p.size.spacing + p.size.margin,
                    g3.y() + g3.h() / 2 - p.size.textSize.h / 2 +
                    p.size.fontMetrics.ascender);
                event.render->drawText(
                    p.draw.glyphs,
                    pos,
                    event.style->getColorRole(enabled ?
                        ColorRole::Text :
                        ColorRole::TextDisabled));
            }
        }

        void CheckBox::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
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

        void CheckBox::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    }
}
