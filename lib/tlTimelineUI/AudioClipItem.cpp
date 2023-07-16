// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/AudioClipItem.h>

#include <tlUI/DrawUtil.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

#include <tlCore/AudioConvert.h>
#include <tlCore/Mesh.h>

#include <opentimelineio/track.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        struct AudioClipItem::Private
        {
            otio::SerializableObject::Retainer<otio::Clip> clip;
            otio::SerializableObject::Retainer<otio::Track> track;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            otime::TimeRange timeRange = time::invalidTimeRange;
            otime::TimeRange availableRange = time::invalidTimeRange;
            bool ioInfoInit = true;
            io::Info ioInfo;

            struct SizeData
            {
                int waveformWidth = 0;
                math::BBox2i clipRect;
            };
            SizeData size;

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
                std::chrono::steady_clock::time_point time;
            };
            std::map<otime::RationalTime, AudioData> audioData;
            std::shared_ptr<observer::ValueObserver<bool> > cancelObserver;
        };

        void AudioClipItem::_init(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto rangeOpt = clip->trimmed_range_in_parent();
            const auto path = timeline::getPath(
                clip->media_reference(),
                itemData.directory,
                itemData.options.pathOptions);
            IBasicItem::_init(
                rangeOpt.has_value() ? rangeOpt.value() : time::invalidTimeRange,
                !clip->name().empty() ? clip->name() : path.get(-1, false),
                ColorRole::AudioClip,
                getMarkers(clip),
                "tl::timelineui::AudioClipItem",
                itemData,
                context,
                parent);
            TLRENDER_P();

            p.clip = clip;
            p.track = dynamic_cast<otio::Track*>(clip->parent());

            p.path = path;
            p.memoryRead = timeline::getMemoryRead(clip->media_reference());

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

            p.cancelObserver = observer::ValueObserver<bool>::create(
                _data.ioManager->observeCancelRequests(),
                [this](bool)
                {
                    _p->audioDataFutures.clear();
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

        void AudioClipItem::setScale(double value)
        {
            const bool changed = value != _scale;
            IBasicItem::setScale(value);
            TLRENDER_P();
            if (changed)
            {
                _data.ioManager->cancelRequests();
                p.audioData.clear();
                _updates |= ui::Update::Draw;
            }
        }

        void AudioClipItem::setOptions(const ItemOptions& value)
        {
            const bool thumbnailsChanged =
                value.thumbnails != _options.thumbnails ||
                value.waveformHeight != _options.waveformHeight;
            IBasicItem::setOptions(value);
            TLRENDER_P();
            if (thumbnailsChanged)
            {
                _data.ioManager->cancelRequests();
                p.audioData.clear();
                _updates |= ui::Update::Draw;
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
                    default: break;
                    }
                }
                return out;
            }
        }

        void AudioClipItem::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
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
                }
                else
                {
                    ++i;
                }
            }

            const auto now = std::chrono::steady_clock::now();
            for (auto& audioData : p.audioData)
            {
                if (audioData.second.meshFuture.valid() &&
                    audioData.second.meshFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    const auto mesh = audioData.second.meshFuture.get();
                    audioData.second.mesh = mesh;
                    audioData.second.time = now;
                    _updates |= ui::Update::Draw;
                }
                const std::chrono::duration<float> diff = now - audioData.second.time;
                if (diff.count() < _options.thumbnailFade)
                {
                    _updates |= ui::Update::Draw;
                }
            }
        }

        void AudioClipItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IBasicItem::sizeHintEvent(event);
            TLRENDER_P();
            const int waveformWidth = _options.thumbnails ?
                (otime::RationalTime(1.0, 1.0).value() * _scale) :
                0;
            if (waveformWidth != p.size.waveformWidth)
            {
                p.size.waveformWidth = waveformWidth;
                _data.ioManager->cancelRequests();
                p.audioData.clear();
                _updates |= ui::Update::Draw;
            }
            if (_options.thumbnails)
            {
                _sizeHint.y += _options.waveformHeight;
            }
        }

        void AudioClipItem::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            IBasicItem::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipRect == p.size.clipRect)
                return;
            p.size.clipRect = clipRect;
            _data.ioManager->cancelRequests();
            _updates |= ui::Update::Draw;
        }

        void AudioClipItem::drawEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IBasicItem::drawEvent(drawRect, event);
            if (_options.thumbnails)
            {
                _drawWaveforms(drawRect, event);
            }
        }

        void AudioClipItem::_drawWaveforms(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::BBox2i g = _getInsideGeometry();
            const int m = _getMargin();
            const auto now = std::chrono::steady_clock::now();

            const math::BBox2i bbox(
                g.min.x,
                g.min.y +
                _getLineHeight() + m * 2,
                g.w(),
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

            if (p.size.waveformWidth > 0)
            {
                const int w = _sizeHint.x;
                for (int x = 0; x < w; x += p.size.waveformWidth)
                {
                    math::BBox2i bbox(
                        g.min.x +
                        x,
                        g.min.y +
                        _getLineHeight() + m * 2,
                        p.size.waveformWidth,
                        _options.waveformHeight);
                    if (bbox.intersects(clipRect))
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
                                const std::chrono::duration<float> diff = now - i->second.time;
                                const float a = std::min(diff.count() / _options.thumbnailFade, 1.F);
                                event.render->drawMesh(
                                    *i->second.mesh,
                                    bbox.min,
                                    imaging::Color4f(1.F, 1.F, 1.F, a));
                            }
                            audioDataDelete.erase(time);
                        }
                        else if (p.ioInfo.audio.isValid())
                        {
                            const auto j = p.audioDataFutures.find(time);
                            if (j == p.audioDataFutures.end())
                            {
                                const otime::TimeRange mediaRange = timeline::toAudioMediaTime(
                                    otime::TimeRange(time, otime::RationalTime(time.rate(), time.rate())),
                                    p.track,
                                    p.clip,
                                    p.ioInfo);
                                p.audioDataFutures[time].future = _data.ioManager->readAudio(
                                    p.path,
                                    p.memoryRead,
                                    p.availableRange.start_time(),
                                    mediaRange);
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
