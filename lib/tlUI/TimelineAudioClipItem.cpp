// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineAudioClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/AudioConvert.h>
#include <tlCore/Mesh.h>

#include <opentimelineio/track.h>

#include <sstream>

namespace tl
{
    namespace ui
    {
        struct TimelineAudioClipItem::Private
        {
            const otio::Clip* clip = nullptr;
            const otio::Track* track = nullptr;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            otime::TimeRange timeRange = time::invalidTimeRange;
            std::string label;
            std::string durationLabel;
            FontRole fontRole = FontRole::Label;
            int margin = 0;
            int spacing = 0;
            int waveformWidth = 0;
            math::BBox2i clipRect;
            bool ioInfoInit = true;
            io::Info ioInfo;
            struct AudioFuture
            {
                std::future<io::AudioData> future;
                math::Vector2i size;
            };
            std::map<otime::RationalTime, AudioFuture> audioDataFutures;
            struct AudioData
            {
                io::AudioData audio;
                math::Vector2i size;
                std::future<std::shared_ptr<geom::TriangleMesh2> > meshFuture;
                std::shared_ptr<geom::TriangleMesh2> mesh;
            };
            std::map<otime::RationalTime, AudioData> audioData;
            std::shared_ptr<observer::ValueObserver<bool> > cancelObserver;
        };

        void TimelineAudioClipItem::_init(
            const otio::Clip* clip,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ITimelineItem::_init("tl::ui::TimelineAudioClipItem", itemData, context, parent);
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
                    _p->audioDataFutures.clear();
                });
        }

        TimelineAudioClipItem::TimelineAudioClipItem() :
            _p(new Private)
        {}

        TimelineAudioClipItem::~TimelineAudioClipItem()
        {}

        std::shared_ptr<TimelineAudioClipItem> TimelineAudioClipItem::create(
            const otio::Clip* clip,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineAudioClipItem>(new TimelineAudioClipItem);
            out->_init(clip, itemData, context, parent);
            return out;
        }

        void TimelineAudioClipItem::setOptions(const TimelineItemOptions& value)
        {
            const bool changed = value != _options;
            ITimelineItem::setOptions(value);
            TLRENDER_P();
            if (changed)
            {
                _textUpdate();
                _data.ioManager->cancelRequests();
                p.audioData.clear();
                _updates |= Update::Draw;
            }
        }

        namespace
        {
            std::shared_ptr<geom::TriangleMesh2> audioMesh(
                const std::shared_ptr<audio::Audio>& audio,
                const math::Vector2i& size)
            {
                auto out = std::shared_ptr<geom::TriangleMesh2>(new geom::TriangleMesh2);
                const auto& info = audio->getInfo();
                const size_t sampleCount = audio->getSampleCount();
                if (sampleCount > 0)
                {
                    switch (info.dataType)
                    {
                    case audio::DataType::F32:
                    {
                        const audio::F32_T* data = reinterpret_cast<const audio::F32_T*>(
                            audio->getData());
                        for (int x = 0; x < size.x; ++x)
                        {
                            const int x0 = std::min(
                                static_cast<size_t>((x + 0) / static_cast<double>(size.x - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            const int x1 = std::min(
                                static_cast<size_t>((x + 1) / static_cast<double>(size.x - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            //std::cout << x << ": " << x0 << " " << x1 << std::endl;
                            audio::F32_T min = 0.F;
                            audio::F32_T max = 0.F;
                            if (x0 < x1)
                            {
                                min = audio::F32Range.getMax();
                                max = audio::F32Range.getMin();
                                for (int i = x0; i < x1; ++i)
                                {
                                    const audio::F32_T v = *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            const int h2 = size.y / 2;
                            const math::BBox2i bbox(
                                math::Vector2i(
                                    x,
                                    h2 - h2 * max),
                                math::Vector2i(
                                    x + 1,
                                    h2 - h2 * min));
                            if (bbox.isValid())
                            {
                                const size_t j = 1 + out->v.size();
                                out->v.push_back(math::Vector2f(bbox.x(), bbox.y()));
                                out->v.push_back(math::Vector2f(bbox.x() + bbox.w(), bbox.y()));
                                out->v.push_back(math::Vector2f(bbox.x() + bbox.w(), bbox.y() + bbox.h()));
                                out->v.push_back(math::Vector2f(bbox.x(), bbox.y() + bbox.h()));
                                out->triangles.push_back(geom::Triangle2({ j + 0, j + 1, j + 2 }));
                                out->triangles.push_back(geom::Triangle2({ j + 2, j + 3, j + 0 }));
                            }
                        }
                        break;
                    }
                    }
                }
                return out;
            }
        }

        void TimelineAudioClipItem::tickEvent(const TickEvent& event)
        {
            TLRENDER_P();
            auto i = p.audioDataFutures.begin();
            while (i != p.audioDataFutures.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto audio = i->second.future.get();
                    const auto size = i->second.size;
                    Private::AudioData audioData;
                    audioData.audio = audio;
                    audioData.size = size;
                    if (audio.audio)
                    {
                        audioData.meshFuture = std::async(
                            std::launch::async,
                            [audio, size]
                            {
                                auto convert = audio::AudioConvert::create(
                                    audio.audio->getInfo(),
                                    audio::Info(1, audio::DataType::F32, audio.audio->getSampleRate()));
                            const auto convertedAudio = convert->convert(audio.audio);
                            return audioMesh(convertedAudio, size);
                            });
                    }
                    p.audioData[i->first] = std::move(audioData);
                    i = p.audioDataFutures.erase(i);
                    continue;
                }
                ++i;
            }

            auto j = p.audioData.begin();
            while (j != p.audioData.end())
            {
                if (j->second.meshFuture.valid() &&
                    j->second.meshFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto mesh = j->second.meshFuture.get();
                    j->second.mesh = mesh;
                    _updates |= Update::Draw;
                }
                ++j;
            }
        }

        void TimelineAudioClipItem::sizeHintEvent(const SizeHintEvent& event)
        {
            ITimelineItem::sizeHintEvent(event);
            TLRENDER_P();

            p.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);
            p.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);

            const int waveformWidth = _options.thumbnails ?
                (otime::RationalTime(1.0, 1.0).value() * _options.scale) :
                0;
            if (waveformWidth != p.waveformWidth)
            {
                p.waveformWidth = waveformWidth;
                _data.ioManager->cancelRequests();
                p.audioData.clear();
                _updates |= Update::Draw;
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _options.scale,
                p.margin +
                fontMetrics.lineHeight +
                p.margin);
            if (_options.thumbnails)
            {
                _sizeHint.y += p.spacing + _options.waveformHeight;
            }
        }

        void TimelineAudioClipItem::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            ITimelineItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipRect == p.clipRect)
                return;
            _data.ioManager->cancelRequests();
            _updates |= Update::Draw;
        }

        void TimelineAudioClipItem::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            ITimelineItem::drawEvent(drawRect, event);
            if (_geometry.isValid() && _geometry.intersects(drawRect))
            {
                const int b = event.style->getSizeRole(SizeRole::Border, event.displayScale);
                const math::BBox2i& g = _geometry;

                //event.render->drawMesh(
                //    border(g, b, _margin / 2),
                //    event.style->getColorRole(ColorRole::Border));

                event.render->drawRect(
                    g.margin(-b),
                    imaging::Color4f(.3F, .25F, .4F));

                _drawInfo(drawRect, event);
                if (_options.thumbnails)
                {
                    _drawWaveforms(drawRect, event);
                }
            }
        }

        void TimelineAudioClipItem::_textUpdate()
        {
            TLRENDER_P();
            p.durationLabel = ITimelineItem::_durationLabel(
                p.timeRange.duration(),
                _options.timeUnits);
        }

        void TimelineAudioClipItem::_drawInfo(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            const math::BBox2i& g = _geometry;

            event.render->drawText(
                event.fontSystem->getGlyphs(p.label, fontInfo),
                math::Vector2i(
                    g.min.x +
                    p.margin,
                    g.min.y +
                    p.margin +
                    fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));

            math::Vector2i textSize = event.fontSystem->getSize(p.durationLabel, fontInfo);
            event.render->drawText(
                event.fontSystem->getGlyphs(p.durationLabel, fontInfo),
                math::Vector2i(
                    g.max.x -
                    p.margin -
                    textSize.x,
                    g.min.y +
                    p.margin +
                    fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));
        }

        void TimelineAudioClipItem::_drawWaveforms(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontMetrics = event.getFontMetrics(p.fontRole);
            const math::BBox2i& g = _geometry;

            const math::BBox2i bbox(
                g.min.x +
                p.margin,
                g.min.y +
                p.margin +
                fontMetrics.lineHeight +
                p.spacing,
                _sizeHint.x - p.margin * 2,
                _options.waveformHeight);
            event.render->drawRect(
                bbox,
                imaging::Color4f(0.F, 0.F, 0.F));
            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(bbox.intersect(clipRectState.getClipRect()));

            std::set<otime::RationalTime> audioDataDelete;
            for (const auto& audioData : p.audioData)
            {
                audioDataDelete.insert(audioData.first);
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

            if (p.waveformWidth > 0)
            {
                const int w = _sizeHint.x - p.margin * 2;
                for (int x = 0; x < w; x += p.waveformWidth)
                {
                    math::BBox2i bbox(
                        g.min.x +
                        p.margin +
                        x,
                        g.min.y +
                        p.margin +
                        fontMetrics.lineHeight +
                        p.spacing,
                        p.waveformWidth,
                        _options.waveformHeight);
                    if (bbox.intersects(drawRect))
                    {
                        const otime::RationalTime time = time::round(otime::RationalTime(
                            p.timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w)) : 0) *
                            p.timeRange.duration().value(),
                            p.timeRange.duration().rate()));
                        auto i = p.audioData.find(time);
                        if (i != p.audioData.end())
                        {
                            if (i->second.mesh)
                            {
                                event.render->drawMesh(
                                    *i->second.mesh,
                                    bbox.min,
                                    imaging::Color4f(1.F, 1.F, 1.F));
                            }
                            audioDataDelete.erase(time);
                        }
                        else if (p.ioInfo.audio.isValid())
                        {
                            const auto j = p.audioDataFutures.find(time);
                            if (j == p.audioDataFutures.end())
                            {
                                const otime::RationalTime mediaTime = timeline::mediaTime(
                                    time,
                                    p.track,
                                    p.clip,
                                    p.ioInfo.audioTime.duration().rate());
                                const otime::TimeRange mediaTimeRange(
                                    mediaTime,
                                    otime::RationalTime(
                                        p.ioInfo.audioTime.duration().rate(),
                                        p.ioInfo.audioTime.duration().rate()));
                                p.audioDataFutures[time].future = _data.ioManager->readAudio(p.path, mediaTimeRange);
                                p.audioDataFutures[time].size = bbox.getSize();
                            }
                        }
                    }
                }
            }

            for (auto i : audioDataDelete)
            {
                const auto j = p.audioData.find(i);
                if (j != p.audioData.end())
                {
                    p.audioData.erase(j);
                }
            }
        }
    }
}
