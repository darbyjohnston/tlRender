// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IBasicItem.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct IBasicItem::Private
        {
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::string label;
            std::string durationLabel;
            ui::FontRole fontRole = ui::FontRole::Label;
            imaging::Color4f color;

            struct SizeData
            {
                int margin = 0;
                int border = 0;
                imaging::FontInfo fontInfo = imaging::FontInfo("", 0);
                int lineHeight = 0;
                bool textUpdate = true;
                math::Vector2i labelSize;
                math::Vector2i durationSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<imaging::Glyph> > durationGlyphs;
            };
            DrawData draw;
        };

        void IBasicItem::_init(
            const otime::TimeRange& timeRange,
            const std::string& label,
            const imaging::Color4f& color,
            const std::string& name,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init(name, itemData, context, parent);
            TLRENDER_P();

            p.timeRange = timeRange;
            p.label = label;
            p.color = color;

            _textUpdate();
        }

        IBasicItem::IBasicItem() :
            _p(new Private)
        {}

        IBasicItem::~IBasicItem()
        {}

        void IBasicItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);
            p.size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);

            auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            if (fontInfo != p.size.fontInfo || p.size.textUpdate)
            {
                p.size.fontInfo = fontInfo;
                auto fontMetrics = event.getFontMetrics(p.fontRole);
                p.size.lineHeight = fontMetrics.lineHeight;
                p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
                p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);
            }
            p.size.textUpdate = false;

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.lineHeight +
                p.size.border * 2);
        }

        void IBasicItem::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            IItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
            }
        }

        void IBasicItem::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i g = _geometry.margin(-p.size.border);

            event.render->drawRect(g, p.color);

            const math::BBox2i labelGeometry(
                g.min.x +
                p.size.margin,
                g.min.y,
                p.size.labelSize.x,
                p.size.lineHeight);
            const math::BBox2i durationGeometry(
                g.max.x -
                p.size.margin -
                p.size.durationSize.x,
                g.min.y,
                p.size.durationSize.x,
                p.size.lineHeight);
            const bool labelVisible = drawRect.intersects(labelGeometry);
            const bool durationVisible =
                drawRect.intersects(durationGeometry) &&
                !durationGeometry.intersects(labelGeometry);

            if (labelVisible)
            {
                if (!p.label.empty() && p.draw.labelGlyphs.empty())
                {
                    p.draw.labelGlyphs = event.fontSystem->getGlyphs(p.label, p.size.fontInfo);
                }
                const auto fontMetrics = event.getFontMetrics(p.fontRole);
                event.render->drawText(
                    p.draw.labelGlyphs,
                    math::Vector2i(
                        labelGeometry.min.x,
                        labelGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            if (durationVisible)
            {
                if (!p.durationLabel.empty() && p.draw.durationGlyphs.empty())
                {
                    p.draw.durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, p.size.fontInfo);
                }
                const auto fontMetrics = event.getFontMetrics(p.fontRole);
                event.render->drawText(
                    p.draw.durationGlyphs,
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        math::BBox2i IBasicItem::_getInsideGeometry() const
        {
            return _geometry.margin(-_p->size.border);
        }

        int IBasicItem::_getLineHeight() const
        {
            return _p->size.lineHeight;
        }

        void IBasicItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void IBasicItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = IItem::_durationLabel(p.timeRange.duration());
            p.size.textUpdate = true;
            p.draw.durationGlyphs.clear();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
    }
}
