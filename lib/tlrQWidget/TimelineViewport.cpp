// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQWidget/TimelineViewport.h>

#include <tlrGL/Render.h>

#include <QGuiApplication>
#include <QSurfaceFormat>

namespace tlr
{
    namespace qwidget
    {
        struct TimelineViewport::Private
        {
            std::weak_ptr<core::Context> context;
            gl::ColorConfig colorConfig;
            qt::TimelinePlayer* timelinePlayer = nullptr;
            timeline::Frame frame;
            std::shared_ptr<gl::Render> render;
        };

        TimelineViewport::TimelineViewport(
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();
            
            p.context = context;
            
            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            setFormat(surfaceFormat);
        }
        
        TimelineViewport::~TimelineViewport()
        {}

        void TimelineViewport::setColorConfig(const gl::ColorConfig& colorConfig)
        {
            _p->colorConfig = colorConfig;
        }

        void TimelineViewport::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
        {
            TLR_PRIVATE_P();
            p.frame = timeline::Frame();
            if (p.timelinePlayer)
            {
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(frameChanged(const tlr::timeline::Frame&)),
                    this,
                    SLOT(_frameCallback(const tlr::timeline::Frame&)));
            }
            p.timelinePlayer = timelinePlayer;
            if (p.timelinePlayer)
            {
                _p->frame = p.timelinePlayer->frame();
                connect(
                    p.timelinePlayer,
                    SIGNAL(frameChanged(const tlr::timeline::Frame&)),
                    SLOT(_frameCallback(const tlr::timeline::Frame&)));
            }
            update();
        }

        void TimelineViewport::_frameCallback(const timeline::Frame& frame)
        {
            _p->frame = frame;
            update();
        }

        void TimelineViewport::initializeGL()
        {
            TLR_PRIVATE_P();
            gladLoaderLoadGL();
            if (auto context = p.context.lock())
            {
                p.render = gl::Render::create(context);
            }
        }

        void TimelineViewport::paintGL()
        {
            TLR_PRIVATE_P();
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
                        "tlr::qwidget::TimelineViewport",
                        e.what(),
                        core::LogType::Error);
                }
            }
            const auto size = imaging::Size(
                width() * devicePixelRatio,
                height() * devicePixelRatio);
            p.render->begin(size);
            p.render->drawFrame(p.frame);
            p.render->end();
        }
    }
}
