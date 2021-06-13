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

                    auto render = gl::Render::create();

                    QOpenGLFramebufferObject* fbo = nullptr;
                    imaging::Info fboInfo;

                    gl::ColorConfig colorConfig;
                    std::list<Request> requests;
                    while (_running)
                    {
                        {
                            std::unique_lock<std::mutex> lock(_mutex);
                            if (_cv.wait_for(
                                lock,
                                thumbnailRequestTimeout,
                                [this, &requests]
                                {
                                    return !_requests.empty() || _cancelRequests || !requests.empty();
                                }))
                            {
                                colorConfig = _colorConfig;
                                if (_cancelRequests)
                                {
                                    _cancelRequests = false;
                                    _timeline->cancelRenders();
                                    requests.clear();
                                    _results.clear();
                                }
                                while (!_requests.empty())
                                {
                                    requests.push_back(std::move(_requests.front()));
                                    _requests.pop_front();
                                }
                            }
                        }
                        _timeline->tick();
                        if (!requests.empty())
                        {
                            const auto request = std::move(requests.front());
                            requests.pop_front();
                            const auto frame = _timeline->render(request.time).get();
                            QImage qImage;
                            if (frame.image)
                            {
                                imaging::Info info;
                                info.size.w = request.size.width();
                                info.size.h = request.size.height();
                                info.pixelType = frame.image->getPixelType();
                                if (info != fboInfo)
                                {
                                    fbo = new QOpenGLFramebufferObject(info.size.w, info.size.h);
                                    fboInfo = info;
                                }
                                fbo->bind();

                                render->setColorConfig(colorConfig);
                                render->begin(info.size);
                                render->drawImage(frame.image, math::BBox2f(0, 0, info.size.w, info.size.h));
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

                                qImage = QImage(
                                    pixels.data(),
                                    info.size.w,
                                    info.size.h,
                                    info.size.w * 4,
                                    QImage::Format_RGBA8888).mirrored();
                            }
                            std::unique_lock<std::mutex> lock(_mutex);
                            _results.push_back(QPair<otime::RationalTime, QImage>(request.time, qImage));
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

        void TimelineThumbnailProvider::setColorConfig(const gl::ColorConfig& colorConfig)
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _colorConfig = colorConfig;
            }
        }

        void TimelineThumbnailProvider::request(const otime::RationalTime& time, const QSize& size)
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_cancelRequests)
                {
                    _requests.clear();
                }
                Request request;
                request.time = time;
                request.size = size;
                _requests.push_back(std::move(request));
            }
            _cv.notify_one();
        }

        void TimelineThumbnailProvider::request(const QList<otime::RationalTime>& times, const QSize& size)
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_cancelRequests)
                {
                    _requests.clear();
                }
                for (const auto& i : times)
                {
                    Request request;
                    request.time = i;
                    request.size = size;
                    _requests.push_back(std::move(request));
                }
            }
            _cv.notify_one();
        }

        void TimelineThumbnailProvider::cancelRequests()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cancelRequests = true;
        }

        void TimelineThumbnailProvider::timerEvent(QTimerEvent*)
        {
            QList<QPair<otime::RationalTime, QImage> > results;
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
