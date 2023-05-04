// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineVideoGapItem.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct TimelineVideoGapItem::Private
        {
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::string label;
            std::string durationLabel;
            FontRole fontRole = FontRole::Label;
            int margin = 0;
            int spacing = 0;
        };

        void TimelineVideoGapItem::_init(
            const otio::Gap* gap,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ITimelineItem::_init("tl::ui::TimelineVideoGapItem", itemData, context, parent);
            TLRENDER_P();

            auto rangeOpt = gap->trimmed_range_in_parent();
            if (rangeOpt.has_value())
            {
                p.timeRange = rangeOpt.value();
            }

            p.label = _nameLabel(gap->name());
            _textUpdate();
        }

        TimelineVideoGapItem::TimelineVideoGapItem() :
            _p(new Private)
        {}

        TimelineVideoGapItem::~TimelineVideoGapItem()
        {}

        std::shared_ptr<TimelineVideoGapItem> TimelineVideoGapItem::create(
            const otio::Gap* gap,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineVideoGapItem>(new TimelineVideoGapItem);
            out->_init(gap, itemData, context, parent);
            return out;
        }

        void TimelineVideoGapItem::setOptions(const TimelineItemOptions& value)
        {
            ITimelineItem::setOptions(value);
            if (_updates & Update::Size)
            {
                _textUpdate();
            }
        }

        void TimelineVideoGapItem::sizeHintEvent(const SizeHintEvent& event)
        {
            ITimelineItem::sizeHintEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);
            p.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                p.margin +
                fontMetrics.lineHeight +
                p.margin);
        }

        void TimelineVideoGapItem::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            ITimelineItem::drawEvent(drawRect, event);
            TLRENDER_P();
            if (_geometry.isValid() && _geometry.intersects(drawRect))
            {
                const int b = event.style->getSizeRole(SizeRole::Border, event.displayScale);
                const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
                const auto fontMetrics = event.getFontMetrics(p.fontRole);
                const math::BBox2i& g = _geometry;

                //event.render->drawMesh(
                //    border(g, b, p.margin / 2),
                //    math::Vector2i(),
                //    event.style->getColorRole(ColorRole::Border));

                event.render->drawRect(
                    g.margin(-b),
                    imaging::Color4f(.25F, .31F, .31F));

                event.render->drawText(
                    event.fontSystem->getGlyphs(p.label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        p.margin,
                        g.min.y +
                        p.margin +
                        fontMetrics.ascender),
                    event.style->getColorRole(ColorRole::Text));

                const math::Vector2i textSize = event.fontSystem->getSize(p.durationLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(p.durationLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        p.margin -
                        textSize.x,
                        g.min.y +
                        p.margin +
                        fontMetrics.ascender),
                    event.style->getColorRole(ColorRole::Text));
            }
        }

        void TimelineVideoGapItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = ITimelineItem::_durationLabel(
                p.timeRange.duration(),
                _options.timeUnits);
        }

        std::string TimelineVideoGapItem::_nameLabel(const std::string& name)
        {
            return !name.empty() ?
                name :
                std::string("Gap");
        }
    }
}