// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

#include <tlIO/Cache.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timelineui
    {
        struct VideoClipItem::Private
        {
            std::string clipName;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            std::shared_ptr<ui::ThumbnailGenerator> thumbnailGenerator;

            struct SizeData
            {
                bool sizeInit = true;
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
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ui::ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto path = timeline::getPath(
                clip->media_reference(),
                itemData->directory,
                itemData->options.pathOptions);
            IBasicItem::_init(
                !clip->name().empty() ? clip->name() : path.get(-1, file::PathType::FileName),
                ui::ColorRole::VideoClip,
                "tl::timelineui::VideoClipItem",
                clip.value,
                scale,
                options,
                displayOptions,
                itemData,
                context,
                parent);
            TLRENDER_P();

            p.clipName = clip->name();
            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());
            p.thumbnailGenerator = thumbnailGenerator;

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
            TLRENDER_P();
            _cancelRequests();
        }

        std::shared_ptr<VideoClipItem> VideoClipItem::create(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ui::ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
            out->_init(
                clip,
                scale,
                options,
                displayOptions,
                itemData,
                thumbnailGenerator,
                context,
                parent);
            return out;
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

        void VideoClipItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _displayOptions.thumbnails ||
                value.thumbnailHeight != _displayOptions.thumbnailHeight;
            IBasicItem::setDisplayOptions(value);
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
                    io::Options ioOptions = _data->options.ioOptions;
                    ioOptions["USD/cameraName"] = p.clipName;
                    const std::string cacheKey = io::getCacheKey(
                        p.path,
                        i->first,
                        ioOptions,
                        {});
                    _data->thumbnails[cacheKey] = image;
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
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IBasicItem::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.dragLength = event.style->getSizeRole(ui::SizeRole::DragLength, _displayScale);
            }
            p.size.sizeInit = false;

            if (_displayOptions.thumbnails)
            {
                _sizeHint.h += _displayOptions.thumbnailHeight;
            }
        }

        void VideoClipItem::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IBasicItem::clipEvent(clipRect, clipped);
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
            if (_displayOptions.thumbnails)
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
                (_displayOptions.clipInfo ? (_getLineHeight() + m * 2) : 0),
                g.w(),
                _displayOptions.thumbnailHeight);
            event.render->drawRect(
                box,
                image::Color4f(0.F, 0.F, 0.F));
            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(box.intersect(clipRectState.getClipRect()));

            const math::Box2i clipRect = _getClipRect(
                drawRect,
                _displayOptions.clipRectScale);
            if (g.intersects(clipRect))
            {
                if (!p.ioInfo && !p.infoRequest.future.valid())
                {
                    p.infoRequest = p.thumbnailGenerator->getInfo(p.path, p.memoryRead);
                }
            }

            const int thumbnailWidth =
                (_displayOptions.thumbnails && p.ioInfo && !p.ioInfo->video.empty()) ?
                static_cast<int>(_displayOptions.thumbnailHeight * p.ioInfo->video[0].size.getAspect()) :
                0;
            if (thumbnailWidth > 0)
            {
                const int w = _sizeHint.w;
                for (int x = 0; x < w; x += thumbnailWidth)
                {
                    const math::Box2i box(
                        g.min.x +
                        x,
                        g.min.y +
                        (_displayOptions.clipInfo ? (_getLineHeight() + m * 2) : 0),
                        thumbnailWidth,
                        _displayOptions.thumbnailHeight);
                    if (box.intersects(clipRect))
                    {
                        const otime::RationalTime time = time::floor(otime::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w - 1)) : 0) *
                            _timeRange.duration().value(),
                            _timeRange.duration().rate()));
                        const otime::RationalTime mediaTime = timeline::toVideoMediaTime(
                            time,
                            _timeRange,
                            _trimmedRange,
                            p.ioInfo->videoTime.duration().rate());

                        io::Options ioOptions = _data->options.ioOptions;
                        ioOptions["USD/cameraName"] = p.clipName;
                        const std::string cacheKey = io::getCacheKey(
                            p.path,
                            mediaTime,
                            ioOptions,
                            {});
                        const auto i = _data->thumbnails.find(cacheKey);
                        if (i != _data->thumbnails.end())
                        {
                            if (i->second)
                            {
                                event.render->drawImage(i->second, box);
                            }
                        }
                        else if (p.ioInfo && !p.ioInfo->video.empty())
                        {
                            const auto k = p.thumbnailRequests.find(mediaTime);
                            if (k == p.thumbnailRequests.end())
                            {
                                p.thumbnailRequests[mediaTime] = p.thumbnailGenerator->getThumbnail(
                                    p.path,
                                    p.memoryRead,
                                    _displayOptions.thumbnailHeight,
                                    mediaTime,
                                    ioOptions);
                            }
                        }
                    }
                }
            }
        }

        void VideoClipItem::_cancelRequests()
        {
            TLRENDER_P();
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
            p.thumbnailGenerator->cancelRequests(ids);
        }
    }
}
