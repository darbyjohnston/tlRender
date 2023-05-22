// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TrackItem.h>

#include <tlTimelineUI/AudioClipItem.h>
#include <tlTimelineUI/AudioGapItem.h>
#include <tlTimelineUI/VideoClipItem.h>
#include <tlTimelineUI/VideoGapItem.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timelineui
    {
        struct TrackItem::Private
        {
            TrackType trackType = TrackType::None;
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::map<std::shared_ptr<IItem>, otime::TimeRange> childTimeRanges;
            int margin = 0;
        };

        void TrackItem::_init(
            const otio::Track* track,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init("tl::timelineui::TrackItem", itemData, context, parent);
            TLRENDER_P();

            if (otio::Track::Kind::video == track->kind())
            {
                p.trackType = TrackType::Video;
            }
            else if (otio::Track::Kind::audio == track->kind())
            {
                p.trackType = TrackType::Audio;
            }

            p.timeRange = track->trimmed_range();

            for (const auto& child : track->children())
            {
                if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                {
                    std::shared_ptr<IItem> clipItem;
                    switch (p.trackType)
                    {
                    case TrackType::Video:
                        clipItem = VideoClipItem::create(
                            clip,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TrackType::Audio:
                        clipItem = AudioClipItem::create(
                            clip,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    default: break;
                    }
                    const auto timeRangeOpt = track->trimmed_range_of_child(clip);
                    if (timeRangeOpt.has_value())
                    {
                        p.childTimeRanges[clipItem] = timeRangeOpt.value();
                    }
                }
                else if (auto gap = dynamic_cast<otio::Gap*>(child.value))
                {
                    std::shared_ptr<IItem> gapItem;
                    switch (p.trackType)
                    {
                    case TrackType::Video:
                        gapItem = VideoGapItem::create(
                            gap,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TrackType::Audio:
                        gapItem = AudioGapItem::create(
                            gap,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    default: break;
                    }
                    const auto timeRangeOpt = track->trimmed_range_of_child(gap);
                    if (timeRangeOpt.has_value())
                    {
                        p.childTimeRanges[gapItem] = timeRangeOpt.value();
                    }
                }
            }
        }

        TrackItem::TrackItem() :
            _p(new Private)
        {}

        TrackItem::~TrackItem()
        {}

        std::shared_ptr<TrackItem> TrackItem::create(
            const otio::Track* track,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TrackItem>(new TrackItem);
            out->_init(track, itemData, context, parent);
            return out;
        }

        void TrackItem::setGeometry(const math::BBox2i& value)
        {
            IItem::setGeometry(value);
            TLRENDER_P();
            for (auto child : _children)
            {
                if (auto item = std::dynamic_pointer_cast<IItem>(child))
                {
                    const auto i = p.childTimeRanges.find(item);
                    if (i != p.childTimeRanges.end())
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        math::BBox2i bbox(
                            _geometry.min.x +
                            i->second.start_time().rescaled_to(1.0).value() * _scale,
                            _geometry.min.y,
                            sizeHint.x,
                            sizeHint.y);
                        child->setGeometry(bbox);
                    }
                }
            }
        }

        void TrackItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall, event.displayScale);

            int childrenHeight = 0;
            for (const auto& child : _children)
            {
                childrenHeight = std::max(childrenHeight, child->getSizeHint().y);
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _scale,
                childrenHeight);
        }

        void TrackItem::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
        }
    }
}
