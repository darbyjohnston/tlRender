// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/AudioClipItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Util.h>

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct AudioClipItem::Private
        {
            file::Path path;
            std::vector<feather_tk::InMemoryFile> memoryRead;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct SizeData
            {
                int dragLength = 0;
                feather_tk::Box2I clipRect;
            };
            SizeData size;

            InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<OTIO_NS::RationalTime, WaveformRequest> waveformRequests;
        };

        void AudioClipItem::_init(
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
                feather_tk::ColorRole::AudioClip,
                "tl::timelineui::AudioClipItem",
                clip.value,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            FEATHER_TK_P();

            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());
            p.thumbnailGenerator = thumbnailGenerator;

            const std::string infoCacheKey = ThumbnailCache::getInfoKey(
                reinterpret_cast<intptr_t>(this),
                path,
                _data->options.ioOptions);
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
            FEATHER_TK_P();
            _cancelRequests();
        }

        std::shared_ptr<AudioClipItem> AudioClipItem::create(
            const std::shared_ptr<feather_tk::Context>& context,
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
            FEATHER_TK_P();
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
            FEATHER_TK_P();
            if (thumbnailsChanged)
            {
                _cancelRequests();
                _setDrawUpdate();
            }
        }

        void AudioClipItem::tickEvent(
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
                const std::string infoCacheKey = ThumbnailCache::getInfoKey(
                    reinterpret_cast<intptr_t>(this),
                    p.path,
                    _data->options.ioOptions);
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
                    const std::string cacheKey = ThumbnailCache::getWaveformKey(
                        reinterpret_cast<intptr_t>(this),
                        p.path,
                        feather_tk::Size2I(
                            _displayOptions.waveformWidth,
                            _displayOptions.waveformHeight),
                        i->second.timeRange,
                        _data->options.ioOptions);
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

        void AudioClipItem::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            FEATHER_TK_P();
            p.size.dragLength = event.style->getSizeRole(feather_tk::SizeRole::DragLength, event.displayScale);
            feather_tk::Size2I sizeHint = getSizeHint();
            if (_displayOptions.thumbnails)
            {
                sizeHint.h += _displayOptions.waveformHeight;
            }
            _setSizeHint(sizeHint);
        }

        void AudioClipItem::clipEvent(const feather_tk::Box2I& clipRect, bool clipped)
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

        void AudioClipItem::drawEvent(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_displayOptions.thumbnails)
            {
                _drawWaveforms(drawRect, event);
            }
        }

        void AudioClipItem::_drawWaveforms(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();

            const feather_tk::Box2I g = _getInsideGeometry();
            const int m = _getMargin();
            const int lineHeight = _getLineHeight();

            const feather_tk::Box2I box(
                g.min.x,
                g.min.y +
                (_displayOptions.clipInfo ? (lineHeight + m * 2) : 0),
                g.w(),
                _displayOptions.waveformHeight);
            event.render->drawRect(
                box,
                feather_tk::Color4F(0.F, 0.F, 0.F));
            const feather_tk::ClipRectEnabledState clipRectEnabledState(event.render);
            const feather_tk::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(feather_tk::intersect(box, clipRectState.getClipRect()));

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
                        _data->options.ioOptions);
                }
            }

            if (_displayOptions.waveformWidth > 0 && p.ioInfo)
            {
                const int w = g.w();
                const bool enabled = isEnabled();
                for (int x = 0; x < w; x += _displayOptions.waveformWidth)
                {
                    const feather_tk::Box2I box(
                        g.min.x +
                        x,
                        g.min.y +
                        (_displayOptions.clipInfo ? (lineHeight + m * 2) : 0),
                        _displayOptions.waveformWidth,
                        _displayOptions.waveformHeight);
                    if (feather_tk::intersects(box, clipRect))
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
                        OTIO_NS::TimeRange trimmedRange = _trimmedRange;
                        if (_data->options.compat &&
                            trimmedRange.start_time() < p.ioInfo->audioTime.start_time())
                        {
                            //! \bug If the trimmed range is less than the media time,
                            //! assume the media time is wrong (e.g., ALab trailer) and
                            //! compensate for it.
                            trimmedRange = OTIO_NS::TimeRange(
                                p.ioInfo->audioTime.start_time() + trimmedRange.start_time(),
                                trimmedRange.duration());
                        }
                        const OTIO_NS::TimeRange mediaRange = timeline::toAudioMediaTime(
                            OTIO_NS::TimeRange::range_from_start_end_time(time, time2),
                            _timeRange,
                            trimmedRange,
                            p.ioInfo->audio.sampleRate);

                        const std::string cacheKey = ThumbnailCache::getWaveformKey(
                            reinterpret_cast<intptr_t>(this),
                            p.path,
                            box.size(),
                            mediaRange,
                            _data->options.ioOptions);
                        const auto i = _data->waveforms.find(cacheKey);
                        if (i != _data->waveforms.end())
                        {
                            if (i->second)
                            {
                                event.render->drawMesh(
                                    *i->second,
                                    enabled ?
                                        feather_tk::Color4F(1.F, 1.F, 1.F) :
                                        feather_tk::Color4F(.5F, .5F, .5F),
                                    feather_tk::V2F(box.min.x, box.min.y));
                            }
                        }
                        else if (p.ioInfo && p.ioInfo->audio.isValid())
                        {
                            const auto j = p.waveformRequests.find(mediaRange.start_time());
                            if (j == p.waveformRequests.end())
                            {
                                p.waveformRequests[mediaRange.start_time()] = p.thumbnailGenerator->getWaveform(
                                    reinterpret_cast<intptr_t>(this),
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
            FEATHER_TK_P();
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
