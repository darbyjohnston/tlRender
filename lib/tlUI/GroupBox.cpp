// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
                bool sizeInit = true;
                int margin = 0;
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

        void GroupBox::_init(
            const std::shared_ptr<dtk::Context>& context,
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
            const std::shared_ptr<dtk::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<GroupBox>(new GroupBox);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<GroupBox> GroupBox::create(
            const std::string& text,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GroupBox>(new GroupBox);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void GroupBox::setText(const std::string& value)
        {
            DTK_P();
            if (value == p.text)
                return;
            p.text = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setFontRole(FontRole value)
        {
            DTK_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            DTK_P();
            dtk::Box2I g = value;
            g.min.y += p.size.fontMetrics.lineHeight + p.size.spacing;
            g = dtk::margin(g, -(p.size.border + p.size.margin));
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        void GroupBox::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            DTK_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, _displayScale);
                p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = event.style->getFontRole(p.fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->getSize(p.text, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint = dtk::Size2I();
            for (const auto& child : _children)
            {
                const dtk::Size2I& sizeHint = child->getSizeHint();
                _sizeHint.w = std::max(_sizeHint.w, sizeHint.w);
                _sizeHint.h = std::max(_sizeHint.h, sizeHint.h);
            }
            _sizeHint.w += p.size.margin * 2 + p.size.border * 2;
            _sizeHint.h += p.size.margin * 2 + p.size.border * 2;
            _sizeHint.w = std::max(_sizeHint.w, p.size.textSize.w);
            _sizeHint.h += p.size.fontMetrics.lineHeight + p.size.spacing;
        }

        void GroupBox::clipEvent(const dtk::Box2I& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            DTK_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void GroupBox::drawEvent(
            const dtk::Box2I& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = _geometry;

            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                p.draw.glyphs = event.fontSystem->getGlyphs(p.text, p.size.fontInfo);
            }
            event.render->drawText(
                p.draw.glyphs,
                p.size.fontMetrics,
                g.min,
                event.style->getColorRole(ColorRole::Text));

            const dtk::Box2I g2(
                dtk::V2I(
                    g.min.x,
                    g.min.y + p.size.fontMetrics.lineHeight + p.size.spacing),
                g.max);
            event.render->drawMesh(
                border(g2, p.size.border, p.size.margin),
                event.style->getColorRole(ColorRole::Border));
        }
    }
}
