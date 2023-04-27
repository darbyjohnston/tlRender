// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineTrackItem.h>

#include <tlUI/TimelineAudioClipItem.h>
#include <tlUI/TimelineAudioGapItem.h>
#include <tlUI/TimelineVideoClipItem.h>
#include <tlUI/TimelineVideoGapItem.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct TimelineTrackItem::Private
        {
            TimelineTrackType trackType = TimelineTrackType::None;
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::map<std::shared_ptr<ITimelineItem>, otime::TimeRange> childTimeRanges;
            int margin = 0;
        };

        void TimelineTrackItem::_init(
            const otio::Track* track,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ITimelineItem::_init("TimelineTrackItem", itemData, context, parent);
            TLRENDER_P();

            if (otio::Track::Kind::video == track->kind())
            {
                p.trackType = TimelineTrackType::Video;
            }
            else if (otio::Track::Kind::audio == track->kind())
            {
                p.trackType = TimelineTrackType::Audio;
            }

            p.timeRange = track->trimmed_range();

            for (const auto& child : track->children())
            {
                if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                {
                    std::shared_ptr<ITimelineItem> clipItem;
                    switch (p.trackType)
                    {
                    case TimelineTrackType::Video:
                        clipItem = TimelineVideoClipItem::create(
                            clip,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TimelineTrackType::Audio:
                        clipItem = TimelineAudioClipItem::create(
                            clip,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    }
                    const auto timeRangeOpt = track->trimmed_range_of_child(clip);
                    if (timeRangeOpt.has_value())
                    {
                        p.childTimeRanges[clipItem] = timeRangeOpt.value();
                    }
                }
                else if (auto gap = dynamic_cast<otio::Gap*>(child.value))
                {
                    std::shared_ptr<ITimelineItem> gapItem;
                    switch (p.trackType)
                    {
                    case TimelineTrackType::Video:
                        gapItem = TimelineVideoGapItem::create(
                            gap,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TimelineTrackType::Audio:
                        gapItem = TimelineAudioGapItem::create(
                            gap,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    }
                    const auto timeRangeOpt = track->trimmed_range_of_child(gap);
                    if (timeRangeOpt.has_value())
                    {
                        p.childTimeRanges[gapItem] = timeRangeOpt.value();
                    }
                }
            }
        }

        TimelineTrackItem::TimelineTrackItem() :
            _p(new Private)
        {}

        TimelineTrackItem::~TimelineTrackItem()
        {}

        std::shared_ptr<TimelineTrackItem> TimelineTrackItem::create(
            const otio::Track* track,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineTrackItem>(new TimelineTrackItem);
            out->_init(track, itemData, context, parent);
            return out;
        }

        void TimelineTrackItem::setGeometry(const math::BBox2i& value)
        {
            ITimelineItem::setGeometry(value);
            TLRENDER_P();
            for (auto child : _children)
            {
                if (auto item = std::dynamic_pointer_cast<ITimelineItem>(child))
                {
                    const auto i = p.childTimeRanges.find(item);
                    if (i != p.childTimeRanges.end())
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        math::BBox2i bbox(
                            _geometry.min.x +
                            i->second.start_time().rescaled_to(1.0).value() * _options.scale,
                            _geometry.min.y,
                            sizeHint.x,
                            sizeHint.y);
                        child->setGeometry(bbox);
                    }
                }
            }
        }

        void TimelineTrackItem::sizeEvent(const ui::SizeEvent& event)
        {
            ITimelineItem::sizeEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;

            int childrenHeight = 0;
            for (const auto& child : _children)
            {
                childrenHeight = std::max(childrenHeight, child->getSizeHint().y);
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                childrenHeight);
        }

        void TimelineTrackItem::drawEvent(const ui::DrawEvent& event)
        {
            ITimelineItem::drawEvent(event);
        }
    }
}