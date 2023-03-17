// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TextLabel.h>

namespace tl
{
    namespace ui
    {
        struct TextLabel::Private
        {
            imaging::FontInfo fontInfo;
            std::string text;
        };

        void TextLabel::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TextLabel", context, parent);
        }

        TextLabel::TextLabel() :
            _p(new Private)
        {}

        TextLabel::~TextLabel()
        {}

        std::shared_ptr<TextLabel> TextLabel::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TextLabel>(new TextLabel);
            out->_init(context, parent);
            return out;
        }

        const std::string& TextLabel::getText() const
        {
            return _p->text;
        }

        void TextLabel::setText(const std::string& value)
        {
            _p->text = value;
        }

        const imaging::FontInfo& TextLabel::getFontInfo() const
        {
            return _p->fontInfo;
        }

        void TextLabel::setFontInfo(const imaging::FontInfo& value)
        {
            _p->fontInfo = value;
        }

        void TextLabel::sizeHintEvent(const SizeHintEvent& event)
        {
            TLRENDER_P();
            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            _sizeHint.x = event.fontSystem->measure(p.text, fontInfo).x;
            _sizeHint.y = fontMetrics.lineHeight;
        }

        void TextLabel::drawEvent(const DrawEvent& event)
        {
            TLRENDER_P();

            //render->drawRect(_geometry, imaging::Color4f(.5F, .3F, .3F));

            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, fontInfo),
                math::Vector2i(
                    _geometry.x(),
                    _geometry.y() + fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));
        }
    }
}
