// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/PlaybackDevice.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>

#if defined(TLRENDER_BUILD_BMD)
#include <tlBMD/PlaybackDevice.h>
#endif // TLRENDER_BUILD_BMD

#include <tlCore/Context.h>

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <iostream>

namespace tl
{
    namespace qt
    {
        struct PlaybackDevice::Private
        {
#if defined(TLRENDER_BUILD_BMD)
            std::shared_ptr<bmd::PlaybackDevice> device;
#endif // TLRENDER_BUILD_BMD
            imaging::ColorConfig colorConfig;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;
            imaging::Size size = imaging::Size(1920, 1080);
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
        };

        PlaybackDevice::PlaybackDevice(
            int deviceIndex,
            const std::shared_ptr<system::Context>& context) :
            _p(new Private)
        {
            TLRENDER_P();

#if defined(TLRENDER_BUILD_BMD)
            p.device = bmd::PlaybackDevice::create(deviceIndex, context);
#endif // TLRENDER_BUILD_BMD

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

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gladLoaderLoadGL();

            p.render = gl::Render::create(context);
        }

        PlaybackDevice::~PlaybackDevice()
        {}

        void PlaybackDevice::setColorConfig(const imaging::ColorConfig& value)
        {
            TLRENDER_P();
            if (value == p.colorConfig)
                return;
            p.colorConfig = value;
            _render();
        }

        void PlaybackDevice::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            _render();
        }

        void PlaybackDevice::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            _render();
        }

        void PlaybackDevice::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            _render();
        }

        void PlaybackDevice::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& value)
        {
            TLRENDER_P();
            p.videoData.clear();
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
                _p->videoData.push_back(i->video());
                connect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            if (p.frameView)
            {
                _frameView();
            }
            _render();
        }

        const math::Vector2i& PlaybackDevice::viewPos() const
        {
            return _p->viewPos;
        }

        float PlaybackDevice::viewZoom() const
        {
            return _p->viewZoom;
        }

        void PlaybackDevice::setViewPosAndZoom(const tl::math::Vector2i&, float)
        {}

        void PlaybackDevice::setViewZoom(float, const tl::math::Vector2i& focus)
        {}

        void PlaybackDevice::frameView()
        {}

        void PlaybackDevice::_videoCallback(const tl::timeline::VideoData& value)
        {
            TLRENDER_P();
            const auto i = std::find(p.timelinePlayers.begin(), p.timelinePlayers.end(), sender());
            if (i != p.timelinePlayers.end())
            {
                const size_t index = i - p.timelinePlayers.begin();
                _p->videoData[index] = value;
            }
            _render();
        }

        void PlaybackDevice::_frameView()
        {}

        void PlaybackDevice::_render()
        {
            TLRENDER_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());

            if (!p.buffer ||
                (p.buffer && p.buffer->getSize() != p.size))
            {
                gl::OffscreenBufferOptions options;
                options.colorType = imaging::PixelType::RGBA_U8;
                options.depth = gl::OffscreenDepth::_24;
                options.stencil = gl::OffscreenStencil::_8;
                p.buffer = gl::OffscreenBuffer::create(p.size, options);
            }

            p.render->setColorConfig(p.colorConfig);

            if (p.buffer)
            {
                gl::OffscreenBufferBinding binding(p.buffer);

                p.render->begin(p.size);
                p.render->drawVideo(
                    { p.videoData },
                    { math::BBox2i(0, 0, p.size.w, p.size.h) });
                p.render->end();

                auto image = imaging::Image::create(imaging::Info(p.size, imaging::PixelType::RGBA_U8));

                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadPixels(
                    0,
                    0,
                    p.size.w,
                    p.size.h,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    image->getData());

#if defined(TLRENDER_BUILD_BMD)
                p.device->display(image);
#endif // TLRENDER_BUILD_BMD
            }
        }
    }
}
