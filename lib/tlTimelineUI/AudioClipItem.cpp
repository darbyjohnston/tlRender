// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/AudioClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

namespace tl
{
    namespace timelineui
    {
        struct AudioClipItem::Private
        {
            otio::SerializableObject::Retainer<otio::Clip> clip;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            otime::TimeRange availableRange = time::invalidTimeRange;

            struct SizeData
            {
                int dragLength = 0;
                math::Box2i clipRect;
            };
            SizeData size;

            std::future<io::Info> infoFuture;
            std::unique_ptr<io::Info> ioInfo;
            std::map<otime::RationalTime, std::future<std::shared_ptr<geom::TriangleMesh2> > > waveformFutures;
            struct Waveform
            {
                std::shared_ptr<geom::TriangleMesh2> mesh;
                std::chrono::steady_clock::time_point time;
            };
            std::map<otime::RationalTime, Waveform> waveforms;
            std::shared_ptr<observer::ValueObserver<bool> > cancelObserver;
        };

        void AudioClipItem::_init(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto path = timeline::getPath(
                clip->media_reference(),
                itemData.directory,
                itemData.options.pathOptions);
            IBasicItem::_init(
                !clip->name().empty() ? clip->name() : path.get(-1, false),
                ui::ColorRole::AudioClip,
                "tl::timelineui::AudioClipItem",
                clip.value,
                itemData,
                context,
                parent);
            TLRENDER_P();

            p.clip = clip;

            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());

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

            p.cancelObserver = observer::ValueObserver<bool>::create(
                _data.ioManager->observeCancelRequests(),
                [this](bool)
                {
                    _p->infoFuture = std::future<io::Info>();
                    _p->waveformFutures.clear();
                });
        }

        AudioClipItem::AudioClipItem() :
            _p(new Private)
        {}

        AudioClipItem::~AudioClipItem()
        {}

        std::shared_ptr<AudioClipItem> AudioClipItem::create(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioClipItem>(new AudioClipItem);
            out->_init(clip, itemData, context, parent);
            return out;
        }

        const otio::SerializableObject::Retainer<otio::Clip>& AudioClipItem::getClip() const
        {
            return _p->clip;
        }

        void AudioClipItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IBasicItem::setScale(value);
            TLRENDER_P();
            if (changed)
            {
                p.waveforms.clear();
                _data.ioManager->cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void AudioClipItem::setOptions(const ItemOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _options.thumbnails ||
                value.waveformWidth != _options.waveformWidth ||
                value.waveformHeight != _options.waveformHeight ||
                value.waveformPrim != _options.waveformPrim;
            IBasicItem::setOptions(value);
            TLRENDER_P();
            if (thumbnailsChanged)
            {
                p.waveforms.clear();
                _data.ioManager->cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void AudioClipItem::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();

            // Check if the I/O information is finished.
            if (p.infoFuture.valid() &&
                p.infoFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.ioInfo = std::make_unique<io::Info>(p.infoFuture.get());
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }

            // Check if any audio waveforms are finished.
            const auto now = std::chrono::steady_clock::now();
            auto i = p.waveformFutures.begin();
            while (i != p.waveformFutures.end())
            {
                if (i->second.valid() &&
                    i->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto mesh = i->second.get();
                    p.waveforms[i->first] = { mesh, now };
                    i = p.waveformFutures.erase(i);
                    _updates |= ui::Update::Draw;
                }
                else
                {
                    ++i;
                }
            }

            // Check if any waveforms need to be redrawn.
            for (const auto& waveform : p.waveforms)
            {
                const std::chrono::duration<float> diff = now - waveform.second.time;
                if (diff.count() <= _options.thumbnailFade)
                {
                    _updates |= ui::Update::Draw;
                }
            }
        }

        void AudioClipItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            TLRENDER_P();
            p.size.dragLength = event.style->getSizeRole(ui::SizeRole::DragLength, event.displayScale);
            if (_options.thumbnails)
            {
                _sizeHint.y += _options.waveformHeight;
            }
        }

        void AudioClipItem::clipEvent(
            const math::Box2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            IBasicItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                p.waveforms.clear();
                _data.ioManager->cancelRequests();
                _updates |= ui::Update::Draw;
            }
        }

        void AudioClipItem::drawEvent(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_options.thumbnails)
            {
                _drawWaveforms(drawRect, event);
            }
        }

        void AudioClipItem::_drawWaveforms(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i g = _getInsideGeometry();
            const int m = _getMargin();

            const math::Box2i box(
                g.min.x,
                g.min.y +
                _getLineHeight() + m * 2,
                g.w(),
                _options.waveformHeight);
            event.render->drawRect(
                box,
                image::Color4f(0.F, 0.F, 0.F));
            const timeline::ClipRectEnabledState clipRectEnabledState(event.render);
            const timeline::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(box.intersect(clipRectState.getClipRect()));

            std::set<otime::RationalTime> waveformsDelete;
            for (const auto& waveform : p.waveforms)
            {
                waveformsDelete.insert(waveform.first);
            }

            const math::Box2i clipRect = _getClipRect(
                drawRect,
                _options.clipRectScale);
            if (g.intersects(clipRect))
            {
                if (!p.ioInfo && !p.infoFuture.valid())
                {
                    p.infoFuture = _data.ioManager->requestInfo(
                        p.path,
                        p.memoryRead,
                        p.availableRange.start_time());
                }
            }

            if (_options.waveformWidth > 0)
            {
                const auto now = std::chrono::steady_clock::now();
                const int w = _sizeHint.x;
                for (int x = 0; x < w; x += _options.waveformWidth)
                {
                    const math::Box2i box(
                        g.min.x +
                        x,
                        g.min.y +
                        _getLineHeight() + m * 2,
                        _options.waveformWidth,
                        _options.waveformHeight);
                    if (box.intersects(clipRect))
                    {
                        const otime::RationalTime time = time::round(otime::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w)) : 0) *
                            _timeRange.duration().value(),
                            _timeRange.duration().rate()));
                        auto i = p.waveforms.find(time);
                        if (i != p.waveforms.end())
                        {
                            if (i->second.mesh)
                            {
                                const std::chrono::duration<float> diff = now - i->second.time;
                                float a = 1.F;
                                if (_options.thumbnailFade > 0.F)
                                {
                                    a = std::min(diff.count() / _options.thumbnailFade, 1.F);
                                }
                                event.render->drawMesh(
                                    *i->second.mesh,
                                    box.min,
                                    image::Color4f(1.F, 1.F, 1.F, a));
                            }
                            waveformsDelete.erase(time);
                        }
                        else if (p.ioInfo && p.ioInfo->audio.isValid())
                        {
                            const auto j = p.waveformFutures.find(time);
                            if (j == p.waveformFutures.end())
                            {
                                const otime::RationalTime time2 = time::round(otime::RationalTime(
                                    _timeRange.start_time().value() +
                                    (w > 0 ? ((x + _options.waveformWidth) / static_cast<double>(w)) : 0) *
                                    _timeRange.duration().value(),
                                    _timeRange.duration().rate()));
                                const otime::TimeRange mediaRange = timeline::toAudioMediaTime(
                                    otime::TimeRange::range_from_start_end_time(time, time2),
                                    p.clip,
                                    p.ioInfo->audio.sampleRate);
                                p.waveformFutures[time] = _data.ioManager->requestAudio(
                                    box.getSize(),
                                    p.path,
                                    p.memoryRead,
                                    p.availableRange.start_time(),
                                    mediaRange);
                            }
                        }
                    }
                }
            }

            for (auto i : waveformsDelete)
            {
                const auto j = p.waveforms.find(i);
                if (j != p.waveforms.end())
                {
                    p.waveforms.erase(j);
                }
            }
        }
    }
}
