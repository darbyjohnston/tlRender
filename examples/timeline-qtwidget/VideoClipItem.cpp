// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "VideoClipItem.h"

#include <tlUI/DrawUtil.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void VideoClipItem::_init(
                const otio::Clip* clip,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("VideoClipItem", itemData, context, parent);

                _clip = clip;
                _track = dynamic_cast<otio::Track*>(clip->parent());

                _path = timeline::getPath(
                    _clip->media_reference(),
                    itemData.directory,
                    itemData.pathOptions);
                _memoryRead = timeline::getMemoryRead(
                    _clip->media_reference());

                auto rangeOpt = clip->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _path.get(-1, false);
                _durationLabel = IItem::_durationLabel(_timeRange.duration(), _timeUnits);
            }

            VideoClipItem::~VideoClipItem()
            {
                _cancelVideoRequests();
            }

            std::shared_ptr<VideoClipItem>  VideoClipItem::create(
                const otio::Clip* clip,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
                out->_init(clip, itemData, context, parent);
                return out;
            }

            void VideoClipItem::setScale(float value)
            {
                IItem::setScale(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelVideoRequests();
                }
            }

            void VideoClipItem::setThumbnailHeight(int value)
            {
                IItem::setThumbnailHeight(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelVideoRequests();
                }
            }

            void VideoClipItem::setViewport(const math::BBox2i& value)
            {
                IItem::setViewport(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelVideoRequests();
                }
            }

            void VideoClipItem::tickEvent(const ui::TickEvent& event)
            {
                if (_ioInfoFuture.valid() &&
                    _ioInfoFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    _ioInfo = _ioInfoFuture.get();
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }

                auto i = _videoDataFutures.begin();
                while (i != _videoDataFutures.end())
                {
                    if (i->second.valid() &&
                        i->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        const auto videoData = i->second.get();
                        _videoData[i->first] = videoData;
                        i = _videoDataFutures.erase(i);
                        _updates |= ui::Update::Draw;
                        continue;
                    }
                    ++i;
                }
            }

            void VideoClipItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                _spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                _fontMetrics = event.fontSystem->getMetrics(fontInfo);

                _thumbnailWidth = !_ioInfo.video.empty() ?
                    static_cast<int>(_thumbnailHeight * _ioInfo.video[0].size.getAspect()) :
                    0;

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _thumbnailHeight +
                    _margin);
            }

            void VideoClipItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);
                if (_insideViewport())
                {
                    const int b = event.style->getSizeRole(ui::SizeRole::Border) * event.contentScale;
                    const math::BBox2i vp(0, 0, _viewport.w(), _viewport.h());
                    math::BBox2i g = _geometry;

                    //event.render->drawMesh(
                    //    ui::border(g, b, _margin / 2),
                    //    event.style->getColorRole(ui::ColorRole::Border));

                    event.render->drawRect(
                        g.margin(-b),
                        imaging::Color4f(.2F, .4F, .4F));

                    _drawInfo(event);
                    _drawThumbnails(event);
                }
            }

            void VideoClipItem::_drawInfo(const ui::DrawEvent& event)
            {
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                math::BBox2i g = _geometry;

                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));

                math::Vector2i textSize = event.fontSystem->measure(_durationLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_durationLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        _margin -
                        textSize.x,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            void VideoClipItem::_drawThumbnails(const ui::DrawEvent& event)
            {
                const math::BBox2i vp(0, 0, _viewport.w(), _viewport.h());
                math::BBox2i g = _geometry;

                const math::BBox2i bbox(
                    g.min.x +
                    _margin,
                    g.min.y +
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing,
                    _sizeHint.x - _margin * 2,
                    _thumbnailHeight);
                event.render->drawRect(
                    bbox,
                    imaging::Color4f(0.F, 0.F, 0.F));
                event.render->setClipRectEnabled(true);
                event.render->setClipRect(bbox);

                std::set<otime::RationalTime> videoDataDelete;
                for (const auto& videoData : _videoData)
                {
                    videoDataDelete.insert(videoData.first);
                }

                if (g.intersects(vp))
                {
                    if (!_reader)
                    {
                        if (auto context = _context.lock())
                        {
                            try
                            {
                                auto ioSystem = context->getSystem<io::System>();
                                _reader = ioSystem->read(
                                    _path,
                                    _memoryRead,
                                    _itemData.ioOptions);
                                _ioInfoFuture = _reader->getInfo();
                            }
                            catch (const std::exception&)
                            {
                            }
                        }
                    }
                }
                else
                {
                    _reader.reset();
                }

                for (int x = _margin; x < _sizeHint.x - _margin; x += _thumbnailWidth)
                {
                    math::BBox2i bbox(
                        g.min.x +
                        x,
                        g.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing,
                        _thumbnailWidth,
                        _thumbnailHeight);
                    if (bbox.intersects(vp))
                    {
                        const int w = _sizeHint.x - _margin * 2;
                        const otime::RationalTime time = time::round(otime::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? ((x - _margin) / static_cast<double>(w)) : 0) *
                            _timeRange.duration().value(),
                            _timeRange.duration().rate()));
                        auto i = _videoData.find(time);
                        if (i != _videoData.end())
                        {
                            if (i->second.image)
                            {
                                event.render->drawImage(i->second.image, bbox);
                            }
                            videoDataDelete.erase(time);
                        }
                        else if (_reader && !_ioInfo.video.empty())
                        {
                            const auto j = _videoDataFutures.find(time);
                            if (j == _videoDataFutures.end())
                            {
                                const auto mediaTime = timeline::mediaTime(
                                    time,
                                    _track,
                                    _clip,
                                    _ioInfo.videoTime.duration().rate());
                                _videoDataFutures[time] = _reader->readVideo(mediaTime);
                            }
                        }
                    }
                }

                for (auto i : videoDataDelete)
                {
                    const auto j = _videoData.find(i);
                    if (j != _videoData.end())
                    {
                        _videoData.erase(j);
                    }
                }

                event.render->setClipRectEnabled(false);
            }

            void VideoClipItem::_cancelVideoRequests()
            {
                if (_reader)
                {
                    _reader->cancelRequests();
                }
                _videoDataFutures.clear();
            }
        }
    }
}
