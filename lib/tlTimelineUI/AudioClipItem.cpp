// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/AudioClipItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Util.h>

#include <dtk/ui/DrawUtil.h>
#include <tlIO/Cache.h>

#include <dtk/core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct AudioClipItem::Private
        {
            file::Path path;
            std::vector<dtk::InMemoryFile> memoryRead;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct SizeData
            {
                int dragLength = 0;
                dtk::Box2I clipRect;
            };
            SizeData size;

            InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<OTIO_NS::RationalTime, WaveformRequest> waveformRequests;
        };

        void AudioClipItem::_init(
            const std::shared_ptr<dtk::Context>& context,
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
                dtk::ColorRole::AudioClip,
                "tl::timelineui::AudioClipItem",
                clip.value,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            DTK_P();

            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());
            p.thumbnailGenerator = thumbnailGenerator;

            const std::string infoCacheKey = io::getInfoCacheKey(path, _data->options.ioOptions);
            const auto i = itemData->info.find(infoCacheKey);
            if (i != itemData->info.end())
            {
                p.ioInfo = i->second;
            }
        }

        AudioClipItem::AudioClipItem() :
            _p(new Private)
        {}

        AudioClipItem::~AudioClipItem()
        {
            DTK_P();
            _cancelRequests();
        }

        std::shared_ptr<AudioClipItem> AudioClipItem::create(
            const std::shared_ptr<dtk::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>& clip,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ThumbnailGenerator> thumbnailGenerator,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioClipItem>(new AudioClipItem);
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

        void AudioClipItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IBasicItem::setScale(value);
            DTK_P();
            if (changed)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void AudioClipItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _displayOptions.thumbnails ||
                value.waveformWidth != _displayOptions.waveformWidth ||
                value.waveformHeight != _displayOptions.waveformHeight ||
                value.waveformPrim != _displayOptions.waveformPrim;
            IBasicItem::setDisplayOptions(value);
            DTK_P();
            if (thumbnailsChanged)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void AudioClipItem::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const dtk::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            DTK_P();

            // Check if the I/O information is finished.
            if (p.infoRequest.future.valid() &&
                p.infoRequest.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                p.ioInfo = std::make_shared<io::Info>(p.infoRequest.future.get());
                const std::string infoCacheKey = io::getInfoCacheKey(p.path, _data->options.ioOptions);
                _data->info[infoCacheKey] = p.ioInfo;
                _setSizeUpdate();
                _setDrawUpdate();
            }

            // Check if any audio waveforms are finished.
            auto i = p.waveformRequests.begin();
            while (i != p.waveformRequests.end())
            {
                if (i->second.future.valid() &&
                    i->second.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto mesh = i->second.future.get();
                    const std::string cacheKey = io::getAudioCacheKey(
                        p.path,
                        i->second.timeRange,
                        _data->options.ioOptions,
                        {});
                    _data->waveforms[cacheKey] = mesh;
                    i = p.waveformRequests.erase(i);
                    _setDrawUpdate();
                }
                else
                {
                    ++i;
                }
            }
        }

        void AudioClipItem::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            DTK_P();
            p.size.dragLength = event.style->getSizeRole(dtk::SizeRole::DragLength, event.displayScale);
            dtk::Size2I sizeHint = getSizeHint();
            if (_displayOptions.thumbnails)
            {
                sizeHint.h += _displayOptions.waveformHeight;
            }
            _setSizeHint(sizeHint);
        }

        void AudioClipItem::clipEvent(const dtk::Box2I& clipRect, bool clipped)
        {
            IBasicItem::clipEvent(clipRect, clipped);
            DTK_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void AudioClipItem::drawEvent(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_displayOptions.thumbnails)
            {
                _drawWaveforms(drawRect, event);
            }
        }

        void AudioClipItem::_drawWaveforms(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            DTK_P();

            const dtk::Box2I g = _getInsideGeometry();
            const int m = _getMargin();
            const int lineHeight = _getLineHeight();

            const dtk::Box2I box(
                g.min.x,
                g.min.y +
                (_displayOptions.clipInfo ? (lineHeight + m * 2) : 0),
                g.w(),
                _displayOptions.waveformHeight);
            event.render->drawRect(
                box,
                dtk::Color4F(0.F, 0.F, 0.F));
            const dtk::ClipRectEnabledState clipRectEnabledState(event.render);
            const dtk::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(dtk::intersect(box, clipRectState.getClipRect()));

            const dtk::Box2I clipRect = _getClipRect(
                drawRect,
                _displayOptions.clipRectScale);
            if (dtk::intersects(g, clipRect))
            {
                if (!p.ioInfo && !p.infoRequest.future.valid())
                {
                    p.infoRequest = p.thumbnailGenerator->getInfo(
                        p.path,
                        p.memoryRead,
                        _data->options.ioOptions);
                }
            }

            if (_displayOptions.waveformWidth > 0 && p.ioInfo)
            {
                const int w = g.w();
                const bool enabled = isEnabled();
                for (int x = 0; x < w; x += _displayOptions.waveformWidth)
                {
                    const dtk::Box2I box(
                        g.min.x +
                        x,
                        g.min.y +
                        (_displayOptions.clipInfo ? (lineHeight + m * 2) : 0),
                        _displayOptions.waveformWidth,
                        _displayOptions.waveformHeight);
                    if (dtk::intersects(box, clipRect))
                    {
                        const OTIO_NS::RationalTime time = OTIO_NS::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? (x / static_cast<double>(w)) : 0) *
                            _timeRange.duration().rescaled_to(_timeRange.start_time()).value(),
                            _timeRange.start_time().rate()).
                            round();
                        const OTIO_NS::RationalTime time2 = OTIO_NS::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? ((x + _displayOptions.waveformWidth) / static_cast<double>(w)) : 0) *
                            _timeRange.duration().rescaled_to(_timeRange.start_time()).value(),
                            _timeRange.start_time().rate()).
                            round();
                        const OTIO_NS::TimeRange mediaRange = timeline::toAudioMediaTime(
                            OTIO_NS::TimeRange::range_from_start_end_time(time, time2),
                            _timeRange,
                            _trimmedRange,
                            p.ioInfo->audio.sampleRate);

                        const std::string cacheKey = io::getAudioCacheKey(
                            p.path,
                            mediaRange,
                            _data->options.ioOptions,
                            {});
                        const auto i = _data->waveforms.find(cacheKey);
                        if (i != _data->waveforms.end())
                        {
                            if (i->second)
                            {
                                event.render->drawMesh(
                                    *i->second,
                                    enabled ?
                                        dtk::Color4F(1.F, 1.F, 1.F) :
                                        dtk::Color4F(.5F, .5F, .5F),
                                    dtk::V2F(box.min.x, box.min.y));
                            }
                        }
                        else if (p.ioInfo && p.ioInfo->audio.isValid())
                        {
                            const auto j = p.waveformRequests.find(mediaRange.start_time());
                            if (j == p.waveformRequests.end())
                            {
                                p.waveformRequests[mediaRange.start_time()] = p.thumbnailGenerator->getWaveform(
                                    p.path,
                                    p.memoryRead,
                                    box.size(),
                                    mediaRange,
                                    _data->options.ioOptions);
                            }
                        }
                    }
                }
            }
        }

        void AudioClipItem::_cancelRequests()
        {
            DTK_P();
            std::vector<uint64_t> ids;
            if (p.infoRequest.future.valid())
            {
                ids.push_back(p.infoRequest.id);
                p.infoRequest = InfoRequest();
            }
            for (const auto& i : p.waveformRequests)
            {
                ids.push_back(i.second.id);
            }
            p.waveformRequests.clear();
            p.thumbnailGenerator->cancelRequests(ids);
        }
    }
}
