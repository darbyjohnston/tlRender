// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineThumbnailProvider.h>

#include <tlrGL/Render.h>

#include <tlrCore/TimelinePlayer.h>

#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QSurfaceFormat>

#include <atomic>
#include <mutex>

namespace tlr
{
    namespace qt
    {
        struct TimelineThumbnailProvider::Private
        {
            std::weak_ptr<core::Context> context;
            std::shared_ptr<timeline::Timeline> timeline;
            gl::ColorConfig colorConfig;
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
            const std::shared_ptr<tlr::timeline::Timeline>& timeline,
            const std::shared_ptr<core::Context>& context,
            QObject* parent) :
            QThread(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

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
            TLR_PRIVATE_P();
            p.running = false;
            wait();
        }

        void TimelineThumbnailProvider::setColorConfig(const gl::ColorConfig& colorConfig)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.colorConfig = colorConfig;
        }

        void TimelineThumbnailProvider::request(const otime::RationalTime& time, const QSize& size)
        {
            TLR_PRIVATE_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (p.cancelRequests)
                {
                    p.requests.clear();
                }
                Private::Request request;
                request.time = time;
                request.size = size;
                p.requests.push_back(std::move(request));
            }
            p.cv.notify_one();
        }

        void TimelineThumbnailProvider::request(const QList<otime::RationalTime>& times, const QSize& size)
        {
            TLR_PRIVATE_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (p.cancelRequests)
                {
                    p.requests.clear();
                }
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
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.cancelRequests = true;
        }

        void TimelineThumbnailProvider::setRequestCount(int value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.requestCount = value > 0 ? value : 0;
        }

        void TimelineThumbnailProvider::setRequestTimeout(int value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.requestTimeout = std::chrono::milliseconds(value > 0 ? value : 0);
        }

        void TimelineThumbnailProvider::setTimerInterval(int value)
        {
            TLR_PRIVATE_P();
            killTimer(p.timer);
            p.timer = startTimer(value);
        }

        void TimelineThumbnailProvider::run()
        {
            TLR_PRIVATE_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gladLoaderLoadGL();

            if (auto context = p.context.lock())
            {
                auto render = gl::Render::create(context);

                std::unique_ptr<QOpenGLFramebufferObject> fbo;
                imaging::Info fboInfo;

                gl::ColorConfig colorConfig;
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
                                return !_p->requests.empty() || _p->cancelRequests || !_p->requestsInProgress.empty();
                            }))
                        {
                            colorConfig = p.colorConfig;
                            if (p.cancelRequests)
                            {
                                p.cancelRequests = false;
                                p.timeline->cancelRequests();
                                p.requestsInProgress.clear();
                                p.results.clear();
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

                            const imaging::Info info(requestIt->size.width(), requestIt->size.height(), imaging::PixelType::RGBA_U8);
                            if (info != fboInfo)
                            {
                                fbo.reset(new QOpenGLFramebufferObject(info.size.w, info.size.h));
                                fboInfo = info;
                            }
                            fbo->bind();

                            try
                            {
                                render->setColorConfig(colorConfig);
                            }
                            catch (const std::exception& e)
                            {
                                context->log(
                                    "tlr::qt::TimelineThumbnailProvider",
                                    e.what(),
                                    core::LogType::Error);
                            }
                            
                            render->begin(info.size);
                            render->drawVideo(videoData);
                            render->end();
                            std::vector<uint8_t> pixels(
                                static_cast<size_t>(info.size.w) *
                                static_cast<size_t>(info.size.h) * 4);
                            glPixelStorei(GL_PACK_ALIGNMENT, 1);
                            glReadPixels(
                                0,
                                0,
                                info.size.w,
                                info.size.h,
                                GL_RGBA,
                                GL_UNSIGNED_BYTE,
                                pixels.data());
                            const auto qImage = QImage(
                                pixels.data(),
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
            TLR_PRIVATE_P();
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
