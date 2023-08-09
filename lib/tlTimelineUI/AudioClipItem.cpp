// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/AudioClipItem.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/EventLoop.h>

#include <tlTimeline/RenderUtil.h>
#include <tlTimeline/Util.h>

#include <tlCore/AudioConvert.h>
#include <tlCore/Mesh.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        AudioDragAndDropData::AudioDragAndDropData(const std::shared_ptr<AudioClipItem>& item) :
            _item(item)
        {}

        AudioDragAndDropData::~AudioDragAndDropData()
        {}

        const std::shared_ptr<AudioClipItem>& AudioDragAndDropData::getItem() const
        {
            return _item;
        }

        struct AudioClipItem::Private
        {
            otio::SerializableObject::Retainer<otio::Clip> clip;
            file::Path path;
            std::vector<file::MemoryRead> memoryRead;
            otime::TimeRange availableRange = time::invalidTimeRange;
            bool ioInfoInit = true;
            io::Info ioInfo;

            struct SizeData
            {
                int dragLength = 0;
                math::Box2i clipRect;
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
                std::future<std::shared_ptr<image::Image> > imageFuture;
                std::shared_ptr<image::Image> image;
                std::chrono::steady_clock::time_point time;
            };
            std::map<otime::RationalTime, AudioData> audioData;
            std::shared_ptr<observer::ValueObserver<bool> > cancelObserver;
        };

        void AudioClipItem::_init(
            const otio::SerializableObject::Retainer<otio::Clip>& clip,
            const otime::TimeRange& timeRange,
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
                getMarkers(clip),
                "tl::timelineui::AudioClipItem",
                timeRange,
                itemData,
                context,
                parent);
            TLRENDER_P();

            _mouse.hoverEnabled = true;
            _mouse.pressEnabled = true;

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
            const otime::TimeRange& timeRange,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioClipItem>(new AudioClipItem);
            out->_init(clip, timeRange, itemData, context, parent);
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
                value.waveformWidth != _options.waveformWidth ||
                value.waveformHeight != _options.waveformHeight ||
                value.waveformPrim != _options.waveformPrim;
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
                            const math::Box2i box(
                                math::Vector2i(
                                    x,
                                    h2 - h2 * max),
                                math::Vector2i(
                                    x + 1,
                                    h2 - h2 * min));
                            if (box.isValid())
                            {
                                const size_t j = 1 + out->v.size();
                                out->v.push_back(math::Vector2f(box.x(), box.y()));
                                out->v.push_back(math::Vector2f(box.x() + box.w(), box.y()));
                                out->v.push_back(math::Vector2f(box.x() + box.w(), box.y() + box.h()));
                                out->v.push_back(math::Vector2f(box.x(), box.y() + box.h()));
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

            std::shared_ptr<image::Image> audioImage(
                const std::shared_ptr<audio::Audio>& audio,
                const math::Vector2i& size)
            {
                auto out = image::Image::create(size.x, size.y, image::PixelType::L_U8);
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
                            uint8_t* p = out->getData() + x;
                            for (int y = 0; y < size.y; ++y)
                            {
                                const float v = y / static_cast<float>(size.y - 1) * 2.F - 1.F;
                                *p = (v > min && v < max) ? 255 : 0;
                                p += size.x;
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
                        switch (_options.waveformPrim)
                        {
                        case WaveformPrim::Mesh:
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
                            break;
                        case WaveformPrim::Image:
                            audioData.imageFuture = std::async(
                                std::launch::async,
                                [audio, size]
                                {
                                    auto convert = audio::AudioConvert::create(
                                        audio.audio->getInfo(),
                                        audio::Info(1, audio::DataType::F32, audio.audio->getSampleRate()));
                                const auto convertedAudio = convert->convert(audio.audio);
                                return audioImage(convertedAudio, size);
                                });
                            break;
                        default: break;
                        }
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
                switch (_options.waveformPrim)
                {
                case WaveformPrim::Mesh:
                    if (audioData.second.meshFuture.valid() &&
                        audioData.second.meshFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        const auto mesh = audioData.second.meshFuture.get();
                        audioData.second.mesh = mesh;
                        audioData.second.time = now;
                        _updates |= ui::Update::Draw;
                    }
                    break;
                case WaveformPrim::Image:
                    if (audioData.second.imageFuture.valid() &&
                        audioData.second.imageFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        const auto image = audioData.second.imageFuture.get();
                        audioData.second.image = image;
                        audioData.second.time = now;
                        _updates |= ui::Update::Draw;
                    }
                    break;
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
            _data.ioManager->cancelRequests();
            _updates |= ui::Update::Draw;
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

        void AudioClipItem::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            if (_mouse.press)
            {
                const float length = math::length(event.pos - _mouse.pressPos);
                if (length > p.size.dragLength)
                {
                    if (auto eventLoop = getEventLoop().lock())
                    {
                        event.dndData = std::make_shared<AudioDragAndDropData>(
                            std::dynamic_pointer_cast<AudioClipItem>(shared_from_this()));
                        event.dndCursor = eventLoop->screenshot(shared_from_this());
                        event.dndCursorHotspot = _mouse.pos - _geometry.min;
                    }
                }
            }
        }

        void AudioClipItem::_drawWaveforms(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i g = _getInsideGeometry();
            const int m = _getMargin();
            const auto now = std::chrono::steady_clock::now();

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

            std::set<otime::RationalTime> audioDataDelete;
            for (const auto& audioData : p.audioData)
            {
                audioDataDelete.insert(audioData.first);
            }

            const math::Box2i clipRect = _getClipRect(
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

            if (_options.waveformWidth > 0)
            {
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
                        auto i = p.audioData.find(time);
                        if (i != p.audioData.end())
                        {
                            switch (_options.waveformPrim)
                            {
                            case WaveformPrim::Mesh:
                                if (i->second.mesh)
                                {
                                    const std::chrono::duration<float> diff = now - i->second.time;
                                    const float a = std::min(diff.count() / _options.thumbnailFade, 1.F);
                                    event.render->drawMesh(
                                        *i->second.mesh,
                                        box.min,
                                        image::Color4f(1.F, 1.F, 1.F, a));
                                }
                                break;
                            case WaveformPrim::Image:
                                if (i->second.image)
                                {
                                    const std::chrono::duration<float> diff = now - i->second.time;
                                    const float a = std::min(diff.count() / _options.thumbnailFade, 1.F);
                                    event.render->drawImage(
                                        (i->second).image,
                                        box,
                                        image::Color4f(1.F, 1.F, 1.F, a));
                                }
                                break;
                            }
                            audioDataDelete.erase(time);
                        }
                        else if (p.ioInfo.audio.isValid())
                        {
                            const auto j = p.audioDataFutures.find(time);
                            if (j == p.audioDataFutures.end())
                            {
                                const otime::RationalTime time2 = time::round(otime::RationalTime(
                                    _timeRange.start_time().value() +
                                    (w > 0 ? ((x + _options.waveformWidth) / static_cast<double>(w)) : 0) *
                                    _timeRange.duration().value(),
                                    _timeRange.duration().rate()));
                                const otime::TimeRange mediaRange = timeline::toAudioMediaTime(
                                    otime::TimeRange::range_from_start_end_time(time, time2),
                                    p.clip,
                                    p.ioInfo);
                                p.audioDataFutures[time].future = _data.ioManager->readAudio(
                                    p.path,
                                    p.memoryRead,
                                    p.availableRange.start_time(),
                                    mediaRange);
                                p.audioDataFutures[time].size = box.getSize();
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
