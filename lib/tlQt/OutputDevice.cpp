// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/OutputDevice.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>

#include <tlDevice/IDeviceSystem.h>
#include <tlDevice/IOutputDevice.h>

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

            std::shared_ptr<device::IOutputDevice> device;
            imaging::ColorConfig colorConfig;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;
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
        }

        OutputDevice::~OutputDevice()
        {
            TLRENDER_P();
            p.running = false;
            wait();
        }

        void OutputDevice::setDevice(int deviceIndex, int displayModeIndex)
        {
            TLRENDER_P();
            std::shared_ptr<device::IOutputDevice> device;
            if (auto context = p.context.lock())
            {
                if (auto deviceSystem = context->getSystem<device::IDeviceSystem>())
                {
                    device = deviceSystem->createDevice(deviceIndex, displayModeIndex);
                }
            }
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.device = device;
            }
            p.cv.notify_one();
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
                auto render = gl::Render::create(context);

                std::shared_ptr<device::IOutputDevice> device;
                imaging::ColorConfig colorConfig;
                std::vector<timeline::ImageOptions> imageOptions;
                std::vector<timeline::DisplayOptions> displayOptions;
                timeline::CompareOptions compareOptions;
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
                            [this, device, colorConfig, imageOptions, displayOptions,
                                compareOptions, viewPos, viewZoom, frameView,
                                videoData]
                            {
                                return
                                    device != _p->device ||
                                    colorConfig != _p->colorConfig ||
                                    imageOptions != _p->imageOptions ||
                                    displayOptions != _p->displayOptions ||
                                    compareOptions != _p->compareOptions ||
                                    viewPos != _p->viewPos ||
                                    viewZoom != _p->viewZoom ||
                                    frameView != _p->frameView ||
                                    videoData != _p->videoData;
                            }))
                        {
                            doRender = true;
                            device = p.device;
                            colorConfig = p.colorConfig;
                            imageOptions = p.imageOptions;
                            displayOptions = p.displayOptions;
                            compareOptions = p.compareOptions;
                            viewPos = p.viewPos;
                            viewZoom = p.viewZoom;
                            frameView = p.frameView;
                            videoData = p.videoData;
                        }
                    }

                    if (doRender)
                    {
                        doRender = false;

                        if (device && device->getSize().isValid())
                        {
                            const imaging::Size size = device->getSize();
                            try
                            {
                                if (!offscreenBuffer ||
                                    (offscreenBuffer && offscreenBuffer->getSize() != size))
                                {
                                    gl::OffscreenBufferOptions options;
                                    options.colorType = imaging::PixelType::RGBA_U8;
                                    options.depth = gl::OffscreenDepth::_24;
                                    options.stencil = gl::OffscreenStencil::_8;
                                    offscreenBuffer = gl::OffscreenBuffer::create(size, options);
                                }

                                if (offscreenBuffer)
                                {
                                    gl::OffscreenBufferBinding binding(offscreenBuffer);

                                    render->setColorConfig(colorConfig);
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

                                    device->display(image);
                                }
                            }
                            catch (const std::exception& e)
                            {
                                context->log("tl::qt::OutputDevice", e.what(), log::Type::Error);
                            }
                        }
                    }
                }
            }
        }

        void OutputDevice::_frameView()
        {}
    }
}
