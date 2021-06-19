// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineViewport.h>

namespace tlr
{
    namespace qt
    {
        TimelineViewport::TimelineViewport(QWidget* parent) :
            QOpenGLWidget(parent)
        {}

        void TimelineViewport::setColorConfig(const gl::ColorConfig& colorConfig)
        {
            _colorConfig = colorConfig;
        }

        void TimelineViewport::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            _frame = timeline::Frame();
            if (_timelinePlayer)
            {
                disconnect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::timeline::Frame&)));
            }
            _timelinePlayer = timelinePlayer;
            if (_timelinePlayer)
            {
                connect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::timeline::Frame&)),
                    SLOT(_frameCallback(const tlr::timeline::Frame&)));
            }
            update();
        }

        void TimelineViewport::_frameCallback(const timeline::Frame& frame)
        {
            _frame = frame;
            update();
        }

        void TimelineViewport::initializeGL()
        {
            gladLoadGL();
            _render = gl::Render::create();
        }

        void TimelineViewport::paintGL()
        {
            const auto size = imaging::Size(width(), height());
            _render->setColorConfig(_colorConfig);
            _render->begin(size);
            _render->drawFrame(_frame);
            _render->end();
        }
    }
}
