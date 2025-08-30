// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoClipItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Util.h>

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct VideoClipItem::Private
        {
            std::string clipName;
            file::Path path;
            std::vector<ftk::InMemoryFile> memoryRead;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct SizeData
            {
                int dragLength = 0;
                ftk::Box2I clipRect;
            };
            SizeData size;

            io::Options ioOptions;
            InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<OTIO_NS::RationalTime, ThumbnailRequest> thumbnailRequests;
        };

        void VideoClipItem::_init(
            const std::shared_ptr<ftk::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>& clip,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto path = timeline::getPath(
                clip->media_reference(),
                itemData->directory,
                itemData->options.pathOptions);
            IBasicItem::_init(
                context,
                !clip->name().empty() ? clip->name() : path.get(-1, file::PathType::FileName),
                ftk::ColorRole::VideoClip,
                "tl::timelineui::VideoClipItem",
                clip.value,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            FTK_P();

            p.clipName = clip->name();
            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());
            p.thumbnailGenerator = thumbnailGenerator;

            p.ioOptions = _data->options.ioOptions;
            p.ioOptions["USD/CameraName"] = p.clipName;
            const std::string infoCacheKey = ThumbnailCache::getInfoKey(
                reinterpret_cast<intptr_t>(this),
                path,
                p.ioOptions);
            const auto i = itemData->info.find(infoCacheKey);
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
            FTK_P();
            _cancelRequests();
        }

        std::shared_ptr<VideoClipItem> VideoClipItem::create(
            const std::shared_ptr<ftk::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>& clip,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
            out->_init(
                context,
                clip,
                scale,
                options,
                displayOptions,
                itemData,
                thumbnailGenerator,
                parent);
            return out;
        }

        void VideoClipItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IBasicItem::setScale(value);
            FTK_P();
            if (changed)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void VideoClipItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _displayOptions.thumbnails ||
                value.thumbnailHeight != _displayOptions.thumbnailHeight;
            IBasicItem::setDisplayOptions(value);
            FTK_P();
            if (thumbnailsChanged)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void VideoClipItem::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const ftk::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            FTK_P();

            // Check if the I/O information is finished.
            if (p.infoRequest.future.valid() &&
                p.infoRequest.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.ioInfo = std::make_shared<io::Info>(p.infoRequest.future.get());
                const std::string infoCacheKey = ThumbnailCache::getInfoKey(
                    reinterpret_cast<intptr_t>(this),
                    p.path,
                    p.ioOptions);
                _data->info[infoCacheKey] = p.ioInfo;
                _setSizeUpdate();
                _setDrawUpdate();
            }

            // Check if any thumbnails are finished.
            auto i = p.thumbnailRequests.begin();
            while (i != p.thumbnailRequests.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto image = i->second.future.get();
                    const std::string cacheKey = ThumbnailCache::getThumbnailKey(
                        reinterpret_cast<intptr_t>(this),
                        p.path,
                        _displayOptions.thumbnailHeight,
                        i->first,
                        p.ioOptions);
                    _data->thumbnails[cacheKey] = image;
                    i = p.thumbnailRequests.erase(i);
                    _setDrawUpdate();
                }
                else
                {
                    ++i;
                }
            }
        }

        void VideoClipItem::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            FTK_P();
            p.size.dragLength = event.style->getSizeRole(ftk::SizeRole::DragLength, event.displayScale);
            ftk::Size2I sizeHint = getSizeHint();
            if (_displayOptions.thumbnails)
            {
                sizeHint.h += _displayOptions.thumbnailHeight;
            }
            _setSizeHint(sizeHint);
        }

        void VideoClipItem::clipEvent(const ftk::Box2I& clipRect, bool clipped)
        {
            IBasicItem::clipEvent(clipRect, clipped);
            FTK_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void VideoClipItem::drawEvent(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_displayOptions.thumbnails)
            {
                _drawThumbnails(drawRect, event);
            }
        }

        void VideoClipItem::_drawThumbnails(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();

            auto render = std::dynamic_pointer_cast<timeline::IRender>(event.render);
            const ftk::Box2I g = _getInsideGeometry();
            const int m = _getMargin();
            const int lineHeight = _getLineHeight();

            const ftk::Box2I box(
                g.min.x,
                g.min.y +
                (!_displayOptions.minimize ? (lineHeight + m * 2) : 0),
                g.w(),
                _displayOptions.thumbnailHeight);
            render->drawRect(
                box,
                ftk::Color4F(0.F, 0.F, 0.F));
            const ftk::ClipRectEnabledState clipRectEnabledState(render);
            const ftk::ClipRectState clipRectState(render);
            render->setClipRectEnabled(true);
            render->setClipRect(ftk::intersect(box, clipRectState.getClipRect()));
            render->setOCIOOptions(_displayOptions.ocio);
            render->setLUTOptions(_displayOptions.lut);

            const ftk::Box2I clipRect = _getClipRect(
                drawRect,
                _displayOptions.clipRectScale);
            if (ftk::intersects(g, clipRect))
            {
                if (!p.ioInfo && !p.infoRequest.future.valid())
                {
                    p.infoRequest = p.thumbnailGenerator->getInfo(
                        reinterpret_cast<intptr_t>(this),
                        p.path,
                        p.memoryRead,
                        p.ioOptions);
                }
            }

            const int thumbnailWidth =
                (_displayOptions.thumbnails && p.ioInfo && !p.ioInfo->video.empty()) ?
                static_cast<int>(_displayOptions.thumbnailHeight * ftk::aspectRatio(p.ioInfo->video[0].size)) :
                0;
            if (thumbnailWidth > 0)
            {
                const int w = g.w();
                const bool enabled = isEnabled();
                for (int x = 0; x < w; x += thumbnailWidth)
                {
                    const ftk::Box2I box(
                        g.min.x +
                        x,
                        g.min.y +
                        (!_displayOptions.minimize ? (lineHeight + m * 2) : 0),
                        thumbnailWidth,
                        _displayOptions.thumbnailHeight);
                    if (ftk::intersects(box, clipRect))
                    {
                        const OTIO_NS::RationalTime time = OTIO_NS::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 1 ? (x / static_cast<double>(w - 1)) : 0) *
                            _timeRange.duration().rescaled_to(_timeRange.start_time()).value(),
                            _timeRange.start_time().rate()).
                            floor();
                        OTIO_NS::TimeRange trimmedRange = _trimmedRange;
                        if (_data->options.compat &&
                            _availableRange.start_time() > p.ioInfo->videoTime.start_time())
                        {
                            //! \bug If the available range is greater than the media time,
                            //! assume the media time is wrong (e.g., Picchu) and
                            //! compensate for it.
                            trimmedRange = OTIO_NS::TimeRange(
                                trimmedRange.start_time() - _availableRange.start_time(),
                                trimmedRange.duration());
                        }
                        const OTIO_NS::RationalTime mediaTime = timeline::toVideoMediaTime(
                            time,
                            _timeRange,
                            trimmedRange,
                            p.ioInfo->videoTime.duration().rate());

                        const std::string cacheKey = ThumbnailCache::getThumbnailKey(
                            reinterpret_cast<intptr_t>(this),
                            p.path,
                            _displayOptions.thumbnailHeight,
                            mediaTime,
                            p.ioOptions);
                        const auto i = _data->thumbnails.find(cacheKey);
                        if (i != _data->thumbnails.end())
                        {
                            if (i->second)
                            {
                                timeline::DisplayOptions displayOptions;
                                if (!enabled)
                                {
                                    displayOptions.color.enabled = true;
                                    displayOptions.color.saturation.x = 0.F;
                                    displayOptions.color.saturation.y = 0.F;
                                    displayOptions.color.saturation.z = 0.F;
                                }
                                timeline::VideoData videoData;
                                videoData.size = i->second->getSize();
                                videoData.layers.push_back({ i->second });
                                render->drawVideo(
                                    { videoData },
                                    { box },
                                    {},
                                    { displayOptions });
                            }
                        }
                        else if (p.ioInfo && !p.ioInfo->video.empty())
                        {
                            const auto k = p.thumbnailRequests.find(mediaTime);
                            if (k == p.thumbnailRequests.end())
                            {
                                p.thumbnailRequests[mediaTime] = p.thumbnailGenerator->getThumbnail(
                                    reinterpret_cast<intptr_t>(this),
                                    p.path,
                                    p.memoryRead,
                                    _displayOptions.thumbnailHeight,
                                    mediaTime,
                                    p.ioOptions);
                            }
                        }
                    }
                }
            }
        }

        void VideoClipItem::_cancelRequests()
        {
            FTK_P();
            std::vector<uint64_t> ids;
            if (p.infoRequest.future.valid())
            {
                ids.push_back(p.infoRequest.id);
                p.infoRequest = InfoRequest();
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
