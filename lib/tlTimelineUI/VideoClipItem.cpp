// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

#include <opentimelineio/track.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        struct VideoClipItem::Private
        {
            const otio::Clip* clip = nullptr;
            const otio::Track* track = nullptr;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            otime::TimeRange timeRange = time::invalidTimeRange;
            otime::TimeRange availableRange = time::invalidTimeRange;
            std::string label;
            std::string durationLabel;
            ui::FontRole fontRole = ui::FontRole::Label;
            bool ioInfoInit = true;
            io::Info ioInfo;

            struct SizeData
            {
                int margin = 0;
                int border = 0;
                imaging::FontInfo fontInfo;
                int lineHeight = 0;
                bool textUpdate = true;
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
            struct Thumbnail
            {
                std::shared_ptr<gl::OffscreenBuffer> buffer;
                std::chrono::steady_clock::time_point time;
            };
            std::map<otime::RationalTime, Thumbnail> thumbnails;
            std::list<std::shared_ptr<gl::OffscreenBuffer> > bufferPool;
            std::shared_ptr<observer::ValueObserver<bool> > cancelObserver;
        };

        void VideoClipItem::_init(
            const otio::Clip* clip,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init("tl::timelineui::VideoClipItem", itemData, context, parent);
            TLRENDER_P();

            p.clip = clip;
            p.track = dynamic_cast<otio::Track*>(clip->parent());

            p.path = timeline::getPath(
                p.clip->media_reference(),
                itemData.directory,
                itemData.options.pathOptions);
            p.memoryRead = timeline::getMemoryRead(
                p.clip->media_reference());

            auto rangeOpt = clip->trimmed_range_in_parent();
            if (rangeOpt.has_value())
            {
                p.timeRange = rangeOpt.value();
            }
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

            p.label = clip->name();
            if (p.label.empty())
            {
                p.label = p.path.get(-1, false);
            }
            _textUpdate();

            p.cancelObserver = observer::ValueObserver<bool>::create(
                _data.ioManager->observeCancelRequests(),
                [this](bool)
                {
                    _p->videoDataFutures.clear();
                });
        }

        VideoClipItem::VideoClipItem() :
            _p(new Private)
        {}

        VideoClipItem::~VideoClipItem()
        {}

        std::shared_ptr<VideoClipItem> VideoClipItem::create(
            const otio::Clip* clip,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoClipItem>(new VideoClipItem);
            out->_init(clip, itemData, context, parent);
            return out;
        }

        void VideoClipItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IItem::setScale(value);
            TLRENDER_P();
            if (changed)
            {
                _data.ioManager->cancelRequests();
                if (!_options.thumbnails)
                {
                    p.videoData.clear();
                    p.thumbnails.clear();
                    p.bufferPool.clear();
                }
            }
        }

        void VideoClipItem::setOptions(const ItemOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _options.thumbnails ||
                value.thumbnailHeight != _options.thumbnailHeight;
            IItem::setOptions(value);
            TLRENDER_P();
            if (thumbnailsChanged)
            {
                _data.ioManager->cancelRequests();
                if (!_options.thumbnails)
                {
                    p.videoData.clear();
                    p.thumbnails.clear();
                    p.bufferPool.clear();
                }
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

            // Check if any thumbnail reads are finished.
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

            // Check if any thumbnails need to be redrawn.
            const auto now = std::chrono::steady_clock::now();
            for (const auto& thumbnail : p.thumbnails)
            {
                const std::chrono::duration<float> diff = now - thumbnail.second.time;
                if (diff.count() < _options.thumbnailFade)
                {
                    _updates |= ui::Update::Draw;
                }
            }
        }

        void VideoClipItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);
            p.size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);

            auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            if (fontInfo != p.size.fontInfo || p.size.textUpdate)
            {
                p.size.fontInfo = fontInfo;
                auto fontMetrics = event.getFontMetrics(p.fontRole);
                p.size.lineHeight = fontMetrics.lineHeight;
                p.size.labelSize = event.fontSystem->getSize(p.label, fontInfo);
                p.size.durationSize = event.fontSystem->getSize(p.durationLabel, fontInfo);
            }
            p.size.textUpdate = false;

            const int thumbnailWidth = (_options.thumbnails && !p.ioInfo.video.empty()) ?
                static_cast<int>(_options.thumbnailHeight * p.ioInfo.video[0].size.getAspect()) :
                0;
            if (thumbnailWidth != p.size.thumbnailWidth)
            {
                p.size.thumbnailWidth = thumbnailWidth;
                _data.ioManager->cancelRequests();
                p.videoData.clear();
                p.thumbnails.clear();
                p.bufferPool.clear();
                _updates |= ui::Update::Draw;
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.lineHeight +
                p.size.border * 2);
            if (_options.thumbnails)
            {
                _sizeHint.y += _options.thumbnailHeight;
            }
        }

        void VideoClipItem::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            IItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                p.draw.labelGlyphs.clear();
                p.draw.durationGlyphs.clear();
                p.videoData.clear();
                p.thumbnails.clear();
                p.bufferPool.clear();
            }
            _data.ioManager->cancelRequests();
            _updates |= ui::Update::Draw;
        }

        void VideoClipItem::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            TLRENDER_P();
            
            const math::BBox2i g = _geometry.margin(-p.size.border);

            event.render->drawRect(
                g,
                _options.colors[ColorRole::VideoClip]);

            _drawInfo(drawRect, event);
            if (_options.thumbnails)
            {
                _drawThumbnails(drawRect, event);
            }
        }

        void VideoClipItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
        }

        void VideoClipItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = IItem::_durationLabel(p.timeRange.duration());
            p.size.textUpdate = true;
            p.draw.durationGlyphs.clear();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void VideoClipItem::_drawInfo(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::BBox2i g = _geometry.margin(-p.size.border);

            const math::BBox2i labelGeometry(
                g.min.x +
                p.size.margin,
                g.min.y,
                p.size.labelSize.x,
                p.size.lineHeight);
            const math::BBox2i durationGeometry(
                g.max.x -
                p.size.margin -
                p.size.durationSize.x,
                g.min.y,
                p.size.durationSize.x,
                p.size.lineHeight);
            const bool labelVisible = drawRect.intersects(labelGeometry);
            const bool durationVisible =
                drawRect.intersects(durationGeometry) &&
                !durationGeometry.intersects(labelGeometry);

            if (labelVisible)
            {
                if (!p.label.empty() && p.draw.labelGlyphs.empty())
                {
                    p.draw.labelGlyphs = event.fontSystem->getGlyphs(p.label, p.size.fontInfo);
                }
                const auto fontMetrics = event.getFontMetrics(p.fontRole);
                event.render->drawText(
                    p.draw.labelGlyphs,
                    math::Vector2i(
                        labelGeometry.min.x,
                        labelGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            if (durationVisible)
            {
                if (!p.durationLabel.empty() && p.draw.durationGlyphs.empty())
                {
                    p.draw.durationGlyphs = event.fontSystem->getGlyphs(p.durationLabel, p.size.fontInfo);
                }
                const auto fontMetrics = event.getFontMetrics(p.fontRole);
                event.render->drawText(
                    p.draw.durationGlyphs,
                    math::Vector2i(
                        durationGeometry.min.x,
                        durationGeometry.min.y +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        void VideoClipItem::_drawThumbnails(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::BBox2i g = _geometry.margin(-p.size.border);
            const auto now = std::chrono::steady_clock::now();

            const math::BBox2i bbox(
                g.min.x,
                g.min.y +
                p.size.lineHeight,
                g.w(),
                _options.thumbnailHeight);
            event.render->drawRect(
                bbox,
                imaging::Color4f(0.F, 0.F, 0.F));
            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(bbox.intersect(clipRectState.getClipRect()));

            std::set<otime::RationalTime> thumbnailsDelete;
            for (const auto& thumbnail : p.thumbnails)
            {
                thumbnailsDelete.insert(thumbnail.first);
            }

            const math::BBox2i clipRect = _getClipRect(
                drawRect,
                _options.clipRectScale);
            if (g.intersects(clipRect))
            {
                if (p.ioInfoInit)
                {
                    p.ioInfoInit = false;
                    p.ioInfo = _data.ioManager->getInfo(
                        p.path,
                        p.memoryRead,
                        p.availableRange.start_time()).get();
                    _updates |= ui::Update::Size;
                    _updates |= ui::Update::Draw;
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
                        std::shared_ptr<gl::OffscreenBuffer> buffer;
                        if (p.bufferPool.size())
                        {
                            buffer = p.bufferPool.front();
                            p.bufferPool.pop_front();
                        }
                        else
                        {
                            gl::OffscreenBufferOptions options;
                            options.colorType = imaging::PixelType::RGB_F32;
                            buffer = gl::OffscreenBuffer::create(size, options);
                        }
                        if (buffer)
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
                        p.thumbnails[i.first] = { buffer, now };
                    }
                }
                p.videoData.clear();

                const int w = _sizeHint.x;
                for (int x = 0; x < w; x += p.size.thumbnailWidth)
                {
                    const math::BBox2i bbox(
                        g.min.x +
                        x,
                        g.min.y +
                        p.size.lineHeight,
                        p.size.thumbnailWidth,
                        _options.thumbnailHeight);
                    if (bbox.intersects(clipRect))
                    {
                        const otime::RationalTime time = time::round(otime::RationalTime(
                            p.timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w)) : 0) *
                            p.timeRange.duration().value(),
                            p.timeRange.duration().rate()));

                        const auto i = p.thumbnails.find(time);
                        if (i != p.thumbnails.end())
                        {
                            const unsigned int id = i->second.buffer->getColorID();
                            const std::chrono::duration<float> diff = now - i->second.time;
                            const float a = std::min(diff.count() / _options.thumbnailFade, 1.F);
                            event.render->drawTexture(
                                id,
                                bbox,
                                imaging::Color4f(1.F, 1.F, 1.F, a));
                            thumbnailsDelete.erase(time);
                        }
                        else if (!p.ioInfo.video.empty())
                        {
                            const auto k = p.videoDataFutures.find(time);
                            if (k == p.videoDataFutures.end())
                            {
                                const auto mediaTime = timeline::toVideoMediaTime(
                                    time,
                                    p.track,
                                    p.clip,
                                    p.ioInfo);
                                p.videoDataFutures[time] = _data.ioManager->readVideo(
                                    p.path,
                                    p.memoryRead,
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
                    p.bufferPool.push_back(j->second.buffer);
                    p.thumbnails.erase(j);
                }
            }
        }
    }
}
