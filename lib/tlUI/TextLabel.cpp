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
            IWidget::_init(context, parent);
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

        void TextLabel::sizeHint(const SizeHintData& data)
        {
            TLRENDER_P();
            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= data.contentScale;
            auto fontMetrics = data.fontSystem->getMetrics(fontInfo);
            _sizeHint.x = data.fontSystem->measure(p.text, fontInfo).x;
            _sizeHint.y = fontMetrics.lineHeight;
        }

        void TextLabel::draw(const DrawData& data)
        {
            TLRENDER_P();

            //render->drawRect(_geometry, imaging::Color4f(.5F, .3F, .3F));

            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= data.contentScale;
            auto fontMetrics = data.fontSystem->getMetrics(fontInfo);
            data.render->drawText(
                data.fontSystem->getGlyphs(p.text, fontInfo),
                math::Vector2i(
                    _geometry.x(),
                    _geometry.y() + fontMetrics.ascender),
                data.style->getColorRole(ColorRole::Text));
        }
    }
}
