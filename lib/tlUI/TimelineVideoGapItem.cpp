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
            ui::FontRole fontRole = ui::FontRole::Label;
            int margin = 0;
            int spacing = 0;
        };

        void TimelineVideoGapItem::_init(
            const otio::Gap* gap,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ITimelineItem::_init("TimelineVideoGapItem", itemData, context, parent);
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
            if (_updates & ui::Update::Size)
            {
                _textUpdate();
            }
        }

        void TimelineVideoGapItem::sizeEvent(const ui::SizeEvent& event)
        {
            ITimelineItem::sizeEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
            p.spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall) * event.contentScale;
            const auto fontMetrics = event.getFontMetrics(p.fontRole);

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                p.margin +
                fontMetrics.lineHeight +
                p.spacing +
                _options.thumbnailHeight +
                p.margin);
        }

        void TimelineVideoGapItem::drawEvent(const ui::DrawEvent& event)
        {
            ITimelineItem::drawEvent(event);
            TLRENDER_P();
            if (_isInsideViewport())
            {
                const int b = event.style->getSizeRole(ui::SizeRole::Border) * event.contentScale;
                const auto fontInfo = event.getFontInfo(p.fontRole);
                const auto fontMetrics = event.getFontMetrics(p.fontRole);
                math::BBox2i g = _geometry;

                //event.render->drawMesh(
                //    ui::border(g, b, p.margin / 2),
                //    event.style->getColorRole(ui::ColorRole::Border));

                //event.render->drawRect(
                //    g.margin(-b),
                //    imaging::Color4f(.2F, .25F, .2F));

                event.render->drawText(
                    event.fontSystem->getGlyphs(p.label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        p.margin,
                        g.min.y +
                        p.margin +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));

                math::Vector2i textSize = event.fontSystem->measure(p.durationLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(p.durationLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        p.margin -
                        textSize.x,
                        g.min.y +
                        p.margin +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
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