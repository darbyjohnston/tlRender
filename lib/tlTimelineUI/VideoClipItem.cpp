// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoClipItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Util.h>

#include <tlIO/Cache.h>

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
            std::vector<feather_tk::InMemoryFile> memoryRead;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct SizeData
            {
                int dragLength = 0;
                feather_tk::Box2I clipRect;
            };
            SizeData size;

            io::Options ioOptions;
            InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<OTIO_NS::RationalTime, ThumbnailRequest> thumbnailRequests;
        };

        void VideoClipItem::_init(
            const std::shared_ptr<feather_tk::Context>& context,
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
                feather_tk::ColorRole::VideoClip,
                "tl::timelineui::VideoClipItem",
                clip.value,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            FEATHER_TK_P();

            p.clipName = clip->name();
            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());
            p.thumbnailGenerator = thumbnailGenerator;

            p.ioOptions = _data->options.ioOptions;
            p.ioOptions["USD/CameraName"] = p.clipName;
            const std::string infoCacheKey = io::getInfoCacheKey(path, p.ioOptions);
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
            FEATHER_TK_P();
            _cancelRequests();
        }

        std::shared_ptr<VideoClipItem> VideoClipItem::create(
            const std::shared_ptr<feather_tk::Context>& context,
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
            FEATHER_TK_P();
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
            FEATHER_TK_P();
            if (thumbnailsChanged)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void VideoClipItem::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const feather_tk::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            FEATHER_TK_P();

            // Check if the I/O information is finished.
            if (p.infoRequest.future.valid() &&
                p.infoRequest.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.ioInfo = std::make_shared<io::Info>(p.infoRequest.future.get());
                const std::string infoCacheKey = io::getInfoCacheKey(p.path, p.ioOptions);
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
                    const std::string cacheKey = io::getVideoCacheKey(
                        p.path,
                        i->first,
                        p.ioOptions,
                        {});
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

        void VideoClipItem::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            FEATHER_TK_P();
            p.size.dragLength = event.style->getSizeRole(feather_tk::SizeRole::DragLength, event.displayScale);
            feather_tk::Size2I sizeHint = getSizeHint();
            if (_displayOptions.thumbnails)
            {
                sizeHint.h += _displayOptions.thumbnailHeight;
            }
            _setSizeHint(sizeHint);
        }

        void VideoClipItem::clipEvent(const feather_tk::Box2I& clipRect, bool clipped)
        {
            IBasicItem::clipEvent(clipRect, clipped);
            FEATHER_TK_P();
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
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_displayOptions.thumbnails)
            {
                _drawThumbnails(drawRect, event);
            }
        }

        void VideoClipItem::_drawThumbnails(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();

            auto render = std::dynamic_pointer_cast<timeline::IRender>(event.render);
            const feather_tk::Box2I g = _getInsideGeometry();
            const int m = _getMargin();
            const int lineHeight = _getLineHeight();

            const feather_tk::Box2I box(
                g.min.x,
                g.min.y +
                (_displayOptions.clipInfo ? (lineHeight + m * 2) : 0),
                g.w(),
                _displayOptions.thumbnailHeight);
            render->drawRect(
                box,
                feather_tk::Color4F(0.F, 0.F, 0.F));
            const feather_tk::ClipRectEnabledState clipRectEnabledState(render);
            const feather_tk::ClipRectState clipRectState(render);
            render->setClipRectEnabled(true);
            render->setClipRect(feather_tk::intersect(box, clipRectState.getClipRect()));
            render->setOCIOOptions(_displayOptions.ocio);
            render->setLUTOptions(_displayOptions.lut);

            const feather_tk::Box2I clipRect = _getClipRect(
                drawRect,
                _displayOptions.clipRectScale);
            if (feather_tk::intersects(g, clipRect))
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
                static_cast<int>(_displayOptions.thumbnailHeight * feather_tk::aspectRatio(p.ioInfo->video[0].size)) :
                0;
            if (thumbnailWidth > 0)
            {
                const int w = g.w();
                const bool enabled = isEnabled();
                for (int x = 0; x < w; x += thumbnailWidth)
                {
                    const feather_tk::Box2I box(
                        g.min.x +
                        x,
                        g.min.y +
                        (_displayOptions.clipInfo ? (lineHeight + m * 2) : 0),
                        thumbnailWidth,
                        _displayOptions.thumbnailHeight);
                    if (feather_tk::intersects(box, clipRect))
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

                        const std::string cacheKey = io::getVideoCacheKey(
                            p.path,
                            mediaTime,
                            p.ioOptions,
                            {});
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
            FEATHER_TK_P();
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
