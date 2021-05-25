// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineThumbnailProvider.h>

#include <tlrGL/Render.h>

#include <tlrCore/TimelinePlayer.h>

#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>

namespace tlr
{
    namespace qt
    {
        TimelineThumbnailProvider::TimelineThumbnailProvider(
            const std::shared_ptr<tlr::timeline::Timeline>& timeline,
            QObject* parent) :
            QObject(parent),
            _timeline(timeline)
        {
            _surface = new QOffscreenSurface;
            _surface->create();

            _running = true;
            _thread = std::thread(
                [this]
                {
                    auto context = new QOpenGLContext;
                    context->create();
                    context->makeCurrent(_surface);

                    gladLoadGL();

                    auto render = gl::Render::create();

                    QOpenGLFramebufferObject* fbo = nullptr;
                    imaging::Info fboInfo;

                    while (_running)
                    {
                        bool requestValid = false;
                        ThumbnailRequest request;
                        {
                            std::unique_lock<std::mutex> lock(_mutex);
                            if (_cv.wait_for(
                                lock,
                                thumbnailRequestTimeout,
                                [this]
                                {
                                    return !_thumbnailRequests.empty();
                                }))
                            {
                                requestValid = true;
                                request = _thumbnailRequests.front();
                                _thumbnailRequests.pop_front();
                            }
                        }
                        if (requestValid && request.frame.image)
                        {
                            imaging::Info info;
                            info.size.w = request.size.width();
                            info.size.h = request.size.height();
                            info.pixelType = request.frame.image->getPixelType();
                            if (info != fboInfo)
                            {
                                fbo = new QOpenGLFramebufferObject(info.size.w, info.size.h);
                                fboInfo = info;
                            }
                            fbo->bind();

                            render->begin(info.size);
                            render->drawImage(request.frame.image, math::BBox2f(0, 0, info.size.w, info.size.h));
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

                            const auto pixmap = QPixmap::fromImage(QImage(
                                pixels.data(),
                                info.size.w,
                                info.size.h,
                                info.size.w * 4,
                                QImage::Format_RGBA8888).mirrored());
                            std::unique_lock<std::mutex> lock(_mutex);
                            _results.push_back(QPair<otime::RationalTime, QPixmap>(request.frame.time, pixmap));
                        }
                    }

                    render.reset();
                    delete context;
                });

            startTimer(thumbnailTimerInterval);
        }

        TimelineThumbnailProvider::~TimelineThumbnailProvider()
        {
            _running = false;
            if (_thread.joinable())
            {
                _thread.join();
            }
            delete _surface;
        }

        void TimelineThumbnailProvider::request(const otime::RationalTime& time, const QSize& size)
        {
            IORequest request;
            request.time = time;
            request.size = size;
            request.future = _timeline->render(time);
            _ioRequests.push_back(std::move(request));
        }

        void TimelineThumbnailProvider::request(const QList<otime::RationalTime>& times, const QSize& size)
        {
            for (const auto& i : times)
            {
                IORequest request;
                request.time = i;
                request.size = size;
                request.future = _timeline->render(i);
                _ioRequests.push_back(std::move(request));
            }
        }

        void TimelineThumbnailProvider::cancelRequests()
        {
            _timeline->cancelRenders();
            _ioRequests.clear();
            std::unique_lock<std::mutex> lock(_mutex);
            _thumbnailRequests.clear();
            _results.clear();
        }

        void TimelineThumbnailProvider::timerEvent(QTimerEvent*)
        {
            std::list<ThumbnailRequest> thumbnailRequests;
            auto i = _ioRequests.begin();
            while (i != _ioRequests.end())
            {
                if (i->future.valid() &&
                    i->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    ThumbnailRequest request;
                    request.frame = i->future.get();
                    request.frame.time = i->time;
                    request.size = i->size;
                    thumbnailRequests.push_back(std::move(request));
                    i = _ioRequests.erase(i);
                }
                else
                {
                    ++i;
                }
            }
            if (!thumbnailRequests.empty())
            {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _thumbnailRequests.insert(_thumbnailRequests.end(), thumbnailRequests.begin(), thumbnailRequests.end());
                }
                _cv.notify_one();
            }

            QList<QPair<otime::RationalTime, QPixmap> > results;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                results.swap(_results);
            }
            if (!results.empty())
            {
                Q_EMIT thumbails(results);
            }
        }
    }
}
