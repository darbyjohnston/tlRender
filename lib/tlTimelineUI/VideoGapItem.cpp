// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoGapItem.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct VideoGapItem::Private
        {
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::string label;
            std::string durationLabel;
            ui::FontRole fontRole = ui::FontRole::Label;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
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

        void VideoGapItem::_init(
            const otio::Gap* gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init("tl::timelineui::VideoGapItem", itemData, context, parent);
            TLRENDER_P();

            auto rangeOpt = gap->trimmed_range_in_parent();
            if (rangeOpt.has_value())
            {
                p.timeRange = rangeOpt.value();
            }

            p.label = _nameLabel(gap->name());
            _textUpdate();
        }

        VideoGapItem::VideoGapItem() :
            _p(new Private)
        {}

        VideoGapItem::~VideoGapItem()
        {}

        std::shared_ptr<VideoGapItem> VideoGapItem::create(
            const otio::Gap* gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoGapItem>(new VideoGapItem);
            out->_init(gap, itemData, context, parent);
            return out;
        }

        void VideoGapItem::setOptions(const ItemOptions& value)
        {
            IItem::setOptions(value);
            if (_updates & ui::Update::Size)
            {
                _textUpdate();
            }
        }

        void VideoGapItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall, event.displayScale);
            p.size.spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
            p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin +
                fontMetrics.lineHeight +
                p.size.margin);
        }

        void VideoGapItem::clipEvent(
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

        void VideoGapItem::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            const math::BBox2i& g = _geometry;

            const math::BBox2i g2 = g.margin(-p.size.border);
            event.render->drawMesh(
                ui::rect(g2, p.size.margin),
                math::Vector2i(),
                imaging::Color4f(.25F, .31F, .31F));

            const math::BBox2i labelGeometry(
                g.min.x +
                p.size.margin,
                g.min.y +
                p.size.margin,
                p.size.labelSize.x,
                p.size.labelSize.y);
            const math::BBox2i durationGeometry(
                g.max.x -
                p.size.margin -
                p.size.durationSize.x,
                g.min.y +
                p.size.margin,
                p.size.durationSize.x,
                p.size.durationSize.y);
            const bool labelVisible = drawRect.intersects(labelGeometry);
            const bool durationVisible =
                drawRect.intersects(durationGeometry) &&
                !durationGeometry.intersects(labelGeometry);

            if (labelVisible)
            {
                if (!p.label.empty() && p.draw.labelGlyphs.empty())
                {
                    p.draw.labelGlyphs = event.fontSystem->getGlyphs(p.label, fontInfo);
                }
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
                    p.draw.durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, fontInfo);
                }
                event.render->drawText(
                    p.draw.durationGlyphs,
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        void VideoGapItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = IItem::_durationLabel(
                p.timeRange.duration(),
                _options.timeUnits);
            p.draw.durationGlyphs.clear();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        std::string VideoGapItem::_nameLabel(const std::string& name)
        {
            return !name.empty() ?
                name :
                std::string("Gap");
        }
    }
}