// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Label.h>

#include <tlUI/LayoutUtil.h>

namespace tl
{
    namespace ui
    {
        struct Label::Private
        {
            std::string text;
            size_t textWidth = 0;
            SizeRole marginRole = SizeRole::None;
            FontRole fontRole = FontRole::Label;

            struct SizeData
            {
                int margin = 0;
                imaging::FontInfo fontInfo = imaging::FontInfo("", 0);
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > glyphs;
            };
            DrawData draw;
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
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setTextWidth(size_t value)
        {
            TLRENDER_P();
            if (value == p.textWidth)
                return;
            p.textWidth = value;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(p.marginRole, event.displayScale);

            auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            p.size.fontInfo = fontInfo;
            p.size.fontMetrics = event.getFontMetrics(p.fontRole);
            p.size.textSize = event.fontSystem->getSize(_getText(), p.size.fontInfo);

            _sizeHint.x =
                p.size.textSize.x +
                p.size.margin * 2;
            _sizeHint.y =
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2;
        }

        void Label::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            IWidget::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void Label::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            //event.render->drawRect(_geometry, imaging::Color4f(.5F, .3F, .3F));

            const math::BBox2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign).margin(-p.size.margin);

            const std::string text = _getText();
            if (!text.empty() && p.draw.glyphs.empty())
            {
                p.draw.glyphs = event.fontSystem->getGlyphs(text, p.size.fontInfo);
            }
            const math::Vector2i pos(
                g.x(),
                g.y() + p.size.fontMetrics.ascender);
            event.render->drawText(
                p.draw.glyphs,
                pos,
                event.style->getColorRole(ColorRole::Text));
        }
        
        std::string Label::_getText() const
        {
            TLRENDER_P();
            std::string out;
            if (!p.text.empty() && p.textWidth > 0)
            {
                out = p.text.substr(0, std::min(p.textWidth, p.text.size()));
            }
            else
            {
                out = p.text;
            }
            return out;
        }
    }
}
