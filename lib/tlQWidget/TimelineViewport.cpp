// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQWidget/TimelineViewport.h>

#include <tlGL/Render.h>

#include <QGuiApplication>
#include <QSurfaceFormat>

namespace tl
{
    namespace qwidget
    {
        struct TimelineViewport::Private
        {
            std::weak_ptr<core::Context> context;
            imaging::ColorConfig colorConfig;
            std::vector<render::ImageOptions> imageOptions;
            render::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<render::IRender> render;
        };

        TimelineViewport::TimelineViewport(
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();
            
            p.context = context;
            
            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            surfaceFormat.setStencilBufferSize(8);
            setFormat(surfaceFormat);
        }
        
        TimelineViewport::~TimelineViewport()
        {}

        void TimelineViewport::setColorConfig(const imaging::ColorConfig & colorConfig)
        {
            if (colorConfig == _p->colorConfig)
                return;
            _p->colorConfig = colorConfig;
            update();
        }

        void TimelineViewport::setImageOptions(const std::vector<render::ImageOptions>& options)
        {
            TLRENDER_P();
            if (options == p.imageOptions)
                return;
            p.imageOptions = options;
            update();
        }

        void TimelineViewport::setCompareOptions(const render::CompareOptions& options)
        {
            TLRENDER_P();
            if (options == p.compareOptions)
                return;
            p.compareOptions = options;
            update();
        }

        void TimelineViewport::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& timelinePlayers)
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
            p.timelinePlayers = timelinePlayers;
            for (const auto& i : p.timelinePlayers)
            {
                _p->videoData.push_back(i->video());
                connect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            update();
        }

        void TimelineViewport::_videoCallback(const timeline::VideoData& value)
        {
            TLRENDER_P();
            const auto i = std::find(p.timelinePlayers.begin(), p.timelinePlayers.end(), sender());
            if (i != p.timelinePlayers.end())
            {
                const size_t index = i - p.timelinePlayers.begin();
                _p->videoData[index] = value;
            }
            update();
        }

        void TimelineViewport::initializeGL()
        {
            TLRENDER_P();
            gladLoaderLoadGL();
            if (auto context = p.context.lock())
            {
                p.render = gl::Render::create(context);
            }
        }

        void TimelineViewport::paintGL()
        {
            TLRENDER_P();
            float devicePixelRatio = 1.F;
            if (auto app = qobject_cast<QGuiApplication*>(QGuiApplication::instance()))
            {
                devicePixelRatio = app->devicePixelRatio();
            }
            try
            {
                p.render->setColorConfig(p.colorConfig);
            }
            catch (const std::exception& e)
            {
                if (auto context = p.context.lock())
                {
                    context->log(
                        "tl::qwidget::TimelineViewport",
                        e.what(),
                        core::LogType::Error);
                }
            }
            const auto size = imaging::Size(
                width() * devicePixelRatio,
                height() * devicePixelRatio);
            p.render->begin(size);
            p.render->drawVideo(p.videoData, p.imageOptions, p.compareOptions);
            p.render->end();
        }
    }
}
