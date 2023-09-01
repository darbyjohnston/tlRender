// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IOManager.h>

#include <tlTimeline/GLRender.h>

#include <tlIO/System.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>
#include <tlGL/OffscreenBuffer.h>

#include <tlCore/AudioConvert.h>
#include <tlCore/LRUCache.h>
#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        namespace
        {
            const size_t infoRequestsMax = 3;
            const size_t thumbnailRequestsMax = 3;
            const size_t waveformRequestsMax = 3;
        }

        struct IOManager::Private
        {
            std::weak_ptr<system::Context> context;
            io::Options ioOptions;

            std::shared_ptr<gl::GLFWWindow> window;
            
            uint64_t requestId = 0;

            struct InfoRequest
            {
                uint64_t id = 0;
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                otime::RationalTime startTime = time::invalidTime;
                std::promise<io::Info> promise;
            };

            struct ThumbnailRequest
            {
                uint64_t id = 0;
                math::Size2i size;
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                otime::RationalTime startTime = time::invalidTime;
                otime::RationalTime time = time::invalidTime;
                uint16_t layer = 0;
                std::promise<std::shared_ptr<image::Image> > promise;
            };

            struct WaveformRequest
            {
                uint64_t id = 0;
                math::Size2i size;
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                otime::RationalTime startTime = time::invalidTime;
                otime::TimeRange range = time::invalidTimeRange;
                std::promise<std::shared_ptr<geom::TriangleMesh2> > promise;
            };
            
            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<ThumbnailRequest> > thumbnailRequests;
                std::list<std::shared_ptr<WaveformRequest> > waveformRequests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;
            
            struct Thread
            {
                memory::LRUCache<std::string, io::Info> infoCache;
                memory::LRUCache<std::string, std::shared_ptr<image::Image> > thumbnailCache;
                memory::LRUCache<std::string, std::shared_ptr<geom::TriangleMesh2> > waveformCache;
                memory::LRUCache<std::string, std::shared_ptr<io::IRead> > ioCache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void IOManager::_init(
            const io::Options& ioOptions,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.ioOptions = ioOptions;
            {
                std::stringstream ss;
                ss << 1;
                p.ioOptions["ffmpeg/VideoBufferSize"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << otime::RationalTime(1.0, 1.0);
                p.ioOptions["ffmpeg/AudioBufferSize"] = ss.str();
            }

            p.window = gl::GLFWWindow::create(
                "tl::timelineui::IOManager",
                math::Size2i(1, 1),
                context,
                static_cast<int>(gl::GLFWWindowOptions::None));

            p.thread.infoCache.setMax(1000);
            p.thread.thumbnailCache.setMax(1000);
            p.thread.waveformCache.setMax(1000);
            p.thread.ioCache.setMax(1000);
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    try
                    {
                        p.window->makeCurrent();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto context = p.context.lock())
                        {
                            context->log(
                                "tl::timelineui::IOManager",
                                string::Format("Cannot make the OpenGL context current: {0}").
                                    arg(e.what()));
                        }
                    }
                    _run();
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    p.window->doneCurrent();
                    _cancelRequests();
                });
        }

        IOManager::IOManager() :
            _p(new Private)
        {}

        IOManager::~IOManager()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<IOManager> IOManager::create(
            const io::Options& options,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<IOManager>(new IOManager);
            out->_init(options, context);
            return out;
        }

        InfoRequest IOManager::requestInfo(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& startTime)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = p.requestId;
            request->path = path;
            request->memoryRead = memoryRead;
            request->startTime = startTime;
            InfoRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return out;
        }

        ThumbnailRequest IOManager::requestThumbnail(
            const math::Size2i& size,
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& startTime,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::ThumbnailRequest>();
            request->id = p.requestId;
            request->size = size;
            request->path = path;
            request->memoryRead = memoryRead;
            request->startTime = startTime;
            request->time = time;
            request->layer = layer;
            ThumbnailRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.thumbnailRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        WaveformRequest IOManager::requestWaveform(
            const math::Size2i& size,
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& startTime,
            const otime::TimeRange& range)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::WaveformRequest>();
            request->id = p.requestId;
            request->size = size;
            request->path = path;
            request->memoryRead = memoryRead;
            request->startTime = startTime;
            request->range = range;
            WaveformRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.waveformRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(nullptr);
            }
            return out;
        }

        void IOManager::cancelRequests(std::vector<uint64_t> ids)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            auto infoRequest = p.mutex.infoRequests.begin();
            while (infoRequest != p.mutex.infoRequests.end())
            {
                const auto id = std::find(ids.begin(), ids.end(), (*infoRequest)->id);
                if (id != ids.end())
                {
                    infoRequest = p.mutex.infoRequests.erase(infoRequest);
                    ids.erase(id);
                }
                else
                {
                    ++infoRequest;
                }
            }
            auto thumbnailRequest = p.mutex.thumbnailRequests.begin();
            while (thumbnailRequest != p.mutex.thumbnailRequests.end())
            {
                const auto id = std::find(ids.begin(), ids.end(), (*thumbnailRequest)->id);
                if (id != ids.end())
                {
                    thumbnailRequest = p.mutex.thumbnailRequests.erase(thumbnailRequest);
                    ids.erase(id);
                }
                else
                {
                    ++thumbnailRequest;
                }
            }
            auto waveformRequest = p.mutex.waveformRequests.begin();
            while (waveformRequest != p.mutex.waveformRequests.end())
            {
                const auto id = std::find(ids.begin(), ids.end(), (*waveformRequest)->id);
                if (id != ids.end())
                {
                    waveformRequest = p.mutex.waveformRequests.erase(waveformRequest);
                    ids.erase(id);
                }
                else
                {
                    ++waveformRequest;
                }
            }
        }

        namespace
        {
            std::shared_ptr<geom::TriangleMesh2> audioMesh(
                const std::shared_ptr<audio::Audio>& audio,
                const math::Size2i& size)
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
                                min = audio::F32Range.getMax();
                                max = audio::F32Range.getMin();
                                for (int i = x0; i < x1; ++i)
                                {
                                    const audio::F32_T v = *(data + i * info.channelCount);
                                    min = std::min(min, v);
                                    max = std::max(max, v);
                                }
                            }
                            const int h2 = size.h / 2;
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
                const math::Size2i& size)
            {
                auto out = image::Image::create(size.w, size.h, image::PixelType::L_U8);
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

            std::string getInfoKey(
                const file::Path& path,
                const otime::RationalTime& startTime)
            {
                return string::Format("{0}_{1}").
                    arg(path.get()).
                    arg(startTime);
            }

            std::string getThumbnailKey(
                const math::Size2i& size,
                const file::Path& path,
                const otime::RationalTime& startTime,
                const otime::RationalTime& time,
                uint16_t layer)
            {
                return string::Format("{0}_{1}_{2}_{3}_{4}").
                    arg(size).
                    arg(path.get()).
                    arg(startTime).
                    arg(time).
                    arg(layer);
            }

            std::string getWaveformKey(
                const math::Size2i& size,
                const file::Path& path,
                const otime::RationalTime& startTime,
                const otime::TimeRange& timeRange)
            {
                return string::Format("{0}_{1}_{2}_{3}").
                    arg(size).
                    arg(path.get()).
                    arg(startTime).
                    arg(timeRange);
            }
        }
        
        void IOManager::_run()
        {
            TLRENDER_P();
            std::shared_ptr<timeline::GLRender> render;
            if (auto context = p.context.lock())
            {
                render = timeline::GLRender::create(context);
            }
            std::shared_ptr<gl::OffscreenBuffer> buffer;
            while (p.thread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::list<std::shared_ptr<Private::ThumbnailRequest> > thumbnailRequests;
                std::list<std::shared_ptr<Private::WaveformRequest> > waveformRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(5),
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.thumbnailRequests.empty() ||
                                !_p->mutex.waveformRequests.empty();
                        }))
                    {
                        while (!p.mutex.infoRequests.empty() &&
                            infoRequests.size() < infoRequestsMax)
                        {
                            infoRequests.push_back(p.mutex.infoRequests.front());
                            p.mutex.infoRequests.pop_front();
                        }
                        if (!p.mutex.thumbnailRequests.empty() &&
                            thumbnailRequests.size() < thumbnailRequestsMax)
                        {
                            thumbnailRequests.push_back(p.mutex.thumbnailRequests.front());
                            p.mutex.thumbnailRequests.pop_front();
                        }
                        if (!p.mutex.waveformRequests.empty() &&
                            waveformRequests.size() < waveformRequestsMax)
                        {
                            waveformRequests.push_back(p.mutex.waveformRequests.front());
                            p.mutex.waveformRequests.pop_front();
                        }
                    }
                }
                                
                // Handle information requests.
                for (const auto& infoRequest : infoRequests)
                {
                    io::Info info;
                    const std::string key = getInfoKey(
                        infoRequest->path,
                        infoRequest->startTime);
                    if (!p.thread.infoCache.get(key, info))
                    {
                        const std::string& fileName = infoRequest->path.get();
                        //std::cout << "info request: " << infoRequest->path.get() << std::endl;
                        std::shared_ptr<io::IRead> read;
                        if (!p.thread.ioCache.get(fileName, read))
                        {
                            if (auto context = p.context.lock())
                            {
                                auto ioSystem = context->getSystem<io::System>();
                                io::Options options = p.ioOptions;
                                options["FFmpeg/StartTime"] = string::Format("{0}").arg(infoRequest->startTime);
                                try
                                {
                                    read = ioSystem->read(
                                        infoRequest->path,
                                        infoRequest->memoryRead,
                                        options);
                                    p.thread.ioCache.add(fileName, read);
                                }
                                catch (const std::exception&)
                                {}
                            }
                        }
                        if (read)
                        {
                            info = read->getInfo().get();
                        }
                        p.thread.infoCache.add(key, info);
                    }
                    infoRequest->promise.set_value(info);
                }
                
                // Handle thumbnail requests.
                for (const auto& request : thumbnailRequests)
                {
                    std::shared_ptr<image::Image> image;
                    const std::string key = getThumbnailKey(
                        request->size,
                        request->path,
                        request->startTime,
                        request->time,
                        request->layer);
                    if (!p.thread.thumbnailCache.get(key, image))
                    {
                        try
                        {
                            const std::string& fileName = request->path.get();
                            //std::cout << "thumbnail request: " << fileName << " " <<
                            //    request->time << std::endl;
                            std::shared_ptr<io::IRead> read;
                            if (!p.thread.ioCache.get(fileName, read))
                            {
                                if (auto context = p.context.lock())
                                {
                                    auto ioSystem = context->getSystem<io::System>();
                                    io::Options options = p.ioOptions;
                                    options["FFmpeg/StartTime"] = string::Format("{0}").arg(request->startTime);
                                    try
                                    {
                                        read = ioSystem->read(
                                            request->path,
                                            request->memoryRead,
                                            options);
                                        p.thread.ioCache.add(fileName, read);
                                    }
                                    catch (const std::exception&)
                                    {}
                                }
                            }
                            if (read)
                            {
                                auto videoData = read->readVideo(request->time).get();
                                gl::OffscreenBufferOptions options;
                                options.colorType = image::PixelType::RGB_F32;
                                if (gl::doCreate(buffer, request->size, options))
                                {
                                    buffer = gl::OffscreenBuffer::create(request->size, options);
                                }
                                if (render && buffer && videoData.image)
                                {
                                    try
                                    {
                                        gl::OffscreenBufferBinding binding(buffer);
                                        render->begin(request->size);
                                        render->drawImage(
                                            videoData.image,
                                            { math::Box2i(0, 0, request->size.w, request->size.h) });
                                        render->end();
                                        image = image::Image::create(
                                            request->size.w,
                                            request->size.h,
                                            image::PixelType::RGBA_U8);
                                        glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                        glReadPixels(
                                            0,
                                            0,
                                            request->size.w,
                                            request->size.h,
                                            GL_RGBA,
                                            GL_UNSIGNED_BYTE,
                                            image->getData());
                                    }
                                    catch (const std::exception&)
                                    {}
                                }
                            }
                        }
                        catch (const std::exception&)
                        {}
                        p.thread.thumbnailCache.add(key, image);
                    }
                    request->promise.set_value(image);
                }
                
                // Handle waveform requests.
                for (const auto& request : waveformRequests)
                {
                    std::shared_ptr<geom::TriangleMesh2> mesh;
                    const std::string key = getWaveformKey(
                        request->size,
                        request->path,
                        request->startTime,
                        request->range);
                    if (!p.thread.waveformCache.get(key, mesh))
                    {
                        try
                        {
                            const std::string& fileName = request->path.get();
                            std::shared_ptr<io::IRead> read;
                            if (!p.thread.ioCache.get(fileName, read))
                            {
                                if (auto context = p.context.lock())
                                {
                                    auto ioSystem = context->getSystem<io::System>();
                                    io::Options options = p.ioOptions;
                                    options["FFmpeg/StartTime"] = string::Format("{0}").arg(request->startTime);
                                    try
                                    {
                                        read = ioSystem->read(
                                            request->path,
                                            request->memoryRead,
                                            options);
                                        p.thread.ioCache.add(fileName, read);
                                    }
                                    catch (const std::exception&)
                                    {}
                                }
                            }
                            if (read)
                            {
                                auto audioData = read->readAudio(request->range).get();
                                if (audioData.audio)
                                {
                                    auto convert = audio::AudioConvert::create(
                                        audioData.audio->getInfo(),
                                        audio::Info(1, audio::DataType::F32, audioData.audio->getSampleRate()));
                                    const auto convertedAudio = convert->convert(audioData.audio);
                                    mesh = audioMesh(convertedAudio, request->size);
                                }
                            }
                        }
                        catch (const std::exception&)
                        {}
                        p.thread.waveformCache.add(key, mesh);
                    }
                    request->promise.set_value(mesh);
                }
            }
        }
        
        void IOManager::_cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::ThumbnailRequest> > thumbnailRequests;
            std::list<std::shared_ptr<Private::WaveformRequest> > waveformRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                thumbnailRequests = std::move(p.mutex.thumbnailRequests);
                waveformRequests = std::move(p.mutex.waveformRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : thumbnailRequests)
            {
                request->promise.set_value(nullptr);
            }
            for (auto& request : waveformRequests)
            {
                request->promise.set_value(nullptr);
            }
        }
    }
}
