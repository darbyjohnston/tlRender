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
            _frame = timeline::RenderFrame();
            if (_timelinePlayer)
            {
                disconnect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::timeline::RenderFrame&)));
            }
            _timelinePlayer = timelinePlayer;
            if (_timelinePlayer)
            {
                connect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::timeline::RenderFrame&)),
                    SLOT(_frameCallback(const tlr::timeline::RenderFrame&)));
            }
            update();
        }

        void TimelineViewport::_frameCallback(const timeline::RenderFrame& frame)
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
            for (const auto& i : _frame.layers)
            {
                if (i.image)
                {
                    _render->drawImage(i.image, timeline::fitWindow(i.image->getSize(), size));
                }
            }
            _render->end();
        }
    }
}
