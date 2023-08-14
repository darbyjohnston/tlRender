// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TrackItem.h>

#include <tlTimelineUI/AudioClipItem.h>
#include <tlTimelineUI/AudioGapItem.h>
#include <tlTimelineUI/Edit.h>
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
            std::shared_ptr<timeline::Player> player;
            otio::SerializableObject::Retainer<otio::Track> track;
            TrackType trackType = TrackType::None;
            int trackIndex = 0;
            std::string label;
            std::string durationLabel;

            struct SizeData
            {
                int margin = 0;
                int handle = 0;
                image::FontInfo fontInfo = image::FontInfo("", 0);
                image::FontMetrics fontMetrics;
                bool textUpdate = true;
                math::Size2i labelSize;
                math::Size2i durationSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<image::Glyph> > durationGlyphs;
                std::vector<math::Box2i> dropTargets;
            };
            DrawData draw;

            struct MouseData
            {
                std::vector<math::Box2i> dropTargets;
                int currentDropTarget = -1;
            };
            MouseData mouse;
        };

        void TrackItem::_init(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Track>& track,
            int trackIndex,
            const otime::TimeRange& timeRange,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init(
                "tl::timelineui::TrackItem",
                timeRange,
                itemData,
                context,
                parent);
            TLRENDER_P();

            _setMouseHover(true);

            p.player = player;
            p.track = track;
            p.trackIndex = trackIndex;
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

            for (const auto& child : track->children())
            {
                otime::TimeRange timeRange = time::invalidTimeRange;
                const auto timeRangeOpt = track->trimmed_range_of_child(child);
                if (timeRangeOpt.has_value())
                {
                    timeRange = timeRangeOpt.value();
                }
                if (auto clip = otio::dynamic_retainer_cast<otio::Clip>(child))
                {
                    std::shared_ptr<IItem> item;
                    switch (p.trackType)
                    {
                    case TrackType::Video:
                        item = VideoClipItem::create(
                            clip,
                            timeRange,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TrackType::Audio:
                        item = AudioClipItem::create(
                            clip,
                            timeRange,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    default: break;
                    }
                }
                else if (auto gap = otio::dynamic_retainer_cast<otio::Gap>(child))
                {
                    std::shared_ptr<IItem> item;
                    switch (p.trackType)
                    {
                    case TrackType::Video:
                        item = VideoGapItem::create(
                            gap,
                            timeRange,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    case TrackType::Audio:
                        item = AudioGapItem::create(
                            gap,
                            timeRange,
                            itemData,
                            context,
                            shared_from_this());
                        break;
                    default: break;
                    }
                }
                else if (auto transition = otio::dynamic_retainer_cast<otio::Transition>(child))
                {
                    auto item = TransitionItem::create(
                        transition,
                        timeRange,
                        itemData,
                        context,
                        shared_from_this());
                }
            }

            _textUpdate();
            _transitionsUpdate();
        }

        TrackItem::TrackItem() :
            _p(new Private)
        {}

        TrackItem::~TrackItem()
        {}

        std::shared_ptr<TrackItem> TrackItem::create(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Track>& track,
            int trackIndex,
            const otime::TimeRange& timeRange,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TrackItem>(new TrackItem);
            out->_init(player, track, trackIndex, timeRange, itemData, context, parent);
            return out;
        }

        void TrackItem::setOptions(const ItemOptions& value)
        {
            const bool changed = value != _options;
            IItem::setOptions(value);
            if (changed)
            {
                _transitionsUpdate();
            }
        }

        void TrackItem::setGeometry(const math::Box2i& value)
        {
            IItem::setGeometry(value);
            TLRENDER_P();
            int y = _geometry.min.y +
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2;
            int h = 0;
            for (const auto& child : _children)
            {
                if (auto item = std::dynamic_pointer_cast<IItem>(child))
                {
                    if (std::dynamic_pointer_cast<VideoClipItem>(child) ||
                        std::dynamic_pointer_cast<VideoGapItem>(child) ||
                        std::dynamic_pointer_cast<AudioClipItem>(child) ||
                        std::dynamic_pointer_cast<AudioGapItem>(child))
                    {
                        const otime::TimeRange& timeRange = item->getTimeRange();
                        const math::Vector2i& sizeHint = item->getSizeHint();
                        const math::Box2i box(
                            _geometry.min.x +
                            timeRange.start_time().rescaled_to(1.0).value() * _scale,
                            y,
                            sizeHint.x,
                            sizeHint.y);
                        item->setGeometry(box);
                        h = std::max(h, sizeHint.y);
                    }
                }
            }
            y += h;
            for (const auto& child : _children)
            {
                if (auto item = std::dynamic_pointer_cast<TransitionItem>(child))
                {
                    const otime::TimeRange& timeRange = item->getTimeRange();
                    const math::Vector2i& sizeHint = item->getSizeHint();
                    const math::Box2i box(
                        _geometry.min.x +
                        timeRange.start_time().rescaled_to(1.0).value() * _scale,
                        y,
                        sizeHint.x,
                        sizeHint.y);
                    item->setGeometry(box);
                }
            }
        }

        void TrackItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);
            p.size.handle = event.style->getSizeRole(ui::SizeRole::Handle, event.displayScale);

            auto fontInfo = image::FontInfo(
                _options.regularFont,
                _options.fontSize * event.displayScale);
            if (fontInfo != p.size.fontInfo || p.size.textUpdate)
            {
                p.size.fontInfo = fontInfo;
                p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);
                p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
                p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
            }
            p.size.textUpdate = false;

            int clipsAndGapsHeight = 0;
            for (const auto& child : _children)
            {
                if (auto item = std::dynamic_pointer_cast<IItem>(child))
                {
                    if (std::dynamic_pointer_cast<VideoClipItem>(child) ||
                        std::dynamic_pointer_cast<VideoGapItem>(child) ||
                        std::dynamic_pointer_cast<AudioClipItem>(child) ||
                        std::dynamic_pointer_cast<AudioGapItem>(child))
                    {
                        clipsAndGapsHeight = std::max(clipsAndGapsHeight, item->getSizeHint().y);
                    }
                }
            }
            int transitionsHeight = 0;
            if (_options.showTransitions)
            {
                for (const auto& child : _children)
                {
                    if (auto item = std::dynamic_pointer_cast<TransitionItem>(child))
                    {
                        transitionsHeight = std::max(transitionsHeight, item->getSizeHint().y);
                    }
                }
            }

            _sizeHint = math::Vector2i(
                _timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.fontMetrics.lineHeight +
                p.size.margin * 2 +
                clipsAndGapsHeight +
                transitionsHeight);
        }

        void TrackItem::drawEvent(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            const math::Box2i labelGeometry(
                g.min.x +
                p.size.margin,
                g.min.y +
                p.size.margin,
                p.size.labelSize.w,
                p.size.fontMetrics.lineHeight);
            const math::Box2i durationGeometry(
                g.max.x -
                p.size.durationSize.w -
                p.size.margin,
                g.min.y +
                p.size.margin,
                p.size.durationSize.w,
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

            if (p.mouse.currentDropTarget >= 0 &&
                p.mouse.currentDropTarget < p.draw.dropTargets.size())
            {
                const math::Box2i& g2 = p.draw.dropTargets[p.mouse.currentDropTarget];
                geom::TriangleMesh2 mesh;
                mesh.v.push_back(math::Vector2f(g2.min.x, g2.min.y));
                mesh.v.push_back(math::Vector2f(g2.max.x, g2.min.y));
                mesh.v.push_back(math::Vector2f((g2.min.x + g2.max.x) / 2.F, g2.max.y));
                mesh.triangles.push_back({ 1, 2, 3 });
                event.render->drawMesh(
                    mesh,
                    math::Vector2i(),
                    event.style->getColorRole(ui::ColorRole::Checked));
            }
        }

        void TrackItem::dragEnterEvent(ui::DragAndDropEvent& event)
        {
            TLRENDER_P();
            if (auto data = std::dynamic_pointer_cast<DragAndDropData>(event.data))
            {
                auto video = std::dynamic_pointer_cast<VideoClipItem>(data->getItem());
                auto audio = std::dynamic_pointer_cast<AudioClipItem>(data->getItem());
                if ((video && video->getClip()->parent() == p.track) ||
                    (audio && audio->getClip()->parent() == p.track))
                {
                    event.accept = true;
                    p.draw.dropTargets.clear();
                    p.mouse.dropTargets.clear();
                    const math::Box2i& g = getGeometry();
                    const float h = p.size.fontMetrics.lineHeight + p.size.margin * 2;
                    for (const auto& child : _children)
                    {
                        if (auto item = std::dynamic_pointer_cast<IItem>(child))
                        {
                            const otime::TimeRange& timeRange = item->getTimeRange();
                            const float x = _timeToPos(timeRange.start_time());
                            p.draw.dropTargets.push_back(math::Box2i(x - h / 2, g.min.y, h, h));
                            p.mouse.dropTargets.push_back(math::Box2i(
                                x - _options.thumbnailHeight,
                                g.min.y,
                                _options.thumbnailHeight * 2,
                                g.h()));
                        }
                    }
                    if (!_children.empty())
                    {
                        if (auto item = std::dynamic_pointer_cast<IItem>(_children.back()))
                        {
                            const otime::TimeRange& timeRange = item->getTimeRange();
                            const float x = _timeToPos(timeRange.end_time_exclusive());
                            p.draw.dropTargets.push_back(math::Box2i(x - h / 2, g.min.y, h, h));
                            p.mouse.dropTargets.push_back(math::Box2i(
                                x - _options.thumbnailHeight,
                                g.min.y,
                                _options.thumbnailHeight * 2,
                                g.h()));
                        }
                    }
                    if (!p.draw.dropTargets.empty())
                    {
                        _updates |= ui::Update::Draw;
                    }
                }
            }
        }

        void TrackItem::dragLeaveEvent(ui::DragAndDropEvent& event)
        {
            TLRENDER_P();
            if (auto data = std::dynamic_pointer_cast<DragAndDropData>(event.data))
            {
                auto video = std::dynamic_pointer_cast<VideoClipItem>(data->getItem());
                auto audio = std::dynamic_pointer_cast<AudioClipItem>(data->getItem());
                if ((video && video->getClip()->parent() == p.track) ||
                    (audio && audio->getClip()->parent() == p.track))
                {
                    event.accept = true;
                    p.mouse.dropTargets.clear();
                    if (!p.draw.dropTargets.empty())
                    {
                        p.draw.dropTargets.clear();
                        _updates |= ui::Update::Draw;
                    }
                }
            }
        }

        void TrackItem::dragMoveEvent(ui::DragAndDropEvent& event)
        {
            TLRENDER_P();
            if (auto data = std::dynamic_pointer_cast<DragAndDropData>(event.data))
            {
                auto video = std::dynamic_pointer_cast<VideoClipItem>(data->getItem());
                auto audio = std::dynamic_pointer_cast<AudioClipItem>(data->getItem());
                if ((video && video->getClip()->parent() == p.track) ||
                    (audio && audio->getClip()->parent() == p.track))
                {
                    event.accept = true;
                    int dropTarget = -1;
                    for (size_t i = 0; i < p.mouse.dropTargets.size(); ++i)
                    {
                        if (p.mouse.dropTargets[i].contains(event.pos))
                        {
                            dropTarget = i;
                            break;
                        }
                    }
                    if (dropTarget != p.mouse.currentDropTarget)
                    {
                        p.mouse.currentDropTarget = dropTarget;
                        _updates |= ui::Update::Draw;
                    }
                }
            }
        }

        void TrackItem::dropEvent(ui::DragAndDropEvent& event)
        {
            TLRENDER_P();
            if (auto data = std::dynamic_pointer_cast<DragAndDropData>(event.data))
            {
                auto video = std::dynamic_pointer_cast<VideoClipItem>(data->getItem());
                auto audio = std::dynamic_pointer_cast<AudioClipItem>(data->getItem());
                if ((video && video->getClip()->parent() == p.track) ||
                    (audio && audio->getClip()->parent() == p.track))
                {
                    if (p.mouse.currentDropTarget != -1)
                    {
                        event.accept = true;
                        auto otioTimeline = insert(
                            p.player->getTimeline()->getTimeline().value,
                            video ? video->getClip() : (audio ? audio->getClip() : nullptr),
                            p.trackIndex,
                            p.mouse.currentDropTarget);
                        p.player->getTimeline()->setTimeline(otioTimeline);
                    }
                }
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
            const otime::RationalTime duration = _timeRange.duration();
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

        void TrackItem::_transitionsUpdate()
        {
            TLRENDER_P();
            for (const auto& child : _children)
            {
                if (auto item = std::dynamic_pointer_cast<TransitionItem>(child))
                {
                    item->setVisible(_options.showTransitions);
                }
            }
        }
    }
}
