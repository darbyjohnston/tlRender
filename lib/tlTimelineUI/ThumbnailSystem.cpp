// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimelineGL/Render.h>

#include <tlTimeline/Timeline.h>

#include <tlIO/System.h>

#include <tlCore/AudioResample.h>

#include <feather-tk/gl/GL.h>
#include <feather-tk/gl/Window.h>
#include <feather-tk/gl/OffscreenBuffer.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/LRUCache.h>
#include <feather-tk/core/String.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        namespace
        {
            const size_t ioCacheMax = 16;
        }

        struct ThumbnailCache::Private
        {
            size_t max = 1000;
            feather_tk::LRUCache<std::string, io::Info> info;
            feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::Image> > thumbnails;
            feather_tk::LRUCache<std::string, std::shared_ptr<feather_tk::TriMesh2F> > waveforms;
            std::mutex mutex;
        };

        void ThumbnailCache::_init(const std::shared_ptr<feather_tk::Context>& context)
        {
            _maxUpdate();
        }

        ThumbnailCache::ThumbnailCache() :
            _p(new Private)
        {}

        ThumbnailCache::~ThumbnailCache()
        {}

        std::shared_ptr<ThumbnailCache> ThumbnailCache::create(
            const std::shared_ptr<feather_tk::Context>& context)
        {
            auto out = std::shared_ptr<ThumbnailCache>(new ThumbnailCache);
            out->_init(context);
            return out;
        }

        size_t ThumbnailCache::getMax() const
        {
            return _p->max;
        }

        void ThumbnailCache::setMax(size_t value)
        {
            FEATHER_TK_P();
            if (value == p.max)
                return;
            p.max = value;
            _maxUpdate();
        }

        size_t ThumbnailCache::getSize() const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.getSize() + p.thumbnails.getSize() + p.waveforms.getSize();
        }

        float ThumbnailCache::getPercentage() const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return
                (p.info.getSize() + p.thumbnails.getSize() + p.waveforms.getSize()) /
                static_cast<float>(p.info.getMax() + p.thumbnails.getMax() + p.waveforms.getMax()) * 100.F;
        }

        std::string ThumbnailCache::getInfoKey(
            const file::Path& path,
            const io::Options& options)
        {
            std::vector<std::string> s;
            s.push_back(path.get());
            for (const auto& i : options)
            {
                s.push_back(feather_tk::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return feather_tk::join(s, ';');
        }

        void ThumbnailCache::addInfo(const std::string& key, const io::Info& info)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.add(key, info);
        }

        bool ThumbnailCache::containsInfo(const std::string& key)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.contains(key);
        }

        bool ThumbnailCache::getInfo(const std::string& key, io::Info& info) const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.info.get(key, info);
        }

        std::string ThumbnailCache::getThumbnailKey(
            int height,
            const file::Path& path,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            std::vector<std::string> s;
            s.push_back(feather_tk::Format("{0}").arg(height));
            s.push_back(path.get());
            s.push_back(feather_tk::Format("{0}").arg(time));
            for (const auto& i : options)
            {
                s.push_back(feather_tk::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return feather_tk::join(s, ';');
        }

        void ThumbnailCache::addThumbnail(
            const std::string& key,
            const std::shared_ptr<feather_tk::Image>& thumbnail)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.thumbnails.add(key, thumbnail);
        }

        bool ThumbnailCache::containsThumbnail(const std::string& key)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.thumbnails.contains(key);
        }

        bool ThumbnailCache::getThumbnail(
            const std::string& key,
            std::shared_ptr<feather_tk::Image>& thumbnail) const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.thumbnails.get(key, thumbnail);
        }

        std::string ThumbnailCache::getWaveformKey(
            const feather_tk::Size2I& size,
            const file::Path& path,
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            std::vector<std::string> s;
            s.push_back(feather_tk::Format("{0}").arg(size));
            s.push_back(path.get());
            s.push_back(feather_tk::Format("{0}").arg(timeRange));
            for (const auto& i : options)
            {
                s.push_back(feather_tk::Format("{0}:{1}").arg(i.first).arg(i.second));
            }
            return feather_tk::join(s, ';');
        }

        void ThumbnailCache::addWaveform(
            const std::string& key,
            const std::shared_ptr<feather_tk::TriMesh2F>& waveform)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.waveforms.add(key, waveform);
        }

        bool ThumbnailCache::containsWaveform(const std::string& key)
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.waveforms.contains(key);
        }

        bool ThumbnailCache::getWaveform(
            const std::string& key,
            std::shared_ptr<feather_tk::TriMesh2F>& waveform) const
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.waveforms.get(key, waveform);
        }

        void ThumbnailCache::clear()
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.clear();
            p.thumbnails.clear();
            p.waveforms.clear();
        }

        void ThumbnailCache::_maxUpdate()
        {
            FEATHER_TK_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.info.setMax(p.max);
            p.thumbnails.setMax(p.max);
            p.waveforms.setMax(p.max);
        }

        struct ThumbnailGenerator::Private
        {
            std::weak_ptr<feather_tk::Context> context;
            std::shared_ptr<ThumbnailCache> cache;
            std::shared_ptr<feather_tk::gl::Window> window;
            uint64_t requestId = 0;

            struct InfoRequest
            {
                uint64_t id = 0;
                file::Path path;
                std::vector<feather_tk::InMemoryFile> memoryRead;
                io::Options options;
                std::promise<io::Info> promise;
            };

            struct ThumbnailRequest
            {
                uint64_t id = 0;
                file::Path path;
                std::vector<feather_tk::InMemoryFile> memoryRead;
                int height = 0;
                OTIO_NS::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<std::shared_ptr<feather_tk::Image> > promise;
            };

            struct WaveformRequest
            {
                uint64_t id = 0;
                file::Path path;
                std::vector<feather_tk::InMemoryFile> memoryRead;
                feather_tk::Size2I size;
                OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
                io::Options options;
                std::promise<std::shared_ptr<feather_tk::TriMesh2F> > promise;
            };

            struct InfoMutex
            {
                std::list<std::shared_ptr<InfoRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            InfoMutex infoMutex;

            struct ThumbnailMutex
            {
                std::list<std::shared_ptr<ThumbnailRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            ThumbnailMutex thumbnailMutex;

            struct WaveformMutex
            {
                std::list<std::shared_ptr<WaveformRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            WaveformMutex waveformMutex;

            struct InfoThread
            {
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            InfoThread infoThread;

            struct ThumbnailThread
            {
                std::shared_ptr<timeline_gl::Render> render;
                std::shared_ptr<feather_tk::gl::OffscreenBuffer> buffer;
                feather_tk::LRUCache<std::string, std::shared_ptr<io::IRead> > ioCache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            ThumbnailThread thumbnailThread;

            struct WaveformThread
            {
                feather_tk::LRUCache<std::string, std::shared_ptr<io::IRead> > ioCache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            WaveformThread waveformThread;
        };

        void ThumbnailGenerator::_init(
            const std::shared_ptr<ThumbnailCache>& cache,
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<feather_tk::gl::Window>& window)
        {
            FEATHER_TK_P();
            
            p.context = context;

            p.cache = cache;

            p.window = window;
            if (!p.window)
            {
                p.window = feather_tk::gl::Window::create(
                    context,
                    "tl::timelineui::ThumbnailGenerator",
                    feather_tk::Size2I(1, 1),
                    static_cast<int>(feather_tk::gl::WindowOptions::None));
            }

            p.infoThread.running = true;
            p.infoThread.thread = std::thread(
                [this]
                {
                    FEATHER_TK_P();
                    while (p.infoThread.running)
                    {
                        _infoRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                        p.infoMutex.stopped = true;
                    }
                    _infoCancel();
                });

            p.thumbnailThread.ioCache.setMax(ioCacheMax);
            p.thumbnailThread.running = true;
            p.thumbnailThread.thread = std::thread(
                [this]
                {
                    FEATHER_TK_P();
                    p.window->makeCurrent();
                    if (auto context = p.context.lock())
                    {
                        p.thumbnailThread.render = timeline_gl::Render::create(context);
                    }
                    while (p.thumbnailThread.running)
                    {
                        _thumbnailRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                        p.thumbnailMutex.stopped = true;
                    }
                    p.thumbnailThread.buffer.reset();
                    p.thumbnailThread.render.reset();
                    p.window->doneCurrent();
                    _thumbnailCancel();
                });

            p.waveformThread.ioCache.setMax(ioCacheMax);
            p.waveformThread.running = true;
            p.waveformThread.thread = std::thread(
                [this]
                {
                    FEATHER_TK_P();
                    while (p.waveformThread.running)
                    {
                        _waveformRun();
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                        p.waveformMutex.stopped = true;
                    }
                    _waveformCancel();
                });
        }

        ThumbnailGenerator::ThumbnailGenerator() :
            _p(new Private)
        {}

        ThumbnailGenerator::~ThumbnailGenerator()
        {
            FEATHER_TK_P();
            p.infoThread.running = false;
            if (p.infoThread.thread.joinable())
            {
                p.infoThread.thread.join();
            }
            p.thumbnailThread.running = false;
            if (p.thumbnailThread.thread.joinable())
            {
                p.thumbnailThread.thread.join();
            }
            p.waveformThread.running = false;
            if (p.waveformThread.thread.joinable())
            {
                p.waveformThread.thread.join();
            }
        }

        std::shared_ptr<ThumbnailGenerator> ThumbnailGenerator::create(
            const std::shared_ptr<ThumbnailCache>& cache,
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<feather_tk::gl::Window>& window)
        {
            auto out = std::shared_ptr<ThumbnailGenerator>(new ThumbnailGenerator);
            out->_init(cache, context, window);
            return out;
        }

        InfoRequest ThumbnailGenerator::getInfo(
            const file::Path& path,
            const io::Options& options)
        {
            return getInfo(path, {}, options);
        }

        InfoRequest ThumbnailGenerator::getInfo(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memoryRead,
            const io::Options& options)
        {
            FEATHER_TK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = p.requestId;
            request->path = path;
            request->memoryRead = memoryRead;
            request->options = options;
            InfoRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                if (!p.infoMutex.stopped)
                {
                    valid = true;
                    p.infoMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.infoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return out;
        }

        ThumbnailRequest ThumbnailGenerator::getThumbnail(
            const file::Path& path,
            int height,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            return getThumbnail(path, {}, height, time, options);
        }

        ThumbnailRequest ThumbnailGenerator::getThumbnail(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memoryRead,
            int height,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            FEATHER_TK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::ThumbnailRequest>();
            request->id = p.requestId;
            request->path = path;
            request->memoryRead = memoryRead;
            request->height = height;
            request->time = time;
            request->options = options;
            ThumbnailRequest out;
            out.id = p.requestId;
            out.height = height;
            out.time = time;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                if (!p.thumbnailMutex.stopped)
                {
                    valid = true;
                    p.thumbnailMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.thumbnailThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        WaveformRequest ThumbnailGenerator::getWaveform(
            const file::Path& path,
            const feather_tk::Size2I& size,
            const OTIO_NS::TimeRange& range,
            const io::Options& options)
        {
            return getWaveform(path, {}, size, range, options);
        }

        WaveformRequest ThumbnailGenerator::getWaveform(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memoryRead,
            const feather_tk::Size2I& size,
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            FEATHER_TK_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::WaveformRequest>();
            request->id = p.requestId;
            request->path = path;
            request->memoryRead = memoryRead;
            request->size = size;
            request->timeRange = timeRange;
            request->options = options;
            WaveformRequest out;
            out.id = p.requestId;
            out.size = size;
            out.timeRange = timeRange;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                if (!p.waveformMutex.stopped)
                {
                    valid = true;
                    p.waveformMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.waveformThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        void ThumbnailGenerator::cancelRequests(const std::vector<uint64_t>& ids)
        {
            FEATHER_TK_P();
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                auto i = p.infoMutex.requests.begin();
                while (i != p.infoMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.infoMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                auto i = p.thumbnailMutex.requests.begin();
                while (i != p.thumbnailMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.thumbnailMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                auto i = p.waveformMutex.requests.begin();
                while (i != p.waveformMutex.requests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.waveformMutex.requests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
        }

        void ThumbnailGenerator::_infoRun()
        {
            FEATHER_TK_P();
            std::shared_ptr<Private::InfoRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                if (p.infoThread.cv.wait_for(
                    lock,
                    std::chrono::milliseconds(5),
                    [this]
                    {
                        return !_p->infoMutex.requests.empty();
                    }))
                {
                    request = p.infoMutex.requests.front();
                    p.infoMutex.requests.pop_front();
                }
            }
            if (request)
            {
                io::Info info;
                const std::string key = ThumbnailCache::getInfoKey(
                    request->path,
                    request->options);
                if (!p.cache->getInfo(key, info))
                {
                    if (auto context = p.context.lock())
                    {
                        auto ioSystem = context->getSystem<io::ReadSystem>();
                        try
                        {
                            const std::string& fileName = request->path.get();
                            //std::cout << "info request: " << request->path.get() << std::endl;
                            std::shared_ptr<io::IRead> read = ioSystem->read(
                                request->path,
                                request->memoryRead,
                                request->options);
                            if (read)
                            {
                                info = read->getInfo().get();
                            }
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
                request->promise.set_value(info);
                p.cache->addInfo(key, info);
            }
        }

        void ThumbnailGenerator::_thumbnailRun()
        {
            FEATHER_TK_P();
            std::shared_ptr<Private::ThumbnailRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                if (p.thumbnailThread.cv.wait_for(
                    lock,
                    std::chrono::milliseconds(5),
                    [this]
                    {
                        return !_p->thumbnailMutex.requests.empty();
                    }))
                {
                    request = p.thumbnailMutex.requests.front();
                    p.thumbnailMutex.requests.pop_front();
                }
            }
            if (request)
            {
                std::shared_ptr<feather_tk::Image> image;
                const std::string key = ThumbnailCache::getThumbnailKey(
                    request->height,
                    request->path,
                    request->time,
                    request->options);
                if (!p.cache->getThumbnail(key, image))
                {
                    if (auto context = p.context.lock())
                    {
                        auto ioSystem = context->getSystem<io::ReadSystem>();
                        try
                        {
                            const std::string& fileName = request->path.get();
                            //std::cout << "thumbnail request: " << fileName << " " <<
                            //    request->time << std::endl;
                            std::shared_ptr<io::IRead> read;
                            if (!p.thumbnailThread.ioCache.get(fileName, read))
                            {
                                read = ioSystem->read(
                                    request->path,
                                    request->memoryRead,
                                    request->options);
                                p.thumbnailThread.ioCache.add(fileName, read);
                            }
                            if (read)
                            {
                                const io::Info info = read->getInfo().get();
                                feather_tk::Size2I size;
                                if (!info.video.empty())
                                {
                                    size.w = request->height * feather_tk::aspectRatio(info.video[0].size);
                                    size.h = request->height;
                                }
                                feather_tk::gl::OffscreenBufferOptions options;
                                options.color = feather_tk::ImageType::RGBA_U8;
                                if (feather_tk::gl::doCreate(p.thumbnailThread.buffer, size, options))
                                {
                                    p.thumbnailThread.buffer = feather_tk::gl::OffscreenBuffer::create(size, options);
                                }
                                const OTIO_NS::RationalTime time =
                                    request->time != time::invalidTime ?
                                    request->time :
                                    info.videoTime.start_time();
                                const auto videoData = read->readVideo(time, request->options).get();
                                if (p.thumbnailThread.render && p.thumbnailThread.buffer && videoData.image)
                                {
                                    feather_tk::gl::OffscreenBufferBinding binding(p.thumbnailThread.buffer);
                                    p.thumbnailThread.render->begin(size);
                                    p.thumbnailThread.render->IRender::drawImage(
                                        videoData.image,
                                        feather_tk::Box2I(0, 0, size.w, size.h));
                                    p.thumbnailThread.render->end();
                                    image = feather_tk::Image::create(
                                        size.w,
                                        size.h,
                                        feather_tk::ImageType::RGBA_U8);
                                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                    glReadPixels(
                                        0,
                                        0,
                                        size.w,
                                        size.h,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE,
                                        image->getData());
                                }
                            }
                            else if (
                                feather_tk::compare(
                                    ".otio",
                                    request->path.getExtension(),
                                    feather_tk::CaseCompare::Insensitive) ||
                                feather_tk::compare(
                                    ".otioz",
                                    request->path.getExtension(),
                                    feather_tk::CaseCompare::Insensitive))
                            {
                                timeline::Options timelineOptions;
                                timelineOptions.ioOptions = request->options;
                                auto timeline = timeline::Timeline::create(
                                    context,
                                    request->path,
                                    timelineOptions);
                                const auto info = timeline->getIOInfo();
                                const auto videoData = timeline->getVideo(
                                    timeline->getTimeRange().start_time()).future.get();
                                feather_tk::Size2I size;
                                if (!info.video.empty())
                                {
                                    size.w = request->height * feather_tk::aspectRatio(info.video.front().size);
                                    size.h = request->height;
                                }
                                if (size.isValid())
                                {
                                    feather_tk::gl::OffscreenBufferOptions options;
                                    options.color = feather_tk::ImageType::RGBA_U8;
                                    if (feather_tk::gl::doCreate(p.thumbnailThread.buffer, size, options))
                                    {
                                        p.thumbnailThread.buffer = feather_tk::gl::OffscreenBuffer::create(size, options);
                                    }
                                    if (p.thumbnailThread.render && p.thumbnailThread.buffer)
                                    {
                                        feather_tk::gl::OffscreenBufferBinding binding(p.thumbnailThread.buffer);
                                        p.thumbnailThread.render->begin(size);
                                        p.thumbnailThread.render->drawVideo(
                                            { videoData },
                                            { feather_tk::Box2I(0, 0, size.w, size.h) });
                                        p.thumbnailThread.render->end();
                                        image = feather_tk::Image::create(
                                            size.w,
                                            size.h,
                                            feather_tk::ImageType::RGBA_U8);
                                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                        glReadPixels(
                                            0,
                                            0,
                                            size.w,
                                            size.h,
                                            GL_RGBA,
                                            GL_UNSIGNED_BYTE,
                                            image->getData());
                                    }
                                }
                            }
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
                request->promise.set_value(image);
                p.cache->addThumbnail(key, image);
            }
        }

        namespace
        {
            std::shared_ptr<feather_tk::TriMesh2F> audioMesh(
                const std::shared_ptr<audio::Audio>& audio,
                const feather_tk::Size2I& size)
            {
                auto out = std::shared_ptr<feather_tk::TriMesh2F>(new feather_tk::TriMesh2F);
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
                        for (int x = 0; x < size.w; ++x)
                        {
                            const int x0 = std::min(
                                static_cast<size_t>((x + 0) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            const int x1 = std::min(
                                static_cast<size_t>((x + 1) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            //std::cout << x << ": " << x0 << " " << x1 << std::endl;
                            audio::F32_T min = 0.F;
                            audio::F32_T max = 0.F;
                            if (x0 <= x1)
                            {
                                min = audio::F32Range.max();
                                max = audio::F32Range.min();
                                for (int i = x0; i <= x1 && i < sampleCount; ++i)
                                {
                                    const audio::F32_T v = *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            const int h2 = size.h / 2;
                            const feather_tk::Box2I box(
                                feather_tk::V2I(
                                    x,
                                    h2 - h2 * max),
                                feather_tk::V2I(
                                    x + 1,
                                    h2 - h2 * min));
                            if (box.isValid())
                            {
                                const size_t j = 1 + out->v.size();
                                out->v.push_back(feather_tk::V2F(box.x(), box.y()));
                                out->v.push_back(feather_tk::V2F(box.x() + box.w(), box.y()));
                                out->v.push_back(feather_tk::V2F(box.x() + box.w(), box.y() + box.h()));
                                out->v.push_back(feather_tk::V2F(box.x(), box.y() + box.h()));
                                out->triangles.push_back(feather_tk::Triangle2({ j + 0, j + 1, j + 2 }));
                                out->triangles.push_back(feather_tk::Triangle2({ j + 2, j + 3, j + 0 }));
                            }
                        }
                        break;
                    }
                    default: break;
                    }
                }
                return out;
            }

            std::shared_ptr<feather_tk::Image> audioImage(
                const std::shared_ptr<audio::Audio>& audio,
                const feather_tk::Size2I& size)
            {
                auto out = feather_tk::Image::create(size.w, size.h, feather_tk::ImageType::L_U8);
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
                        for (int x = 0; x < size.w; ++x)
                        {
                            const int x0 = std::min(
                                static_cast<size_t>((x + 0) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            const int x1 = std::min(
                                static_cast<size_t>((x + 1) / static_cast<double>(size.w - 1) * (sampleCount - 1)),
                                sampleCount - 1);
                            //std::cout << x << ": " << x0 << " " << x1 << std::endl;
                            audio::F32_T min = 0.F;
                            audio::F32_T max = 0.F;
                            if (x0 < x1)
                            {
                                min = audio::F32Range.max();
                                max = audio::F32Range.min();
                                for (int i = x0; i < x1; ++i)
                                {
                                    const audio::F32_T v = *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            uint8_t* p = out->getData() + x;
                            for (int y = 0; y < size.h; ++y)
                            {
                                const float v = y / static_cast<float>(size.h - 1) * 2.F - 1.F;
                                *p = (v > min && v < max) ? 255 : 0;
                                p += size.w;
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

        void ThumbnailGenerator::_waveformRun()
        {
            FEATHER_TK_P();
            std::shared_ptr<Private::WaveformRequest> request;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                if (p.waveformThread.cv.wait_for(
                    lock,
                    std::chrono::milliseconds(5),
                    [this]
                    {
                        return !_p->waveformMutex.requests.empty();
                    }))
                {
                    request = p.waveformMutex.requests.front();
                    p.waveformMutex.requests.pop_front();
                }
            }
            if (request)
            {
                std::shared_ptr<feather_tk::TriMesh2F> mesh;
                const std::string key = ThumbnailCache::getWaveformKey(
                    request->size,
                    request->path,
                    request->timeRange,
                    request->options);
                if (!p.cache->getWaveform(key, mesh))
                {
                    if (auto context = p.context.lock())
                    {
                        try
                        {
                            const std::string& fileName = request->path.get();
                            std::shared_ptr<io::IRead> read;
                            if (!p.waveformThread.ioCache.get(fileName, read))
                            {
                                auto ioSystem = context->getSystem<io::ReadSystem>();
                                read = ioSystem->read(
                                    request->path,
                                    request->memoryRead,
                                    request->options);
                                p.waveformThread.ioCache.add(fileName, read);
                            }
                            if (read)
                            {
                                const auto info = read->getInfo().get();
                                const OTIO_NS::TimeRange timeRange =
                                    request->timeRange != time::invalidTimeRange ?
                                    request->timeRange :
                                    OTIO_NS::TimeRange(
                                        OTIO_NS::RationalTime(0.0, 1.0),
                                        OTIO_NS::RationalTime(1.0, 1.0));
                                const auto audioData = read->readAudio(timeRange, request->options).get();
                                if (audioData.audio)
                                {
                                    auto resample = audio::AudioResample::create(
                                        audioData.audio->getInfo(),
                                        audio::Info(1, audio::DataType::F32, audioData.audio->getSampleRate()));
                                    const auto resampledAudio = resample->process(audioData.audio);
                                    mesh = audioMesh(resampledAudio, request->size);
                                }
                            }
                        }
                        catch (const std::exception&)
                        {
                        }
                    }
                }
                request->promise.set_value(mesh);
                p.cache->addWaveform(key, mesh);
            }
        }

        void ThumbnailGenerator::_infoCancel()
        {
            FEATHER_TK_P();
            std::list<std::shared_ptr<Private::InfoRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.infoMutex.mutex);
                requests = std::move(p.infoMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::Info());
            }
        }

        void ThumbnailGenerator::_thumbnailCancel()
        {
            FEATHER_TK_P();
            std::list<std::shared_ptr<Private::ThumbnailRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.thumbnailMutex.mutex);
                requests = std::move(p.thumbnailMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(nullptr);
            }
        }

        void ThumbnailGenerator::_waveformCancel()
        {
            FEATHER_TK_P();
            std::list<std::shared_ptr<Private::WaveformRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.waveformMutex.mutex);
                requests = std::move(p.waveformMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(nullptr);
            }
        }

        struct ThumbnailSystem::Private
        {
            std::shared_ptr<ThumbnailCache> cache;
            std::shared_ptr<ThumbnailGenerator> generator;
        };

        ThumbnailSystem::ThumbnailSystem(const std::shared_ptr<feather_tk::Context>& context) :
            ISystem(context, "tl::timelineui::ThumbnailSystem"),
            _p(new Private)
        {
            FEATHER_TK_P();
            p.cache = ThumbnailCache::create(context);
            p.generator = ThumbnailGenerator::create(p.cache, context);
        }

        ThumbnailSystem::~ThumbnailSystem()
        {}

        std::shared_ptr<ThumbnailSystem> ThumbnailSystem::create(
            const std::shared_ptr<feather_tk::Context>& context)
        {
            auto out = context->getSystem<ThumbnailSystem>();
            if (!out)
            {
                out = std::shared_ptr<ThumbnailSystem>(new ThumbnailSystem(context));
                context->addSystem(out);
            }
            return out;
        }

        InfoRequest ThumbnailSystem::getInfo(
            const file::Path& path,
            const io::Options& ioOptions)
        {
            return _p->generator->getInfo(path, ioOptions);
        }

        ThumbnailRequest ThumbnailSystem::getThumbnail(
            const file::Path& path,
            int height,
            const OTIO_NS::RationalTime& time,
            const io::Options& ioOptions)
        {
            return _p->generator->getThumbnail(path, height, time, ioOptions);
        }

        WaveformRequest ThumbnailSystem::getWaveform(
            const file::Path& path,
            const feather_tk::Size2I& size,
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& ioOptions)
        {
            return _p->generator->getWaveform(path, size, timeRange, ioOptions);
        }

        void ThumbnailSystem::cancelRequests(const std::vector<uint64_t>& ids)
        {
            _p->generator->cancelRequests(ids);
        }
        
        const std::shared_ptr<ThumbnailCache>& ThumbnailSystem::getCache() const
        {
            return _p->cache;
        }
    }
}
