// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQt/TimelineThumbnailObject.h>

#include <tlTimeline/GLRender.h>

#include <tlGL/Init.h>
#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/Timeline.h>

#include <tlCore/StringFormat.h>

#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <atomic>
#include <mutex>

namespace tl
{
    namespace qt
    {
        struct TimelineThumbnailObject::Private
        {
            std::weak_ptr<system::Context> context;

            struct Request
            {
                qint64 id;
                QString fileName;
                QList<otime::RationalTime> times;
                QSize size;
                timeline::ColorConfigOptions colorConfigOptions;
                timeline::LUTOptions lutOptions;

                std::shared_ptr<timeline::Timeline> timeline;
                std::vector<std::future<timeline::VideoData> > futures;
            };
            std::list<Request> requests;
            std::list<Request> requestsInProgress;

            struct Result
            {
                qint64 id;
                QList<QPair<otime::RationalTime, QImage> > thumbnails;
            };
            std::vector<Result> results;

            qint64 id = 0;
            std::vector<qint64> cancelRequests;
            size_t requestCount = 1;
            std::chrono::milliseconds requestTimeout = std::chrono::milliseconds(25);
            int timer = 0;
            int timerInterval = 50;
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
            std::condition_variable cv;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        TimelineThumbnailObject::TimelineThumbnailObject(
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            QThread(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            p.glContext.reset(new QOpenGLContext);
            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            p.glContext->setFormat(surfaceFormat);
            p.glContext->create();

            p.offscreenSurface.reset(new QOffscreenSurface);
            p.offscreenSurface->setFormat(p.glContext->format());
            p.offscreenSurface->create();

            p.glContext->moveToThread(this);

            p.running = true;
            start();
            p.timer = startTimer(p.timerInterval);
        }

        TimelineThumbnailObject::~TimelineThumbnailObject()
        {
            TLRENDER_P();
            p.running = false;
            wait();
        }

        qint64 TimelineThumbnailObject::request(
            const QString& fileName,
            const QSize& size,
            const otime::RationalTime& time,
            const timeline::ColorConfigOptions& colorConfigOptions,
            const timeline::LUTOptions& lutOptions)
        {
            TLRENDER_P();
            qint64 out = 0;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.id = p.id + 1;
                Private::Request request;
                request.id = p.id;
                request.fileName = fileName;
                request.times.push_back(time);
                request.size = size;
                request.colorConfigOptions = colorConfigOptions;
                request.lutOptions = lutOptions;
                p.requests.push_back(std::move(request));
                out = p.id;
            }
            p.cv.notify_one();
            return out;
        }

        qint64 TimelineThumbnailObject::request(
            const QString& fileName,
            const QSize& size,
            const QList<otime::RationalTime>& times,
            const timeline::ColorConfigOptions& colorConfigOptions,
            const timeline::LUTOptions& lutOptions)
        {
            TLRENDER_P();
            qint64 out = 0;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.id = p.id + 1;
                Private::Request request;
                request.id = p.id;
                request.fileName = fileName;
                request.times = times;
                request.size = size;
                request.colorConfigOptions = colorConfigOptions;
                request.lutOptions = lutOptions;
                p.requests.push_back(std::move(request));
                out = p.id;
            }
            p.cv.notify_one();
            return out;
        }

        void TimelineThumbnailObject::cancelRequests(qint64 id)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            auto requestIt = p.requests.begin();
            while (requestIt != p.requests.end())
            {
                if (id == requestIt->id)
                {
                    requestIt = p.requests.erase(requestIt);
                    continue;
                }
                ++requestIt;
            }
            auto resultIt = p.results.begin();
            while (resultIt != p.results.end())
            {
                if (id == resultIt->id)
                {
                    resultIt = p.results.erase(resultIt);
                    continue;
                }
                ++resultIt;
            }
            p.cancelRequests.push_back(id);
        }

        void TimelineThumbnailObject::setRequestCount(int value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.requestCount = value > 0 ? value : 0;
        }

        void TimelineThumbnailObject::setRequestTimeout(int value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.requestTimeout = std::chrono::milliseconds(value > 0 ? value : 0);
        }

        void TimelineThumbnailObject::setTimerInterval(int value)
        {
            TLRENDER_P();
            killTimer(p.timer);
            p.timer = startTimer(value);
        }

        void TimelineThumbnailObject::run()
        {
            TLRENDER_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gl::initGLAD();

            if (auto context = p.context.lock())
            {
                auto render = timeline::GLRender::create(context);

                std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
                while (p.running)
                {
                    //std::cout << "requests: " << p.requests.size() << std::endl;
                    //std::cout << "requests in progress: " << p.requestsInProgress.size() << std::endl;
                    //std::cout << "results: " << p.results.size() << std::endl;

                    // Gather requests.
                    std::list<Private::Request> newRequests;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if (p.cv.wait_for(
                            lock,
                            p.requestTimeout,
                            [this]
                            {
                                return
                                    !_p->requests.empty() ||
                                    !_p->requestsInProgress.empty() ||
                                    !_p->cancelRequests.empty();
                            }))
                        {
                            for (auto i : p.cancelRequests)
                            {
                                auto j = p.requestsInProgress.begin();
                                while (j != p.requestsInProgress.end())
                                {
                                    if (i == j->id)
                                    {
                                        j = p.requestsInProgress.erase(j);
                                        continue;
                                    }
                                    ++j;
                                }
                            }
                            p.cancelRequests.clear();
                            while (!p.requests.empty() &&
                                (p.requestsInProgress.size() + newRequests.size()) < p.requestCount)
                            {
                                newRequests.push_back(std::move(p.requests.front()));
                                p.requests.pop_front();
                            }
                        }
                    }

                    // Initialize new requests.
                    for (auto& request : newRequests)
                    {
                        timeline::Options options;
                        options.videoRequestCount = 1;
                        options.audioRequestCount = 1;
                        options.requestTimeout = std::chrono::milliseconds(25);
                        options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg(1);
                        options.ioOptions["FFmpeg/ThreadCount"] = string::Format("{0}").arg(1);
                        try
                        {
                            request.timeline = timeline::Timeline::create(
                                file::Path(request.fileName.toUtf8().data()),
                                context,
                                options);
                            for (const auto& i : request.times)
                            {
                                request.futures.push_back(request.timeline->getVideo(
                                    time::isValid(i) ?
                                    i :
                                    request.timeline->getTimeRange().start_time()));
                            }
                        }
                        catch (const std::exception& e)
                        {
                            if (auto context = p.context.lock())
                            {
                                context->log("tl::qt::TimelineThumbnailObject", e.what(), log::Type::Error);
                            }
                        }
                        p.requestsInProgress.push_back(std::move(request));
                    }

                    // Check for finished requests.
                    std::vector<Private::Result> results;
                    auto requestIt = p.requestsInProgress.begin();
                    while (requestIt != p.requestsInProgress.end())
                    {
                        auto futureIt = requestIt->futures.begin();
                        while (futureIt != requestIt->futures.end())
                        {
                            if (futureIt->valid() &&
                                futureIt->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                            {
                                const auto videoData = futureIt->get();
                                const image::Info info(
                                    requestIt->size.width(),
                                    requestIt->size.height(),
                                    image::PixelType::RGBA_U8);
                                std::vector<uint8_t> pixelData(
                                    static_cast<size_t>(info.size.w) *
                                    static_cast<size_t>(info.size.h) * 4);

                                try
                                {
                                    math::Size2i offscreenBufferSize(info.size.w, info.size.h);
                                    gl::OffscreenBufferOptions offscreenBufferOptions;
                                    offscreenBufferOptions.colorType = image::PixelType::RGBA_U8;
                                    if (gl::doCreate(offscreenBuffer, offscreenBufferSize, offscreenBufferOptions))
                                    {
                                        offscreenBuffer = gl::OffscreenBuffer::create(offscreenBufferSize, offscreenBufferOptions);
                                    }
                                    gl::OffscreenBufferBinding binding(offscreenBuffer);

                                    render->begin(
                                        offscreenBufferSize,
                                        requestIt->colorConfigOptions,
                                        requestIt->lutOptions);
                                    render->drawVideo(
                                        { videoData },
                                        { math::Box2i(0, 0, info.size.w, info.size.h) });
                                    render->end();

                                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                                    glReadPixels(
                                        0,
                                        0,
                                        info.size.w,
                                        info.size.h,
                                        GL_RGBA,
                                        GL_UNSIGNED_BYTE,
                                        pixelData.data());
                                }
                                catch (const std::exception& e)
                                {
                                    context->log(
                                        "tl::qt::TimelineThumbnailObject",
                                        e.what(),
                                        log::Type::Error);
                                }

                                const auto qImage = QImage(
                                    pixelData.data(),
                                    info.size.w,
                                    info.size.h,
                                    info.size.w * 4,
                                    QImage::Format_RGBA8888).mirrored();
                                {
                                    const auto i = std::find_if(
                                        results.begin(),
                                        results.end(),
                                        [&requestIt](const Private::Result& value)
                                        {
                                            return requestIt->id == value.id;
                                        });
                                    if (i == results.end())
                                    {
                                        results.push_back({ requestIt->id, { QPair<otime::RationalTime, QImage>(videoData.time, qImage) } });
                                    }
                                    else
                                    {
                                        i->thumbnails.push_back(QPair<otime::RationalTime, QImage>(videoData.time, qImage));
                                    }
                                }

                                futureIt = requestIt->futures.erase(futureIt);
                                continue;
                            }
                            ++futureIt;
                        }
                        if (requestIt->futures.empty())
                        {
                            requestIt = p.requestsInProgress.erase(requestIt);
                            continue;
                        }
                        ++requestIt;
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.results.insert(p.results.end(), results.begin(), results.end());
                    }
                }
            }

            p.glContext->doneCurrent();
        }

        void TimelineThumbnailObject::timerEvent(QTimerEvent*)
        {
            TLRENDER_P();
            std::vector<Private::Result> results;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                results.swap(p.results);
            }
            for (const auto& i : results)
            {
                Q_EMIT thumbails(i.id, i.thumbnails);
            }
        }
    }
}
