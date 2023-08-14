// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IOManager.h>

#include <tlTimeline/GLRender.h>

#include <tlIO/IOSystem.h>

#include <tlGl/GL.h>
#include <tlGL/OffscreenBuffer.h>

#include <tlCore/AudioConvert.h>
#include <tlCore/LRUCache.h>
#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        struct IOManager::Private
        {
            std::weak_ptr<system::Context> context;
            io::Options ioOptions;
            std::shared_ptr<observer::Value<bool> > cancelRequests;

            GLFWwindow* glfwWindow = nullptr;
            
            struct InfoRequest
            {
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                otime::RationalTime startTime = time::invalidTime;
                std::promise<io::Info> promise;
            };

            struct VideoRequest
            {
                math::Size2i size;
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                otime::RationalTime startTime = time::invalidTime;
                otime::RationalTime time = time::invalidTime;
                uint16_t layer = 0;
                std::promise<std::shared_ptr<image::Image> > promise;
            };

            struct AudioRequest
            {
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
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::list<std::shared_ptr<AudioRequest> > audioRequests;
                bool cancelRequests = true;
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
            p.cancelRequests = observer::Value<bool>::create(false);

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
            p.glfwWindow = glfwCreateWindow(1, 1, "tl::ui::ThumbnailSystem", NULL, NULL);
            if (!p.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }

            p.thread.infoCache.setMax(1000);
            p.thread.thumbnailCache.setMax(1000);
            p.thread.waveformCache.setMax(1000);
            p.thread.ioCache.setMax(1000);
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    glfwMakeContextCurrent(p.glfwWindow);
                    _run();
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
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

        std::future<io::Info> IOManager::requestInfo(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& startTime)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            request->path = path;
            request->memoryRead = memoryRead;
            request->startTime = startTime;
            auto future = request->promise.get_future();
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
            return future;
        }

        std::future<std::shared_ptr<image::Image> > IOManager::requestVideo(
            const math::Size2i& size,
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& startTime,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->size = size;
            request->path = path;
            request->memoryRead = memoryRead;
            request->startTime = startTime;
            request->time = time;
            request->layer = layer;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
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
            return future;
        }

        std::future<std::shared_ptr<geom::TriangleMesh2> > IOManager::requestAudio(
            const math::Size2i& size,
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& startTime,
            const otime::TimeRange& range)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->size = size;
            request->path = path;
            request->memoryRead = memoryRead;
            request->startTime = startTime;
            request->range = range;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.audioRequests.push_back(request);
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
            return future;
        }

        void IOManager::cancelRequests()
        {
            TLRENDER_P();
            p.cancelRequests->setAlways(true);
            _cancelRequests();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            p.mutex.cancelRequests = true;
        }

        std::shared_ptr<observer::IValue<bool> > IOManager::observeCancelRequests() const
        {
            return _p->cancelRequests;
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

            std::string getVideoKey(
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

            std::string getAudioKey(
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
                std::shared_ptr<Private::InfoRequest> infoRequest;
                std::shared_ptr<Private::VideoRequest> videoRequest;
                std::shared_ptr<Private::AudioRequest> audioRequest;
                bool cancelRequests = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(5),
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.videoRequests.empty() ||
                                !_p->mutex.audioRequests.empty() ||
                                _p->mutex.cancelRequests;
                        }))
                    {
                        if (!p.mutex.infoRequests.empty())
                        {
                            infoRequest = p.mutex.infoRequests.front();
                            p.mutex.infoRequests.pop_front();
                        }
                        if (!p.mutex.videoRequests.empty())
                        {
                            videoRequest = p.mutex.videoRequests.front();
                            p.mutex.videoRequests.pop_front();
                        }
                        if (!p.mutex.audioRequests.empty())
                        {
                            audioRequest = p.mutex.audioRequests.front();
                            p.mutex.audioRequests.pop_front();
                        }
                        if (p.mutex.cancelRequests)
                        {
                            cancelRequests = true;
                            p.mutex.cancelRequests = false;
                        }
                    }
                }
                
                // Cancel requests.
                if (cancelRequests)
                {
                    for (const auto& i : p.thread.ioCache.getValues())
                    {
                        if (i)
                        {
                            i->cancelRequests();
                        }
                    }
                }
                
                // Handle information requests.
                if (infoRequest)
                {
                    io::Info info;
                    const std::string key = getInfoKey(
                        infoRequest->path,
                        infoRequest->startTime);
                    if (!p.thread.infoCache.get(key, info))
                    {
                        std::shared_ptr<io::IRead> read;
                        const std::string& fileName = infoRequest->path.get();
                        if (!p.thread.ioCache.get(fileName, read))
                        {
                            if (auto context = p.context.lock())
                            {
                                auto ioSystem = context->getSystem<io::System>();
                                io::Options options = p.ioOptions;
                                options["FFmpeg/StartTime"] = string::Format("{0}").arg(infoRequest->startTime);
                                read = ioSystem->read(
                                    infoRequest->path,
                                    infoRequest->memoryRead,
                                    options);
                                p.thread.ioCache.add(fileName, read);
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
                
                // Handle video requests.
                if (videoRequest)
                {
                    std::shared_ptr<image::Image> image;
                    const std::string key = getVideoKey(
                        videoRequest->size,
                        videoRequest->path,
                        videoRequest->startTime,
                        videoRequest->time,
                        videoRequest->layer);
                    if (!p.thread.thumbnailCache.get(key, image))
                    {
                        try
                        {
                            const std::string& fileName = videoRequest->path.get();
                            std::shared_ptr<io::IRead> read;
                            if (!p.thread.ioCache.get(fileName, read))
                            {
                                if (auto context = p.context.lock())
                                {
                                    auto ioSystem = context->getSystem<io::System>();
                                    io::Options options = p.ioOptions;
                                    options["FFmpeg/StartTime"] = string::Format("{0}").arg(videoRequest->startTime);
                                    read = ioSystem->read(
                                        videoRequest->path,
                                        videoRequest->memoryRead,
                                        options);
                                    p.thread.ioCache.add(fileName, read);
                                }
                            }
                            if (read)
                            {
                                auto videoData = read->readVideo(videoRequest->time).get();
                                gl::OffscreenBufferOptions options;
                                options.colorType = image::PixelType::RGB_F32;
                                if (gl::doCreate(buffer, videoRequest->size, options))
                                {
                                    buffer = gl::OffscreenBuffer::create(videoRequest->size, options);
                                }
                                if (render && buffer && videoData.image)
                                {
                                    gl::OffscreenBufferBinding binding(buffer);
                                    render->begin(videoRequest->size);
                                    render->drawImage(
                                        videoData.image,
                                        { math::Box2i(0, 0, videoRequest->size.w, videoRequest->size.h) });
                                    render->end();
                                    image = image::Image::create(
                                        videoRequest->size.w,
                                        videoRequest->size.h,
                                        image::PixelType::RGBA_U8);
                                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                    glReadPixels(
                                        0,
                                        0,
                                        videoRequest->size.w,
                                        videoRequest->size.h,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE,
                                        image->getData());
                                }
                            }
                        }
                        catch (const std::exception&)
                        {}
                        p.thread.thumbnailCache.add(key, image);
                    }
                    videoRequest->promise.set_value(image);
                }
                
                // Handle audio requests.
                if (audioRequest)
                {
                    std::shared_ptr<geom::TriangleMesh2> mesh;
                    const std::string key = getAudioKey(
                        audioRequest->size,
                        audioRequest->path,
                        audioRequest->startTime,
                        audioRequest->range);
                    if (!p.thread.waveformCache.get(key, mesh))
                    {
                        try
                        {
                            const std::string& fileName = audioRequest->path.get();
                            std::shared_ptr<io::IRead> read;
                            if (!p.thread.ioCache.get(fileName, read))
                            {
                                if (auto context = p.context.lock())
                                {
                                    auto ioSystem = context->getSystem<io::System>();
                                    io::Options options = p.ioOptions;
                                    options["FFmpeg/StartTime"] = string::Format("{0}").arg(audioRequest->startTime);
                                    read = ioSystem->read(
                                        audioRequest->path,
                                        audioRequest->memoryRead,
                                        options);
                                    p.thread.ioCache.add(fileName, read);
                                }
                            }
                            if (read)
                            {
                                auto audioData = read->readAudio(audioRequest->range).get();
                                if (audioData.audio)
                                {
                                    auto convert = audio::AudioConvert::create(
                                        audioData.audio->getInfo(),
                                        audio::Info(1, audio::DataType::F32, audioData.audio->getSampleRate()));
                                    const auto convertedAudio = convert->convert(audioData.audio);
                                    mesh = audioMesh(convertedAudio, audioRequest->size);
                                }
                            }
                        }
                        catch (const std::exception&)
                        {}
                        p.thread.waveformCache.add(key, mesh);
                    }
                    audioRequest->promise.set_value(mesh);
                }
            }
        }
        
        void IOManager::_cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                videoRequests = std::move(p.mutex.videoRequests);
                audioRequests = std::move(p.mutex.audioRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(nullptr);
            }
            for (auto& request : audioRequests)
            {
                request->promise.set_value(nullptr);
            }
        }
    }
}
