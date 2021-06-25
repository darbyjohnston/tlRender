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
            QThread(parent),
            _timeline(timeline)
        {
            _context = new QOpenGLContext;
            _context->create();

            _surface = new QOffscreenSurface;
            _surface->setFormat(_context->format());
            _surface->create();

            _context->moveToThread(this);

            _running = true;
            start();
            startTimer(thumbnailTimerInterval);
        }

        TimelineThumbnailProvider::~TimelineThumbnailProvider()
        {
            _running = false;
            wait();
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

        void TimelineThumbnailProvider::run()
        {
            _context->makeCurrent(_surface);
            gladLoadGL();

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
                            _timeline->cancelFrames();
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
                if (!requests.empty())
                {
                    const auto request = std::move(requests.front());
                    requests.pop_front();
                    const auto frame = _timeline->getFrame(request.time).get();

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

                    std::unique_lock<std::mutex> lock(_mutex);
                    _results.push_back(QPair<otime::RationalTime, QImage>(request.time, qImage));
                }
            }

            render.reset();
            _context->doneCurrent();
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
