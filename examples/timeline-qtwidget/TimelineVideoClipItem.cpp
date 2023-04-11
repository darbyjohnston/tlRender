// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineVideoClipItem.h"

#include <tlUI/DrawUtil.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <sstream>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void TimelineVideoClipItem::_init(
                const otio::Clip* clip,
                const TimelineItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                ITimelineItem::_init("TimelineVideoClipItem", itemData, context, parent);

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
                _textUpdate();

                _cancelObserver = observer::ValueObserver<bool>::create(
                    _data.ioManager->observeCancelRequests(),
                    [this](bool)
                    {
                        _videoDataFutures.clear();
                    });
            }

            TimelineVideoClipItem::~TimelineVideoClipItem()
            {}

            std::shared_ptr<TimelineVideoClipItem> TimelineVideoClipItem::create(
                const otio::Clip* clip,
                const TimelineItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<TimelineVideoClipItem>(new TimelineVideoClipItem);
                out->_init(clip, itemData, context, parent);
                return out;
            }

            void TimelineVideoClipItem::setOptions(const TimelineItemOptions& value)
            {
                ITimelineItem::setOptions(value);
                if (_updates & ui::Update::Size)
                {
                    _textUpdate();
                    _data.ioManager->cancelRequests();
                }
            }

            void TimelineVideoClipItem::setViewport(const math::BBox2i& value)
            {
                ITimelineItem::setViewport(value);
                if (_updates & ui::Update::Size)
                {
                    _data.ioManager->cancelRequests();
                }
            }

            void TimelineVideoClipItem::tickEvent(const ui::TickEvent& event)
            {
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

            void TimelineVideoClipItem::sizeEvent(const ui::SizeEvent& event)
            {
                ITimelineItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                _spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall) * event.contentScale;
                const auto fontMetrics = event.getFontMetrics(_fontRole);

                const int thumbnailWidth = !_ioInfo.video.empty() ?
                    static_cast<int>(_options.thumbnailHeight * _ioInfo.video[0].size.getAspect()) :
                    0;
                if (thumbnailWidth != _thumbnailWidth)
                {
                    _thumbnailWidth = thumbnailWidth;
                    _data.ioManager->cancelRequests();
                    _videoData.clear();
                    _buffers.clear();
                }

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                    _margin +
                    fontMetrics.lineHeight +
                    _spacing +
                    _options.thumbnailHeight +
                    _margin);
            }

            void TimelineVideoClipItem::drawEvent(const ui::DrawEvent& event)
            {
                ITimelineItem::drawEvent(event);
                if (_geometry.isValid() && _insideViewport())
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

            void TimelineVideoClipItem::_textUpdate()
            {
                _durationLabel = ITimelineItem::_durationLabel(
                    _timeRange.duration(),
                    _options.timeUnits);
            }

            void TimelineVideoClipItem::_drawInfo(const ui::DrawEvent& event)
            {
                const auto fontInfo = event.getFontInfo(_fontRole);
                const auto fontMetrics = event.getFontMetrics(_fontRole);
                math::BBox2i g = _geometry;

                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        fontMetrics.ascender),
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
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            void TimelineVideoClipItem::_drawThumbnails(const ui::DrawEvent& event)
            {
                const auto fontMetrics = event.getFontMetrics(_fontRole);
                const math::BBox2i vp(0, 0, _viewport.w(), _viewport.h());
                math::BBox2i g = _geometry;

                const math::BBox2i bbox(
                    g.min.x +
                    _margin,
                    g.min.y +
                    _margin +
                    fontMetrics.lineHeight +
                    _spacing,
                    _sizeHint.x - _margin * 2,
                    _options.thumbnailHeight);
                event.render->drawRect(
                    bbox,
                    imaging::Color4f(0.F, 0.F, 0.F));
                event.render->setClipRectEnabled(true);
                event.render->setClipRect(bbox);

                std::set<otime::RationalTime> buffersDelete;
                for (const auto& buffers : _buffers)
                {
                    buffersDelete.insert(buffers.first);
                }

                if (g.intersects(vp))
                {
                    if (_ioInfoInit)
                    {
                        _ioInfoInit = false;
                        _ioInfo = _data.ioManager->getInfo(_path).get();
                        _updates |= ui::Update::Size;
                        _updates |= ui::Update::Draw;
                    }
                }

                if (_thumbnailWidth > 0)
                {
                    for (const auto& i : _videoData)
                    {
                        const imaging::Size renderSize = event.render->getRenderSize();
                        const math::BBox2i viewport = event.render->getViewport();
                        const bool clipRectEnabled = event.render->getClipRectEnabled();
                        const math::BBox2i clipRect = event.render->getClipRect();
                        const math::Matrix4x4f transform = event.render->getTransform();

                        const imaging::Size size(
                            _thumbnailWidth,
                            _options.thumbnailHeight);
                        gl::OffscreenBufferOptions options;
                        options.colorType = imaging::PixelType::RGBA_F32;
                        auto buffer = gl::OffscreenBuffer::create(size, options);
                        {
                            gl::OffscreenBufferBinding binding(buffer);
                            event.render->setRenderSize(size);
                            event.render->setViewport(math::BBox2i(0, 0, size.w, size.h));
                            event.render->setClipRectEnabled(false);
                            event.render->clearViewport(imaging::Color4f(0.F, 0.F, 0.F));
                            event.render->setTransform(math::ortho(
                                0.F,
                                static_cast<float>(size.w),
                                0.F,
                                static_cast<float>(size.h),
                                -1.F,
                                1.F));
                            if (i.second.image)
                            {
                                event.render->drawImage(i.second.image, math::BBox2i(0, 0, size.w, size.h));
                            }
                        }
                        _buffers[i.first] = buffer;

                        event.render->setRenderSize(renderSize);
                        event.render->setViewport(viewport);
                        event.render->setClipRectEnabled(clipRectEnabled);
                        event.render->setClipRect(clipRect);
                        event.render->setTransform(transform);
                    }
                    _videoData.clear();

                    for (int x = _margin; x < _sizeHint.x - _margin; x += _thumbnailWidth)
                    {
                        math::BBox2i bbox(
                            g.min.x +
                            x,
                            g.min.y +
                            _margin +
                            fontMetrics.lineHeight +
                            _spacing,
                            _thumbnailWidth,
                            _options.thumbnailHeight);
                        if (bbox.intersects(vp))
                        {
                            const int w = _sizeHint.x - _margin * 2;
                            const otime::RationalTime time = time::round(otime::RationalTime(
                                _timeRange.start_time().value() +
                                (w > 0 ? ((x - _margin) / static_cast<double>(w)) : 0) *
                                _timeRange.duration().value(),
                                _timeRange.duration().rate()));

                            const auto i = _buffers.find(time);
                            if (i != _buffers.end())
                            {
                                const unsigned int id = i->second->getColorID();
                                event.render->drawTexture(id, bbox);
                                buffersDelete.erase(time);
                            }
                            else if (!_ioInfo.video.empty())
                            {
                                const auto k = _videoDataFutures.find(time);
                                if (k == _videoDataFutures.end())
                                {
                                    const auto mediaTime = timeline::mediaTime(
                                        time,
                                        _track,
                                        _clip,
                                        _ioInfo.videoTime.duration().rate());
                                    _videoDataFutures[time] = _data.ioManager->readVideo(_path, mediaTime);
                                }
                            }
                        }
                    }
                }

                for (auto i : buffersDelete)
                {
                    const auto j = _buffers.find(i);
                    if (j != _buffers.end())
                    {
                        _buffers.erase(j);
                    }
                }

                event.render->setClipRectEnabled(false);
            }
        }
    }
}
