// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/TimelineThumbnailProvider.h>

#include <tlRenderGL/OffscreenBuffer.h>
#include <tlRenderGL/Render.h>

#include <tlTimeline/TimelinePlayer.h>

#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <atomic>
#include <mutex>

using namespace tl::core;

namespace tl
{
    namespace qt
    {
        struct TimelineThumbnailProvider::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<timeline::Timeline> timeline;
            imaging::ColorConfig colorConfig;
            struct Request
            {
                otime::RationalTime time = time::invalidTime;
                QSize size;

                std::future<timeline::VideoData> future;
            };
            std::list<Request> requests;
            std::list<Request> requestsInProgress;
            QList<QPair<otime::RationalTime, QImage> > results;
            bool cancelRequests = false;
            size_t requestCount = 1;
            std::chrono::milliseconds requestTimeout = std::chrono::milliseconds(100);
            int timer = 0;
            int timerInterval = 100;
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
            std::condition_variable cv;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        TimelineThumbnailProvider::TimelineThumbnailProvider(
            const std::shared_ptr<timeline::Timeline>& timeline,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            QThread(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;
            p.timeline = timeline;

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

        void TimelineThumbnailProvider::setColorConfig(const imaging::ColorConfig& colorConfig)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.colorConfig = colorConfig;
        }

        void TimelineThumbnailProvider::request(const otime::RationalTime& time, const QSize& size)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                Private::Request request;
                request.time = time;
                request.size = size;
                p.requests.push_back(std::move(request));
            }
            p.cv.notify_one();
        }

        void TimelineThumbnailProvider::request(const QList<otime::RationalTime>& times, const QSize& size)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                for (const auto& i : times)
                {
                    Private::Request request;
                    request.time = i;
                    request.size = size;
                    p.requests.push_back(std::move(request));
                }
            }
            p.cv.notify_one();
        }

        void TimelineThumbnailProvider::cancelRequests()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.timeline->cancelRequests();
            p.requests.clear();
            p.results.clear();
            p.cancelRequests = true;
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
                imaging::ColorConfig colorConfig;
                while (p.running)
                {
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
                                    _p->cancelRequests;
                            }))
                        {
                            colorConfig = p.colorConfig;
                            if (p.cancelRequests)
                            {
                                p.cancelRequests = false;
                                p.requestsInProgress.clear();
                            }
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
                        p.timeline->setActiveRanges({ otime::TimeRange(
                            p.timeline->getGlobalStartTime() + request.time,
                            otime::RationalTime(1.0, request.time.rate())) });

                        request.future = p.timeline->getVideo(request.time);

                        p.requestsInProgress.push_back(std::move(request));
                    }

                    // Check for finished requests.
                    auto requestIt = p.requestsInProgress.begin();
                    while (requestIt != p.requestsInProgress.end())
                    {
                        if (requestIt->future.valid() &&
                            requestIt->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                        {
                            const auto videoData = requestIt->future.get();
                            const imaging::Info info(
                                requestIt->size.width(),
                                requestIt->size.height(),
                                imaging::PixelType::RGBA_U8);
                            std::vector<uint8_t> pixelData(
                                static_cast<size_t>(info.size.w) *
                                static_cast<size_t>(info.size.h) * 4);

                            try
                            {
                                if (!offscreenBuffer ||
                                    (offscreenBuffer && offscreenBuffer->getSize() != info.size))
                                {
                                    gl::OffscreenBufferOptions options;
                                    options.colorType = imaging::PixelType::RGBA_U8;
                                    offscreenBuffer = gl::OffscreenBuffer::create(info.size, options);
                                }

                                render->setColorConfig(colorConfig);

                                gl::OffscreenBufferBinding binding(offscreenBuffer);

                                render->begin(info.size);
                                render->drawVideo({ videoData });
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
                                std::unique_lock<std::mutex> lock(p.mutex);
                                p.results.push_back(QPair<otime::RationalTime, QImage>(requestIt->time, qImage));
                            }

                            requestIt = p.requestsInProgress.erase(requestIt);
                            continue;
                        }
                        ++requestIt;
                    }
                }
                
                for (auto& i : p.requestsInProgress)
                {
                    i.future.get();
                }
            }

            p.glContext->doneCurrent();
        }

        void TimelineThumbnailProvider::timerEvent(QTimerEvent*)
        {
            TLRENDER_P();
            QList<QPair<otime::RationalTime, QImage> > results;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                results.swap(p.results);
            }
            if (!results.empty())
            {
                Q_EMIT thumbails(results);
            }
        }
    }
}
