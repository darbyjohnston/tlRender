// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TrackItem.h>

#include <tlTimelineUI/AudioClipItem.h>
#include <tlTimelineUI/AudioGapItem.h>
#include <tlTimelineUI/TransitionItem.h>
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
            std::string label;
            std::string durationLabel;
            std::map<std::shared_ptr<IItem>, otime::TimeRange> itemTimeRanges;
            std::vector<std::shared_ptr<IItem> > clipsAndGaps;
            std::vector<std::shared_ptr<IItem> > transitions;

            struct SizeData
            {
                int margin = 0;
                imaging::FontInfo fontInfo = imaging::FontInfo("", 0);
                imaging::FontMetrics fontMetrics;
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

        void TrackItem::_init(
            const otio::SerializableObject::Retainer<otio::Track>& track,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init("tl::timelineui::TrackItem", itemData, context, parent);
            TLRENDER_P();

            p.label = track->name();
            if (otio::Track::Kind::video == track->kind())
            {
                p.trackType = TrackType::Video;
                if (p.label.empty())
                {
                    p.label = "Video Track";
                }
            }
            else if (otio::Track::Kind::audio == track->kind())
            {
                p.trackType = TrackType::Audio;
                if (p.label.empty())
                {
                    p.label = "Audio Track";
                }
            }
            p.timeRange = track->trimmed_range();

            _textUpdate();

            for (const auto& child : track->children())
            {
                if (auto clip = otio::dynamic_retainer_cast<otio::Clip>(child))
                {
                    std::shared_ptr<IItem> item;
                    switch (p.trackType)
                    {
                    case TrackType::Video:
                        item = VideoClipItem::create(
                            clip,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TrackType::Audio:
                        item = AudioClipItem::create(
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
                        p.itemTimeRanges[item] = timeRangeOpt.value();
                    }
                    p.clipsAndGaps.push_back(item);
                }
                else if (auto gap = otio::dynamic_retainer_cast<otio::Gap>(child))
                {
                    std::shared_ptr<IItem> item;
                    switch (p.trackType)
                    {
                    case TrackType::Video:
                        item = VideoGapItem::create(
                            gap,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TrackType::Audio:
                        item = AudioGapItem::create(
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
                        p.itemTimeRanges[item] = timeRangeOpt.value();
                    }
                    p.clipsAndGaps.push_back(item);
                }
                else if (auto transition = otio::dynamic_retainer_cast<otio::Transition>(child))
                {
                    auto item = TransitionItem::create(
                        transition,
                        itemData,
                        context,
                        shared_from_this());
                    const auto timeRangeOpt = track->trimmed_range_of_child(transition);
                    if (timeRangeOpt.has_value())
                    {
                        const otime::TimeRange timeRange = timeRangeOpt.value();
                        p.itemTimeRanges[item] = timeRange;
                    }
                    p.transitions.push_back(item);
                }
            }
        }

        TrackItem::TrackItem() :
            _p(new Private)
        {}

        TrackItem::~TrackItem()
        {}

        std::shared_ptr<TrackItem> TrackItem::create(
            const otio::SerializableObject::Retainer<otio::Track>& track,
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
            int y = _geometry.min.y +
                p.size.fontMetrics.lineHeight;
            int h = 0;
            for (auto item : p.clipsAndGaps)
            {
                const auto i = p.itemTimeRanges.find(item);
                if (i != p.itemTimeRanges.end())
                {
                    const math::Vector2i& sizeHint = item->getSizeHint();
                    const math::BBox2i bbox(
                        _geometry.min.x +
                        i->second.start_time().rescaled_to(1.0).value() * _scale,
                        y,
                        sizeHint.x,
                        sizeHint.y);
                    item->setGeometry(bbox);
                    h = std::max(h, sizeHint.y);
                }
            }
            y += h;
            for (auto item : p.transitions)
            {
                const auto i = p.itemTimeRanges.find(item);
                if (i != p.itemTimeRanges.end())
                {
                    const math::Vector2i& sizeHint = item->getSizeHint();
                    const math::BBox2i bbox(
                        _geometry.min.x +
                        i->second.start_time().rescaled_to(1.0).value() * _scale,
                        y,
                        sizeHint.x,
                        sizeHint.y);
                    item->setGeometry(bbox);
                }
            }
        }

        void TrackItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);

            auto fontInfo = imaging::FontInfo(
                _options.regularFont,
                _options.fontSize * event.displayScale);
            if (fontInfo != p.size.fontInfo || p.size.textUpdate)
            {
                p.size.fontInfo = fontInfo;
                p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);
                p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
                p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);
            }
            p.size.textUpdate = false;

            int clipsAndGapsHeight = 0;
            for (const auto& item : p.clipsAndGaps)
            {
                clipsAndGapsHeight = std::max(clipsAndGapsHeight, item->getSizeHint().y);
            }
            int transitionsHeight = 0;
            for (const auto& item : p.transitions)
            {
                transitionsHeight = std::max(transitionsHeight, item->getSizeHint().y);
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.fontMetrics.lineHeight +
                clipsAndGapsHeight +
                transitionsHeight);
        }

        void TrackItem::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

            const math::BBox2i labelGeometry(
                g.min.x +
                p.size.margin,
                g.min.y,
                p.size.labelSize.x,
                p.size.fontMetrics.lineHeight);
            const math::BBox2i durationGeometry(
                g.max.x -
                p.size.margin -
                p.size.durationSize.x,
                g.min.y,
                p.size.durationSize.x,
                p.size.fontMetrics.lineHeight);
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
                event.render->drawText(
                    p.draw.labelGlyphs,
                    math::Vector2i(
                        labelGeometry.min.x,
                        labelGeometry.min.y +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            if (durationVisible)
            {
                if (!p.durationLabel.empty() && p.draw.durationGlyphs.empty())
                {
                    p.draw.durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, p.size.fontInfo);
                }
                event.render->drawText(
                    p.draw.durationGlyphs,
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        void TrackItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void TrackItem::_textUpdate()
        {
            TLRENDER_P();
            const otime::RationalTime duration = p.timeRange.duration();
            const bool khz =
                TrackType::Audio == p.trackType ?
                (duration.rate() >= 1000.0) :
                false;
            const otime::RationalTime rescaled = duration.rescaled_to(_data.speed);
            p.durationLabel = string::Format("{0}, {1}{2}").
                arg(_data.timeUnitsModel->getLabel(rescaled)).
                arg(khz ? (duration.rate() / 1000.0) : duration.rate()).
                arg(khz ? "kHz" : "FPS");
            p.size.textUpdate = true;
            p.draw.durationGlyphs.clear();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
    }
}
