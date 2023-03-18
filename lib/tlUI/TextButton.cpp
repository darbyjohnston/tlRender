// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TextButton.h>

namespace tl
{
    namespace ui
    {
        struct TextButton::Private
        {
            imaging::FontInfo fontInfo;
            std::string text;
            bool inside = false;
            math::Vector2i cursorPos;
            bool pressed = false;
            std::shared_ptr<observer::Value<bool> > click;
        };

        void TextButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TextButton", context, parent);
            TLRENDER_P();
            setStretch(Stretch::Expanding, Orientation::Horizontal);
            p.click = observer::Value<bool>::create(false);
        }

        TextButton::TextButton() :
            _p(new Private)
        {}

        TextButton::~TextButton()
        {}

        std::shared_ptr<TextButton> TextButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TextButton>(new TextButton);
            out->_init(context, parent);
            return out;
        }

        const std::string& TextButton::getText() const
        {
            return _p->text;
        }

        void TextButton::setText(const std::string& value)
        {
            _p->text = value;
        }

        const imaging::FontInfo& TextButton::getFontInfo() const
        {
            return _p->fontInfo;
        }

        void TextButton::setFontInfo(const imaging::FontInfo& value)
        {
            _p->fontInfo = value;
        }

        std::shared_ptr<observer::IValue<bool> > TextButton::observeClick() const
        {
            return _p->click;
        }

        void TextButton::sizeHintEvent(const SizeHintEvent& event)
        {
            TLRENDER_P();
            const int m = event.style->getSizeRole(SizeRole::Margin) * event.contentScale;
            const int b = event.style->getSizeRole(SizeRole::Border) * event.contentScale;
            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            _sizeHint.x = event.fontSystem->measure(p.text, fontInfo).x + m * 2 + b * 2;
            _sizeHint.y = fontMetrics.lineHeight + m * 2 + b * 2;
        }

        void TextButton::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const int m = event.style->getSizeRole(SizeRole::Margin) * event.contentScale;
            const int b = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            event.render->drawRect(
                _geometry,
                lighter(event.style->getColorRole(ColorRole::Button), .1F));

            event.render->drawRect(
                _geometry.margin(-b),
                event.style->getColorRole(ColorRole::Button));

            if (p.pressed && _geometry.contains(p.cursorPos))
            {
                event.render->drawRect(
                    _geometry.margin(-b),
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (p.inside)
            {
                event.render->drawRect(
                    _geometry.margin(-b),
                    event.style->getColorRole(ColorRole::Hover));
            }

            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            math::Vector2i textSize = event.fontSystem->measure(p.text, fontInfo);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, fontInfo),
                math::Vector2i(
                    _geometry.x() + _geometry.w() / 2 - textSize.x / 2,
                    _geometry.y() + _geometry.h() / 2 - textSize.y / 2 + fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));
        }

        void TextButton::enterEvent()
        {
            TLRENDER_P();
            p.inside = true;
        }

        void TextButton::leaveEvent()
        {
            TLRENDER_P();
            p.inside = false;
        }

        void TextButton::mouseMoveEvent(const MouseMoveEvent& event)
        {
            TLRENDER_P();
            p.cursorPos = event.pos;
        }

        void TextButton::mousePressEvent(const MouseClickEvent&)
        {
            TLRENDER_P();
            p.pressed = true;
        }

        void TextButton::mouseReleaseEvent(const MouseClickEvent&)
        {
            TLRENDER_P();
            p.pressed = false;
            if (_geometry.contains(p.cursorPos))
            {
                p.click->setAlways(true);
            }
        }
    }
}
