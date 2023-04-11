// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineVideoClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <opentimelineio/track.h>

#include <sstream>

namespace tl
{
    namespace ui
    {
        struct TimelineVideoClipItem::Private
        {
            const otio::Clip* clip = nullptr;
            const otio::Track* track = nullptr;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::string label;
            std::string durationLabel;
            ui::FontRole fontRole = ui::FontRole::Label;
            int margin = 0;
            int spacing = 0;
            int thumbnailWidth = 0;
            bool ioInfoInit = true;
            io::Info ioInfo;
            std::map<otime::RationalTime, std::future<io::VideoData> > videoDataFutures;
            std::map<otime::RationalTime, io::VideoData> videoData;
            std::map<otime::RationalTime, std::shared_ptr<gl::OffscreenBuffer> > buffers;
            std::shared_ptr<observer::ValueObserver<bool> > cancelObserver;
        };

        void TimelineVideoClipItem::_init(
            const otio::Clip* clip,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ITimelineItem::_init("TimelineVideoClipItem", itemData, context, parent);
            TLRENDER_P();

            p.clip = clip;
            p.track = dynamic_cast<otio::Track*>(clip->parent());

            p.path = timeline::getPath(
                p.clip->media_reference(),
                itemData.directory,
                itemData.pathOptions);
            p.memoryRead = timeline::getMemoryRead(
                p.clip->media_reference());

            auto rangeOpt = clip->trimmed_range_in_parent();
            if (rangeOpt.has_value())
            {
                p.timeRange = rangeOpt.value();
            }

            p.label = p.path.get(-1, false);
            _textUpdate();

            p.cancelObserver = observer::ValueObserver<bool>::create(
                _data.ioManager->observeCancelRequests(),
                [this](bool)
                {
                    _p->videoDataFutures.clear();
                });
        }

        TimelineVideoClipItem::TimelineVideoClipItem() :
            _p(new Private)
        {}

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
            TLRENDER_P();
            auto i = p.videoDataFutures.begin();
            while (i != p.videoDataFutures.end())
            {
                if (i->second.valid() &&
                    i->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto videoData = i->second.get();
                    p.videoData[i->first] = videoData;
                    i = p.videoDataFutures.erase(i);
                    _updates |= ui::Update::Draw;
                    continue;
                }
                ++i;
            }
        }

        void TimelineVideoClipItem::sizeEvent(const ui::SizeEvent& event)
        {
            ITimelineItem::sizeEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
            p.spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall) * event.contentScale;
            const auto fontMetrics = event.getFontMetrics(p.fontRole);

            const int thumbnailWidth = !p.ioInfo.video.empty() ?
                static_cast<int>(_options.thumbnailHeight * p.ioInfo.video[0].size.getAspect()) :
                0;
            if (thumbnailWidth != p.thumbnailWidth)
            {
                p.thumbnailWidth = thumbnailWidth;
                _data.ioManager->cancelRequests();
                p.videoData.clear();
                p.buffers.clear();
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                p.margin +
                fontMetrics.lineHeight +
                p.spacing +
                _options.thumbnailHeight +
                p.margin);
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
            TLRENDER_P();
            p.durationLabel = ITimelineItem::_durationLabel(
                p.timeRange.duration(),
                _options.timeUnits);
        }

        void TimelineVideoClipItem::_drawInfo(const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontInfo = event.getFontInfo(p.fontRole);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            math::BBox2i g = _geometry;

            event.render->drawText(
                event.fontSystem->getGlyphs(p.label, fontInfo),
                math::Vector2i(
                    g.min.x +
                    p.margin,
                    g.min.y +
                    p.margin +
                    fontMetrics.ascender),
                event.style->getColorRole(ui::ColorRole::Text));

            math::Vector2i textSize = event.fontSystem->measure(p.durationLabel, fontInfo);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.durationLabel, fontInfo),
                math::Vector2i(
                    g.max.x -
                    p.margin -
                    textSize.x,
                    g.min.y +
                    p.margin +
                    fontMetrics.ascender),
                event.style->getColorRole(ui::ColorRole::Text));
        }

        void TimelineVideoClipItem::_drawThumbnails(const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            const math::BBox2i vp(0, 0, _viewport.w(), _viewport.h());
            math::BBox2i g = _geometry;

            const math::BBox2i bbox(
                g.min.x +
                p.margin,
                g.min.y +
                p.margin +
                fontMetrics.lineHeight +
                p.spacing,
                _sizeHint.x - p.margin * 2,
                _options.thumbnailHeight);
            event.render->drawRect(
                bbox,
                imaging::Color4f(0.F, 0.F, 0.F));
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(bbox);

            std::set<otime::RationalTime> buffersDelete;
            for (const auto& buffers : p.buffers)
            {
                buffersDelete.insert(buffers.first);
            }

            if (g.intersects(vp))
            {
                if (p.ioInfoInit)
                {
                    p.ioInfoInit = false;
                    p.ioInfo = _data.ioManager->getInfo(p.path).get();
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
                }
            }

            if (p.thumbnailWidth > 0)
            {
                for (const auto& i : p.videoData)
                {
                    const imaging::Size renderSize = event.render->getRenderSize();
                    const math::BBox2i viewport = event.render->getViewport();
                    const bool clipRectEnabled = event.render->getClipRectEnabled();
                    const math::BBox2i clipRect = event.render->getClipRect();
                    const math::Matrix4x4f transform = event.render->getTransform();

                    const imaging::Size size(
                        p.thumbnailWidth,
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
                    p.buffers[i.first] = buffer;

                    event.render->setRenderSize(renderSize);
                    event.render->setViewport(viewport);
                    event.render->setClipRectEnabled(clipRectEnabled);
                    event.render->setClipRect(clipRect);
                    event.render->setTransform(transform);
                }
                p.videoData.clear();

                for (int x = p.margin; x < _sizeHint.x - p.margin; x += p.thumbnailWidth)
                {
                    math::BBox2i bbox(
                        g.min.x +
                        x,
                        g.min.y +
                        p.margin +
                        fontMetrics.lineHeight +
                        p.spacing,
                        p.thumbnailWidth,
                        _options.thumbnailHeight);
                    if (bbox.intersects(vp))
                    {
                        const int w = _sizeHint.x - p.margin * 2;
                        const otime::RationalTime time = time::round(otime::RationalTime(
                            p.timeRange.start_time().value() +
                            (w > 0 ? ((x - p.margin) / static_cast<double>(w)) : 0) *
                            p.timeRange.duration().value(),
                            p.timeRange.duration().rate()));

                        const auto i = p.buffers.find(time);
                        if (i != p.buffers.end())
                        {
                            const unsigned int id = i->second->getColorID();
                            event.render->drawTexture(id, bbox);
                            buffersDelete.erase(time);
                        }
                        else if (!p.ioInfo.video.empty())
                        {
                            const auto k = p.videoDataFutures.find(time);
                            if (k == p.videoDataFutures.end())
                            {
                                const auto mediaTime = timeline::mediaTime(
                                    time,
                                    p.track,
                                    p.clip,
                                    p.ioInfo.videoTime.duration().rate());
                                p.videoDataFutures[time] = _data.ioManager->readVideo(p.path, mediaTime);
                            }
                        }
                    }
                }
            }

            for (auto i : buffersDelete)
            {
                const auto j = p.buffers.find(i);
                if (j != p.buffers.end())
                {
                    p.buffers.erase(j);
                }
            }

            event.render->setClipRectEnabled(false);
        }
    }
}