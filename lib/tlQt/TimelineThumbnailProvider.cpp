// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/TimelineThumbnailProvider.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>

#include <tlTimeline/TimelinePlayer.h>

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
        struct TimelineThumbnailProvider::Private
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
            std::chrono::milliseconds requestTimeout = std::chrono::milliseconds(50);
            int timer = 0;
            int timerInterval = 50;
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
            std::condition_variable cv;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        TimelineThumbnailProvider::TimelineThumbnailProvider(
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

        TimelineThumbnailProvider::~TimelineThumbnailProvider()
        {
            TLRENDER_P();
            p.running = false;
            wait();
        }

        qint64 TimelineThumbnailProvider::request(
            const QString& fileName,
            const otime::RationalTime& time,
            const QSize& size,
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

        qint64 TimelineThumbnailProvider::request(
            const QString& fileName,
            const QList<otime::RationalTime>& times,
            const QSize& size,
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

        void TimelineThumbnailProvider::cancelRequests(qint64 id)
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

        void TimelineThumbnailProvider::setRequestCount(int value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.requestCount = value > 0 ? value : 0;
        }

        void TimelineThumbnailProvider::setRequestTimeout(int value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.requestTimeout = std::chrono::milliseconds(value > 0 ? value : 0);
        }

        void TimelineThumbnailProvider::setTimerInterval(int value)
        {
            TLRENDER_P();
            killTimer(p.timer);
            p.timer = startTimer(value);
        }

        void TimelineThumbnailProvider::run()
        {
            TLRENDER_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gladLoaderLoadGL();

            if (auto context = p.context.lock())
            {
                auto render = gl::Render::create(context);

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
                        options.requestTimeout = std::chrono::milliseconds(100);
                        options.ioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg(1);
                        options.ioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg(1);
                        request.timeline = timeline::Timeline::create(request.fileName.toUtf8().data(), context, options);
                        otime::TimeRange timeRange;
                        if (!request.times.empty())
                        {
                            timeRange = otime::TimeRange(request.times[0], otime::RationalTime(1.0, request.times[0].rate()));
                            for (size_t i = 1; i < request.times.size(); ++i)
                            {
                                timeRange = timeRange.extended_by(
                                    otime::TimeRange(request.times[i], otime::RationalTime(1.0, request.times[i].rate())));
                            }
                            request.timeline->setActiveRanges({ timeRange });
                        }
                        for (const auto& i : request.times)
                        {
                            request.futures.push_back(request.timeline->getVideo(i));
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
                                const imaging::Info info(
                                    requestIt->size.width(),
                                    requestIt->size.height(),
                                    imaging::PixelType::RGBA_U8);
                                std::vector<uint8_t> pixelData(
                                    static_cast<size_t>(info.size.w) *
                                    static_cast<size_t>(info.size.h) * 4);

                                try
                                {
                                    gl::OffscreenBufferOptions offscreenBufferOptions;
                                    offscreenBufferOptions.colorType = imaging::PixelType::RGBA_U8;
                                    if (gl::doCreate(offscreenBuffer, info.size, offscreenBufferOptions))
                                    {
                                        offscreenBuffer = gl::OffscreenBuffer::create(info.size, offscreenBufferOptions);
                                    }

                                    render->setColorConfig(requestIt->colorConfigOptions);
                                    render->setLUT(requestIt->lutOptions);

                                    gl::OffscreenBufferBinding binding(offscreenBuffer);

                                    render->begin(info.size);
                                    render->drawVideo(
                                        { videoData },
                                        { math::BBox2i(0, 0, info.size.w, info.size.h) });
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
                                        "tl::qt::TimelineThumbnailProvider",
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

        void TimelineThumbnailProvider::timerEvent(QTimerEvent*)
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
