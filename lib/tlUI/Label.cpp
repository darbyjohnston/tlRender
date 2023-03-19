// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Label.h>

namespace tl
{
    namespace ui
    {
        struct Label::Private
        {
            std::string text;
            imaging::FontInfo fontInfo;
        };

        void Label::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Label", context, parent);
        }

        Label::Label() :
            _p(new Private)
        {}

        Label::~Label()
        {}

        std::shared_ptr<Label> Label::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Label>(new Label);
            out->_init(context, parent);
            return out;
        }

        void Label::setText(const std::string& value)
        {
            _p->text = value;
        }

        void Label::setFontInfo(const imaging::FontInfo& value)
        {
            _p->fontInfo = value;
        }

        void Label::sizeEvent(const SizeEvent& event)
        {
            TLRENDER_P();
            imaging::FontInfo fontInfo = p.fontInfo;
            fontInfo.size *= event.contentScale;
            auto fontMetrics = event.fontSystem->getMetrics(fontInfo);
            _sizeHint.x = event.fontSystem->measure(p.text, fontInfo).x;
            _sizeHint.y = fontMetrics.lineHeight;
        }

        void Label::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
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
