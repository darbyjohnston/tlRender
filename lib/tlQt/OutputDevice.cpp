// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/OutputDevice.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>

#if defined(TLRENDER_BUILD_BMD)
#include <tlBMD/OutputDevice.h>
#endif // TLRENDER_BUILD_BMD

#include <tlCore/Context.h>

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <atomic>
#include <iostream>
#include <mutex>

namespace tl
{
    namespace qt
    {
        struct OutputDevice::Private
        {
            std::weak_ptr<system::Context> context;

            int deviceIndex = 0;
            int displayModeIndex = 0;

            imaging::ColorConfig colorConfig;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;
            imaging::Size size;
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::vector<timeline::VideoData> videoData;
            std::chrono::milliseconds timeout = std::chrono::milliseconds(5);
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
            std::condition_variable cv;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        OutputDevice::OutputDevice(
            int deviceIndex,
            int displayModeIndex,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            QThread(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            p.deviceIndex = deviceIndex;
            p.displayModeIndex = displayModeIndex;

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
        }

        OutputDevice::~OutputDevice()
        {
            TLRENDER_P();
            p.running = false;
            wait();
        }

        void OutputDevice::setColorConfig(const imaging::ColorConfig& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.colorConfig = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.imageOptions = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.displayOptions = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.compareOptions = value;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& value)
        {
            TLRENDER_P();
            if (value == p.timelinePlayers)
                return;
            for (const auto& i : p.timelinePlayers)
            {
                disconnect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    this,
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            p.timelinePlayers = value;
            for (const auto& i : p.timelinePlayers)
            {
                connect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            bool frameView = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.frameView = p.frameView;
                p.videoData.clear();
                for (const auto& i : p.timelinePlayers)
                {
                    p.videoData.push_back(i->video());
                }
            }
            if (frameView)
            {
                _frameView();
            }
        }

        void OutputDevice::setViewPosAndZoom(const tl::math::Vector2i& pos, float zoom)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.viewPos = pos;
                p.viewZoom = zoom;
            }
            p.cv.notify_one();
        }

        void OutputDevice::setViewZoom(float zoom, const tl::math::Vector2i& focus)
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.viewZoom = zoom;
            }
            p.cv.notify_one();
        }

        void OutputDevice::frameView()
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.frameView = true;
            }
            p.cv.notify_one();
        }

        void OutputDevice::_videoCallback(const tl::timeline::VideoData& value)
        {
            TLRENDER_P();
            const auto i = std::find(p.timelinePlayers.begin(), p.timelinePlayers.end(), sender());
            if (i != p.timelinePlayers.end())
            {
                const size_t index = i - p.timelinePlayers.begin();
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.videoData[index] = value;
                }
                p.cv.notify_one();
            }
        }

        void OutputDevice::run()
        {
            TLRENDER_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gladLoaderLoadGL();

            if (auto context = p.context.lock())
            {
#if defined(TLRENDER_BUILD_BMD)
                auto device = bmd::OutputDevice::create(p.deviceIndex, p.displayModeIndex, context);
                p.size = device->getSize();
#endif // TLRENDER_BUILD_BMD

                auto render = gl::Render::create(context);

                imaging::ColorConfig colorConfig;
                std::vector<timeline::ImageOptions> imageOptions;
                std::vector<timeline::DisplayOptions> displayOptions;
                timeline::CompareOptions compareOptions;
                imaging::Size size = imaging::Size(1920, 1080);
                math::Vector2i viewPos;
                float viewZoom = 1.F;
                bool frameView = true;
                std::vector<timeline::VideoData> videoData;

                bool doRender = false;
                std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;
                while (p.running)
                {
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if (p.cv.wait_for(
                            lock,
                            p.timeout,
                            [this, colorConfig, imageOptions, displayOptions,
                                compareOptions, size, viewPos, viewZoom, frameView,
                                videoData]
                            {
                                return
                                    colorConfig != _p->colorConfig ||
                                    imageOptions != _p->imageOptions ||
                                    displayOptions != _p->displayOptions ||
                                    compareOptions != _p->compareOptions ||
                                    size != _p->size ||
                                    viewPos != _p->viewPos ||
                                    viewZoom != _p->viewZoom ||
                                    frameView != _p->frameView ||
                                    videoData != _p->videoData;
                            }))
                        {
                            doRender = true;
                            colorConfig = p.colorConfig;
                            imageOptions = p.imageOptions;
                            displayOptions = p.displayOptions;
                            compareOptions = p.compareOptions;
                            size = p.size;
                            viewPos = p.viewPos;
                            viewZoom = p.viewZoom;
                            frameView = p.frameView;
                            videoData = p.videoData;
                        }
                    }

                    if (doRender)
                    {
                        doRender = false;

                        if (!offscreenBuffer ||
                            (offscreenBuffer && offscreenBuffer->getSize() != size))
                        {
                            gl::OffscreenBufferOptions options;
                            options.colorType = imaging::PixelType::RGBA_U8;
                            options.depth = gl::OffscreenDepth::_24;
                            options.stencil = gl::OffscreenStencil::_8;
                            offscreenBuffer = gl::OffscreenBuffer::create(size, options);
                        }

                        render->setColorConfig(colorConfig);

                        if (offscreenBuffer)
                        {
                            gl::OffscreenBufferBinding binding(offscreenBuffer);

                            render->begin(size);
                            render->drawVideo(
                                { videoData },
                                { math::BBox2i(0, 0, size.w, size.h) },
                                { imageOptions },
                                { displayOptions },
                                compareOptions);
                            render->end();

                            auto image = imaging::Image::create(imaging::Info(size, imaging::PixelType::RGBA_U8));

                            glPixelStorei(GL_PACK_ALIGNMENT, 1);
                            glReadPixels(
                                0,
                                0,
                                size.w,
                                size.h,
                                GL_BGRA,
                                GL_UNSIGNED_BYTE,
                                image->getData());

#if defined(TLRENDER_BUILD_BMD)
                            device->display(image);
#endif // TLRENDER_BUILD_BMD
                        }
                    }
                }
            }
        }

        void OutputDevice::_frameView()
        {}
    }
}
