// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineViewport.h>

#include <QSurfaceFormat>

namespace tlr
{
    namespace qt
    {
        struct TimelineViewport::Private
        {
            gl::ColorConfig colorConfig;
            TimelinePlayer* timelinePlayer = nullptr;
            timeline::Frame frame;
            std::shared_ptr<gl::Render> render;
        };

        TimelineViewport::TimelineViewport(QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
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

        void TimelineViewport::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            TLR_PRIVATE_P();
            p.frame = timeline::Frame();
            if (p.timelinePlayer)
            {
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(frameChanged(const tlr::timeline::Frame&)));
            }
            p.timelinePlayer = timelinePlayer;
            if (p.timelinePlayer)
            {
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
            gladLoaderLoadGL();
            _p->render = gl::Render::create();
        }

        void TimelineViewport::paintGL()
        {
            TLR_PRIVATE_P();
            const auto size = imaging::Size(width(), height());
            p.render->setColorConfig(p.colorConfig);
            p.render->begin(size);
            p.render->drawFrame(p.frame);
            p.render->end();
        }
    }
}
