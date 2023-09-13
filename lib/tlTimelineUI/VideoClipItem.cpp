// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoClipItem.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/ThumbnailSystem.h>

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
            std::weak_ptr<ui::ThumbnailSystem> thumbnailSystem;

            struct SizeData
            {
                int dragLength = 0;
                math::Box2i clipRect;
            };
            SizeData size;

            ui::InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<otime::RationalTime, ui::ThumbnailRequest> thumbnailRequests;
        };

        void VideoClipItem::_init(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            double scale,
            const ItemOptions& options,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto path = timeline::getPath(
                clip->media_reference(),
                itemData->directory,
                itemData->options.pathOptions);
            IBasicItem::_init(
                !clip->name().empty() ? clip->name() : path.get(-1, false),
                ui::ColorRole::VideoClip,
                "tl::timelineui::VideoClipItem",
                clip.value,
                scale,
                options,
                itemData,
                context,
                parent);
            TLRENDER_P();

            p.clip = clip;
            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());
            p.thumbnailSystem = context->getSystem<ui::ThumbnailSystem>();

            const auto i = itemData->info.find(path.get());
            if (i != itemData->info.end())
            {
                p.ioInfo = i->second;
            }
        }

        VideoClipItem::VideoClipItem() :
            _p(new Private)
        {}

        VideoClipItem::~VideoClipItem()
        {
            _cancelRequests();
        }

        std::shared_ptr<VideoClipItem> VideoClipItem::create(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            double scale,
            const ItemOptions& options,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
            out->_init(clip, scale, options, itemData, context, parent);
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
                _cancelRequests();
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
                _cancelRequests();
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
            const std::string fileName = p.path.get();
            if (p.infoRequest.future.valid() &&
                p.infoRequest.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.ioInfo = std::make_shared<io::Info>(p.infoRequest.future.get());
                _data->info[fileName] = p.ioInfo;
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }

            // Check if any thumbnails are finished.
            auto i = p.thumbnailRequests.begin();
            while (i != p.thumbnailRequests.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto image = i->second.future.get();
                    _data->thumbnails[fileName][i->first] = image;
                    i = p.thumbnailRequests.erase(i);
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
            p.size.dragLength = event.style->getSizeRole(ui::SizeRole::DragLength, event.displayScale);
            if (_options.thumbnails)
            {
                _sizeHint.h += _options.thumbnailHeight;
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
                _cancelRequests();
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

            const math::Box2i clipRect = _getClipRect(
                drawRect,
                _options.clipRectScale);
            auto thumbnailSystem = p.thumbnailSystem.lock();
            if (g.intersects(clipRect))
            {
                if (!p.ioInfo && !p.infoRequest.future.valid() && thumbnailSystem)
                {
                    p.infoRequest = thumbnailSystem->getInfo(p.path, p.memoryRead);
                }
            }

            const int thumbnailWidth =
                (_options.thumbnails && p.ioInfo && !p.ioInfo->video.empty()) ?
                static_cast<int>(_options.thumbnailHeight * p.ioInfo->video[0].size.getAspect()) :
                0;
            if (thumbnailWidth > 0 && thumbnailSystem)
            {
                const std::string fileName = p.path.get();
                const int w = _sizeHint.w;
                for (int x = 0; x < w; x += thumbnailWidth)
                {
                    const math::Box2i box(
                        g.min.x +
                        x,
                        g.min.y +
                        _getLineHeight() + m * 2,
                        thumbnailWidth,
                        _options.thumbnailHeight);
                    if (box.intersects(clipRect))
                    {
                        const otime::RationalTime time = time::floor(otime::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w - 1)) : 0) *
                            _timeRange.duration().value(),
                            _timeRange.duration().rate()));
                        const otime::RationalTime mediaTime = timeline::toVideoMediaTime(
                            time,
                            p.clip,
                            p.ioInfo->videoTime.duration().rate());

                        bool found = false;
                        const auto i = _data->thumbnails.find(fileName);
                        if (i != _data->thumbnails.end())
                        {
                            const auto j = i->second.find(mediaTime);
                            if (j != i->second.end())
                            {
                                found = true;
                                if (j->second)
                                {
                                    event.render->drawImage(j->second, box);
                                }
                            }
                        }
                        if (!found && p.ioInfo && !p.ioInfo->video.empty())
                        {
                            const auto k = p.thumbnailRequests.find(mediaTime);
                            if (k == p.thumbnailRequests.end())
                            {
                                p.thumbnailRequests[mediaTime] = thumbnailSystem->getThumbnail(
                                    _options.thumbnailHeight,
                                    p.path,
                                    p.memoryRead,
                                    mediaTime);
                            }
                        }
                    }
                }
            }
        }

        void VideoClipItem::_cancelRequests()
        {
            TLRENDER_P();
            if (auto thumbnailSystem = p.thumbnailSystem.lock())
            {
                std::vector<uint64_t> ids;
                if (p.infoRequest.future.valid())
                {
                    ids.push_back(p.infoRequest.id);
                    p.infoRequest = ui::InfoRequest();
                }
                for (const auto& i : p.thumbnailRequests)
                {
                    ids.push_back(i.second.id);
                }
                p.thumbnailRequests.clear();
                thumbnailSystem->cancelRequests(ids);
            }
        }
    }
}
