// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

namespace tl
{
    namespace timelineui
    {
        struct VideoClipItem::Private
        {
            otio::SerializableObject::Retainer<otio::Clip> clip;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            otime::TimeRange availableRange = time::invalidTimeRange;

            struct SizeData
            {
                int thumbnailWidth = 0;
                int dragLength = 0;
                math::Box2i clipRect;
            };
            SizeData size;

            std::future<io::Info> infoFuture;
            std::unique_ptr<io::Info> ioInfo;
            std::map<otime::RationalTime, std::future<std::shared_ptr<image::Image> > > thumbnailFutures;
            std::map<otime::RationalTime, std::shared_ptr<image::Image> > thumbnails;
            std::shared_ptr<observer::ValueObserver<bool> > cancelObserver;
        };

        void VideoClipItem::_init(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            const otime::TimeRange& timeRange,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto path = timeline::getPath(
                clip->media_reference(),
                itemData.directory,
                itemData.options.pathOptions);
            IBasicItem::_init(
                !clip->name().empty() ? clip->name() : path.get(-1, false),
                ui::ColorRole::VideoClip,
                getMarkers(clip),
                "tl::timelineui::VideoClipItem",
                timeRange,
                itemData,
                context,
                parent);
            TLRENDER_P();

            p.clip = clip;

            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());

            otio::ErrorStatus error;
            const otime::TimeRange availableRange = clip->available_range(&error);
            if (!otio::is_error(error))
            {
                p.availableRange = availableRange;
            }
            else if (clip->source_range().has_value())
            {
                p.availableRange = clip->source_range().value();
            }

            p.cancelObserver = observer::ValueObserver<bool>::create(
                _data.ioManager->observeCancelRequests(),
                [this](bool)
                {
                    _p->infoFuture = std::future<io::Info>();
                    _p->thumbnailFutures.clear();
                });
        }

        VideoClipItem::VideoClipItem() :
            _p(new Private)
        {}

        VideoClipItem::~VideoClipItem()
        {}

        std::shared_ptr<VideoClipItem> VideoClipItem::create(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            const otime::TimeRange& timeRange,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
            out->_init(clip, timeRange, itemData, context, parent);
            return out;
        }

        const otio::SerializableObject::Retainer<otio::Clip>& VideoClipItem::getClip() const
        {
            return _p->clip;
        }

        void VideoClipItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IBasicItem::setScale(value);
            TLRENDER_P();
            if (changed)
            {
                p.thumbnails.clear();
                _data.ioManager->cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void VideoClipItem::setOptions(const ItemOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _options.thumbnails ||
                value.thumbnailHeight != _options.thumbnailHeight;
            IBasicItem::setOptions(value);
            TLRENDER_P();
            if (thumbnailsChanged)
            {
                p.thumbnails.clear();
                _data.ioManager->cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void VideoClipItem::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();

            // Check if the I/O information is finished.
            if (p.infoFuture.valid() &&
                p.infoFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.ioInfo = std::make_unique<io::Info>(p.infoFuture.get());
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }

            // Check if any thumbnails are finished.
            auto i = p.thumbnailFutures.begin();
            while (i != p.thumbnailFutures.end())
            {
                if (i->second.valid() &&
                    i->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto image = i->second.get();
                    p.thumbnails[i->first] = image;
                    i = p.thumbnailFutures.erase(i);
                    _updates |= ui::Update::Draw;
                }
                else
                {
                    ++i;
                }
            }
        }

        void VideoClipItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            TLRENDER_P();
            const int thumbnailWidth =
                (_options.thumbnails && p.ioInfo && !p.ioInfo->video.empty()) ?
                static_cast<int>(_options.thumbnailHeight * p.ioInfo->video[0].size.getAspect()) :
                0;
            if (thumbnailWidth != p.size.thumbnailWidth)
            {
                p.size.thumbnailWidth = thumbnailWidth;

                p.thumbnails.clear();
                _data.ioManager->cancelRequests();
                _updates |= ui::Update::Draw;
            }
            p.size.dragLength = event.style->getSizeRole(ui::SizeRole::DragLength, event.displayScale);
            if (_options.thumbnails)
            {
                _sizeHint.y += _options.thumbnailHeight;
            }
        }

        void VideoClipItem::clipEvent(
            const math::Box2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            IBasicItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                p.thumbnails.clear();
                _data.ioManager->cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void VideoClipItem::drawEvent(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_options.thumbnails)
            {
                _drawThumbnails(drawRect, event);
            }
        }

        void VideoClipItem::_drawThumbnails(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i g = _getInsideGeometry();
            const int m = _getMargin();

            const math::Box2i box(
                g.min.x,
                g.min.y +
                _getLineHeight() + m * 2,
                g.w(),
                _options.thumbnailHeight);
            event.render->drawRect(
                box,
                image::Color4f(0.F, 0.F, 0.F));
            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(box.intersect(clipRectState.getClipRect()));

            std::set<otime::RationalTime> thumbnailsDelete;
            for (const auto& thumbnail : p.thumbnails)
            {
                thumbnailsDelete.insert(thumbnail.first);
            }

            const math::Box2i clipRect = _getClipRect(
                drawRect,
                _options.clipRectScale);
            if (g.intersects(clipRect))
            {
                if (!p.ioInfo && !p.infoFuture.valid())
                {
                    p.infoFuture = _data.ioManager->requestInfo(
                        p.path,
                        p.memoryRead,
                        p.availableRange.start_time());
                }
            }

            if (p.size.thumbnailWidth > 0)
            {
                const int w = _sizeHint.x;
                for (int x = 0; x < w; x += p.size.thumbnailWidth)
                {
                    const math::Box2i box(
                        g.min.x +
                        x,
                        g.min.y +
                        _getLineHeight() + m * 2,
                        p.size.thumbnailWidth,
                        _options.thumbnailHeight);
                    if (box.intersects(clipRect))
                    {
                        const otime::RationalTime time = time::floor(otime::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w - 1)) : 0) *
                            _timeRange.duration().value(),
                            _timeRange.duration().rate()));

                        const auto i = p.thumbnails.find(time);
                        if (i != p.thumbnails.end())
                        {
                            if (i->second)
                            {
                                event.render->drawImage(i->second, box);
                            }
                            thumbnailsDelete.erase(time);
                        }
                        else if (p.ioInfo && !p.ioInfo->video.empty())
                        {
                            const auto k = p.thumbnailFutures.find(time);
                            if (k == p.thumbnailFutures.end())
                            {
                                const auto mediaTime = timeline::toVideoMediaTime(
                                    time,
                                    p.clip,
                                    p.ioInfo->videoTime.duration().rate());
                                p.thumbnailFutures[time] = _data.ioManager->requestVideo(
                                    math::Size2i(p.size.thumbnailWidth, _options.thumbnailHeight),
                                    p.path,
                                    p.memoryRead,
                                    p.availableRange.start_time(),
                                    mediaTime);
                            }
                        }
                    }
                }
            }

            for (auto i : thumbnailsDelete)
            {
                const auto j = p.thumbnails.find(i);
                if (j != p.thumbnails.end())
                {
                    p.thumbnails.erase(j);
                }
            }
        }
    }
}
