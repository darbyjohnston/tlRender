// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Label.h>

#include <tlUI/GeometryUtil.h>

namespace tl
{
    namespace ui
    {
        struct Label::Private
        {
            std::string text;
            FontRole fontRole = FontRole::Label;
        };

        void Label::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Label", context, parent);
            _hAlign = HAlign::Left;
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
            TLRENDER_P();
            if (value == p.text)
                return;
            p.text = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            const auto fontInfo = event.getFontInfo(p.fontRole);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);

            _sizeHint.x = event.fontSystem->measure(p.text, fontInfo).x;
            _sizeHint.y = fontMetrics.lineHeight;
        }

        void Label::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const auto fontInfo = event.getFontInfo(p.fontRole);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);

            //event.render->drawRect(_geometry, imaging::Color4f(.5F, .3F, .3F));

            math::BBox2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, fontInfo),
                math::Vector2i(g.x(), g.y() + fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));
        }
    }
}
