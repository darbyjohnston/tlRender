// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "VideoClipItem.h"

#include <tlUI/DrawUtil.h>

#include <QPainter>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void VideoClipItem::_init(
                const otio::Clip* clip,
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("VideoClipItem", timeline, context, parent);

                auto rangeOpt = clip->trimmed_range_in_parent();
                if (rangeOpt.has_value())
                {
                    _timeRange = rangeOpt.value();
                }

                _label = _nameLabel(clip->name());
                _durationLabel = IItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            VideoClipItem::~VideoClipItem()
            {
                _cancelVideoRequests();
            }

            std::shared_ptr<VideoClipItem>  VideoClipItem::create(
                const otio::Clip* clip,
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
                out->_init(clip, timeline, context, parent);
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
                auto i = _videoDataFutures.begin();
                while (i != _videoDataFutures.end())
                {
                    if (i->second.valid() &&
                        i->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        const auto videoData = i->second.get();
                        _videoData[videoData.time] = videoData;
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
                _border = event.style->getSizeRole(ui::SizeRole::Border) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                _fontMetrics = event.fontSystem->getMetrics(fontInfo);

                const auto& info = _timeline->getIOInfo();
                _thumbnailWidth = !info.video.empty() ?
                    static_cast<int>(_thumbnailHeight * info.video[0].size.getAspect()) :
                    0;

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _thumbnailHeight +
                    _margin);
            }

            void VideoClipItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);

                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                event.render->drawMesh(
                    ui::border(g, _border, _margin / 2),
                    event.style->getColorRole(ui::ColorRole::Border));

                event.render->drawRect(
                    g.margin(-_border),
                    imaging::Color4f(.2F, .2F, .4F));

                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                event.render->drawText(
                    event.fontSystem->getGlyphs(_startLabel, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
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
                textSize = event.fontSystem->measure(_endLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_endLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        _margin -
                        textSize.x,
                        g.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));

                const math::BBox2i bbox(
                    g.min.x +
                    _margin,
                    g.min.y +
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing +
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
                for (int x = _margin; x < _sizeHint.x - _margin; x += _thumbnailWidth)
                {
                    math::BBox2i bbox(
                        _geometry.min.x +
                        x,
                        _geometry.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.lineHeight +
                        _spacing,
                        _thumbnailWidth,
                        _thumbnailHeight);
                    if (bbox.intersects(_viewport))
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
                            bbox.min = bbox.min - _viewport.min;
                            bbox.max = bbox.max - _viewport.min;
                            event.render->drawVideo(
                                { i->second },
                                { bbox });
                            videoDataDelete.erase(time);
                        }
                        else
                        {
                            const auto j = _videoDataFutures.find(time);
                            if (j == _videoDataFutures.end())
                            {
                                _videoDataFutures[time] = _timeline->getVideo(time);
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

            std::string VideoClipItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    name :
                    std::string("Clip");
            }

            void VideoClipItem::_cancelVideoRequests()
            {
                //_timeline->cancelRequests();
                //_videoDataFutures.clear();
            }
        }
    }
}
