// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineUI/AudioClipItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Util.h>

#include <ftk/UI/DrawUtil.h>
#include <ftk/Core/RenderUtil.h>

namespace tl
{
    namespace timelineui
    {
        struct AudioClipItem::Private
        {
            file::Path path;
            std::vector<ftk::InMemoryFile> memoryRead;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct SizeData
            {
                int dragLength = 0;
                ftk::Box2I clipRect;
            };
            SizeData size;

            InfoRequest infoRequest;
            std::shared_ptr<io::Info> ioInfo;
            std::map<OTIO_NS::RationalTime, WaveformRequest> waveformRequests;
        };

        void AudioClipItem::_init(
            const std::shared_ptr<ftk::Context>& context,
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
                ftk::ColorRole::AudioClip,
                "tl::timelineui::AudioClipItem",
                clip.value,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            FTK_P();

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
            FTK_P();
            _cancelRequests();
        }

        std::shared_ptr<AudioClipItem> AudioClipItem::create(
            const std::shared_ptr<ftk::Context>& context,
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
            FTK_P();
            if (changed)
            {
                _cancelRequests();
                setDrawUpdate();
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
            FTK_P();
            if (thumbnailsChanged)
            {
                _cancelRequests();
                setDrawUpdate();
            }
        }

        void AudioClipItem::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const ftk::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            FTK_P();

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
                setSizeUpdate();
                setDrawUpdate();
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
                        ftk::Size2I(
                            _displayOptions.waveformWidth,
                            _displayOptions.waveformHeight),
                        i->second.timeRange,
                        _data->options.ioOptions);
                    _data->waveforms[cacheKey] = mesh;
                    i = p.waveformRequests.erase(i);
                    setDrawUpdate();
                }
                else
                {
                    ++i;
                }
            }
        }

        void AudioClipItem::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            FTK_P();
            p.size.dragLength = event.style->getSizeRole(ftk::SizeRole::DragLength, event.displayScale);
            ftk::Size2I sizeHint = getSizeHint();
            if (_displayOptions.thumbnails)
            {
                sizeHint.h += _displayOptions.waveformHeight;
            }
            _setSizeHint(sizeHint);
        }

        void AudioClipItem::clipEvent(const ftk::Box2I& clipRect, bool clipped)
        {
            IBasicItem::clipEvent(clipRect, clipped);
            FTK_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            if (clipped)
            {
                _cancelRequests();
                setDrawUpdate();
            }
        }

        void AudioClipItem::drawEvent(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_displayOptions.thumbnails)
            {
                _drawWaveforms(drawRect, event);
            }
        }

        void AudioClipItem::_drawWaveforms(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();

            const ftk::Box2I g = _getInsideGeometry();
            const int m = _getMargin();
            const int lineHeight = _getLineHeight();

            const ftk::Box2I box(
                g.min.x,
                g.min.y +
                (!_displayOptions.minimize ? (lineHeight + m * 2) : 0),
                g.w(),
                _displayOptions.waveformHeight);
            event.render->drawRect(
                box,
                ftk::Color4F(0.F, 0.F, 0.F));
            const ftk::ClipRectEnabledState clipRectEnabledState(event.render);
            const ftk::ClipRectState clipRectState(event.render);
            event.render->setClipRectEnabled(true);
            event.render->setClipRect(ftk::intersect(box, clipRectState.getClipRect()));

            const ftk::Box2I clipRect = _getClipRect(
                drawRect,
                _displayOptions.clipRectScale);
            if (ftk::intersects(g, clipRect))
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
                    const ftk::Box2I box(
                        g.min.x +
                        x,
                        g.min.y +
                        (!_displayOptions.minimize ? (lineHeight + m * 2) : 0),
                        _displayOptions.waveformWidth,
                        _displayOptions.waveformHeight);
                    if (ftk::intersects(box, clipRect))
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
                                        ftk::Color4F(1.F, 1.F, 1.F) :
                                        ftk::Color4F(.5F, .5F, .5F),
                                    ftk::V2F(box.min.x, box.min.y));
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
            FTK_P();
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
