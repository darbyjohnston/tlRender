// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineVideoClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/RenderUtil.h>
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
            FontRole fontRole = FontRole::Label;
            bool ioInfoInit = true;
            io::Info ioInfo;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                math::Vector2i labelSize;
                math::Vector2i durationSize;
                int thumbnailWidth = 0;
                math::BBox2i clipRect;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<imaging::Glyph> > labelGlyphs;
                std::vector<std::shared_ptr<imaging::Glyph> > durationGlyphs;
            };
            DrawData draw;

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
            ITimelineItem::_init("tl::ui::TimelineVideoClipItem", itemData, context, parent);
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
            const bool changed = value != _options;
            ITimelineItem::setOptions(value);
            TLRENDER_P();
            if (changed)
            {
                _textUpdate();
                _data.ioManager->cancelRequests();
                if (!_options.thumbnails)
                {
                    p.videoData.clear();
                    p.buffers.clear();
                }
                _updates |= Update::Draw;
            }
        }

        void TimelineVideoClipItem::tickEvent(const TickEvent& event)
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
                    _updates |= Update::Draw;
                    continue;
                }
                ++i;
            }
        }

        void TimelineVideoClipItem::sizeHintEvent(const SizeHintEvent& event)
        {
            ITimelineItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
            p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);

            const int thumbnailWidth = (_options.thumbnails && !p.ioInfo.video.empty()) ?
                static_cast<int>(_options.thumbnailHeight * p.ioInfo.video[0].size.getAspect()) :
                0;
            if (thumbnailWidth != p.size.thumbnailWidth)
            {
                p.size.thumbnailWidth = thumbnailWidth;
                _data.ioManager->cancelRequests();
                p.videoData.clear();
                p.buffers.clear();
                _updates |= Update::Draw;
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                p.size.margin +
                fontMetrics.lineHeight +
                p.size.margin);
            if (_options.thumbnails)
            {
                _sizeHint.y += p.size.spacing + _options.thumbnailHeight;
            }
        }

        void TimelineVideoClipItem::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            ITimelineItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
            }
            _data.ioManager->cancelRequests();
            _updates |= Update::Draw;
        }

        void TimelineVideoClipItem::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            ITimelineItem::drawEvent(drawRect, event);

            const int b = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            math::BBox2i g = _geometry;

            //event.render->drawMesh(
            //    border(g, b, _margin / 2),
            //    event.style->getColorRole(ColorRole::Border));

            event.render->drawRect(
                g.margin(-b),
                imaging::Color4f(.2F, .4F, .4F));

            _drawInfo(drawRect, event);
            if (_options.thumbnails)
            {
                _drawThumbnails(drawRect, event);
            }

            //event.render->drawRect(
            //    drawRect,
            //    imaging::Color4f(1.F, 0.F, 0.F, .2F));
        }

        void TimelineVideoClipItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = ITimelineItem::_durationLabel(
                p.timeRange.duration(),
                _options.timeUnits);
        }

        void TimelineVideoClipItem::_drawInfo(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            math::BBox2i g = _geometry;

            const math::BBox2i labelGeometry(
                g.min.x +
                p.size.margin,
                g.min.y +
                p.size.margin,
                p.size.labelSize.x,
                p.size.labelSize.y);
            const math::BBox2i durationGeometry(
                g.max.x -
                p.size.margin -
                p.size.durationSize.x,
                g.min.y +
                p.size.margin,
                p.size.durationSize.x,
                p.size.durationSize.y);
            const bool labelVisible = drawRect.intersects(labelGeometry);
            const bool durationVisible =
                drawRect.intersects(durationGeometry) &&
                !durationGeometry.intersects(labelGeometry);

            if (labelVisible)
            {
                if (!p.label.empty() && p.draw.labelGlyphs.empty())
                {
                    p.draw.labelGlyphs = event.fontSystem->getGlyphs(p.label, fontInfo);
                }
                event.render->drawText(
                    p.draw.labelGlyphs,
                    math::Vector2i(
                        labelGeometry.min.x,
                        labelGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ColorRole::Text));
            }

            if (durationVisible)
            {
                if (!p.durationLabel.empty() && p.draw.durationGlyphs.empty())
                {
                    p.draw.durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, fontInfo);
                }
                event.render->drawText(
                    p.draw.durationGlyphs,
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ColorRole::Text));
            }
        }

        void TimelineVideoClipItem::_drawThumbnails(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            math::BBox2i g = _geometry;

            const math::BBox2i bbox(
                g.min.x +
                p.size.margin,
                g.min.y +
                p.size.margin +
                fontMetrics.lineHeight +
                p.size.spacing,
                _sizeHint.x - p.size.margin * 2,
                _options.thumbnailHeight);
            event.render->drawRect(
                bbox,
                imaging::Color4f(0.F, 0.F, 0.F));
            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(bbox.intersect(clipRectState.getClipRect()));

            std::set<otime::RationalTime> buffersDelete;
            for (const auto& buffers : p.buffers)
            {
                buffersDelete.insert(buffers.first);
            }

            if (g.intersects(drawRect))
            {
                if (p.ioInfoInit)
                {
                    p.ioInfoInit = false;
                    p.ioInfo = _data.ioManager->getInfo(p.path).get();
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                }
            }

            if (p.size.thumbnailWidth > 0)
            {
                {
                    const timeline::ViewportState viewportState(event.render);
                    const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
                    const timeline::ClipRectState clipRectState(event.render);
                    const timeline::TransformState transformState(event.render);
                    const timeline::RenderSizeState renderSizeState(event.render);
                    for (const auto& i : p.videoData)
                    {
                        const imaging::Size size(
                            p.size.thumbnailWidth,
                            _options.thumbnailHeight);
                        gl::OffscreenBufferOptions options;
                        options.colorType = imaging::PixelType::RGB_F32;
                        auto buffer = gl::OffscreenBuffer::create(size, options);
                        {
                            gl::OffscreenBufferBinding binding(buffer);
                            event.render->setRenderSize(size);
                            event.render->setViewport(math::BBox2i(0, 0, size.w, size.h));
                            event.render->setClipRectEnabled(false);
                            event.render->setTransform(
                                math::ortho(
                                    0.F,
                                    static_cast<float>(size.w),
                                    0.F,
                                    static_cast<float>(size.h),
                                    -1.F,
                                    1.F));
                            event.render->clearViewport(imaging::Color4f(0.F, 0.F, 0.F));
                            if (i.second.image)
                            {
                                event.render->drawImage(
                                    i.second.image,
                                    math::BBox2i(0, 0, size.w, size.h));
                            }
                        }
                        p.buffers[i.first] = buffer;
                    }
                }
                p.videoData.clear();

                const int w = _sizeHint.x - p.size.margin * 2;
                for (int x = 0; x < w; x += p.size.thumbnailWidth)
                {
                    const math::BBox2i bbox(
                        g.min.x +
                        p.size.margin +
                        x,
                        g.min.y +
                        p.size.margin +
                        fontMetrics.lineHeight +
                        p.size.spacing,
                        p.size.thumbnailWidth,
                        _options.thumbnailHeight);
                    if (bbox.intersects(drawRect))
                    {
                        const otime::RationalTime time = time::round(otime::RationalTime(
                            p.timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w)) : 0) *
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
        }
    }
}