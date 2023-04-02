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
            imaging::FontInfo fontInfo;

            struct Size
            {
                imaging::FontInfo fontInfo;
                imaging::FontMetrics fontMetrics;
            };
            Size size;
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

        void Label::setFontInfo(const imaging::FontInfo& value)
        {
            TLRENDER_P();
            if (value == p.fontInfo)
                return;
            p.fontInfo = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.size.fontInfo = p.fontInfo;
            p.size.fontInfo.size *= event.contentScale;
            p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);

            _sizeHint.x = event.fontSystem->measure(p.text, p.size.fontInfo).x;
            _sizeHint.y = p.size.fontMetrics.lineHeight;
        }

        void Label::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            //event.render->drawRect(_geometry, imaging::Color4f(.5F, .3F, .3F));

            math::BBox2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.text, p.size.fontInfo),
                math::Vector2i(g.x(), g.y() + p.size.fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));
        }
    }
}
