// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/GroupBox.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct GroupBox::Private
        {
            std::string text;
            FontRole fontRole = FontRole::Label;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
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

        void GroupBox::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::GroupBox", context, parent);
        }

        GroupBox::GroupBox() :
            _p(new Private)
        {}

        GroupBox::~GroupBox()
        {}

        std::shared_ptr<GroupBox> GroupBox::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GroupBox>(new GroupBox);
            out->_init(context, parent);
            return out;
        }

        void GroupBox::setText(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.text)
                return;
            p.text = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            math::BBox2i g = value;
            g.min.y += p.size.fontMetrics.lineHeight + p.size.spacing;
            g = g.margin(-p.size.margin);
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        void GroupBox::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::Margin, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);

            p.size.fontMetrics = event.getFontMetrics(p.fontRole);
            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            p.size.textSize = event.fontSystem->measure(p.text, fontInfo);

            _sizeHint = math::Vector2i();
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
            }
            _sizeHint.x += p.size.margin * 2 + p.size.border * 2;
            _sizeHint.y += p.size.margin * 2 + p.size.border * 2;
            _sizeHint.x = std::max(_sizeHint.x, p.size.textSize.x);
            _sizeHint.y += p.size.fontMetrics.lineHeight + p.size.spacing;
        }

        void GroupBox::clipEvent(bool clipped, const ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipped, event);
            TLRENDER_P();
            if (changed)
            {
                if (clipped)
                {
                    p.draw.glyphs.clear();
                }
                else
                {
                    const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
                    p.draw.glyphs = event.fontSystem->getGlyphs(p.text, fontInfo);
                }
            }
        }

        void GroupBox::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const math::BBox2i g = _geometry;

            event.render->drawText(
                p.draw.glyphs,
                math::Vector2i(g.x(), g.y() + p.size.fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));

            const math::BBox2i g2(
                math::Vector2i(
                    g.min.x,
                    g.min.y + p.size.fontMetrics.lineHeight + p.size.spacing),
                g.max);
            event.render->drawMesh(
                border(g2, p.size.border, p.size.margin / 2),
                math::Vector2i(),
                event.style->getColorRole(ColorRole::Border));
        }
    }
}
