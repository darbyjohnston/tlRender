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

#include <atomic>
#include <mutex>

namespace tlr
{
    namespace qt
    {
        struct TimelineThumbnailProvider::Private
        {
            std::shared_ptr<timeline::Timeline> timeline;
            gl::ColorConfig colorConfig;
            struct Request
            {
                otime::RationalTime time = time::invalidTime;
                QSize size;
            };
            std::list<Request> requests;
            QList<QPair<otime::RationalTime, QImage> > results;
            bool cancelRequests = false;
            QOffscreenSurface* surface = nullptr;
            QOpenGLContext* context = nullptr;
            std::condition_variable cv;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        TimelineThumbnailProvider::TimelineThumbnailProvider(
            const std::shared_ptr<tlr::timeline::Timeline>& timeline,
            QObject* parent) :
            QThread(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            p.timeline = timeline;

            p.context = new QOpenGLContext;
            p.context->create();

            p.surface = new QOffscreenSurface;
            p.surface->setFormat(p.context->format());
            p.surface->create();

            p.context->moveToThread(this);

            p.running = true;
            start();
            startTimer(thumbnailTimerInterval);
        }

        TimelineThumbnailProvider::~TimelineThumbnailProvider()
        {
            TLR_PRIVATE_P();
            p.running = false;
            wait();
            delete p.surface;
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

        void TimelineThumbnailProvider::run()
        {
            TLR_PRIVATE_P();

            p.context->makeCurrent(p.surface);
            gladLoadGL();

            auto render = gl::Render::create();

            QOpenGLFramebufferObject* fbo = nullptr;
            imaging::Info fboInfo;

            gl::ColorConfig colorConfig;
            std::list<Private::Request> requests;
            while (p.running)
            {
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.cv.wait_for(
                        lock,
                        thumbnailRequestTimeout,
                        [this, &requests]
                        {
                            return !_p->requests.empty() || _p->cancelRequests || !requests.empty();
                        }))
                    {
                        colorConfig = p.colorConfig;
                        if (p.cancelRequests)
                        {
                            p.cancelRequests = false;
                            p.timeline->cancelFrames();
                            requests.clear();
                            p.results.clear();
                        }
                        while (!p.requests.empty())
                        {
                            requests.push_back(std::move(p.requests.front()));
                            p.requests.pop_front();
                        }
                    }
                }
                if (!requests.empty())
                {
                    const auto request = std::move(requests.front());
                    requests.pop_front();

                    p.timeline->setActiveRanges({ otime::TimeRange(
                        p.timeline->getGlobalStartTime() + request.time,
                        otime::RationalTime(1.0, request.time.rate())) });
                    
                    const auto frame = p.timeline->getFrame(request.time).get();

                    const imaging::Info info(request.size.width(), request.size.height(), imaging::PixelType::RGBA_U8);
                    if (info != fboInfo)
                    {
                        fbo = new QOpenGLFramebufferObject(info.size.w, info.size.h);
                        fboInfo = info;
                    }
                    fbo->bind();

                    render->setColorConfig(colorConfig);
                    render->begin(info.size);
                    render->drawFrame(frame);
                    render->end();
                    std::vector<uint8_t> pixels(info.size.w * info.size.h * 4);
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

                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.results.push_back(QPair<otime::RationalTime, QImage>(request.time, qImage));
                }
            }

            render.reset();
            p.context->doneCurrent();
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
