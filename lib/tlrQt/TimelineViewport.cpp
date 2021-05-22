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

        void TimelineViewport::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            _frame = io::VideoFrame();
            if (_timelinePlayer)
            {
                disconnect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::io::VideoFrame&)));
            }
            _timelinePlayer = timelinePlayer;
            if (_timelinePlayer)
            {
                connect(
                    _timelinePlayer,
                    SIGNAL(frameChanged(const tlr::io::VideoFrame&)),
                    SLOT(_frameCallback(const tlr::io::VideoFrame&)));
            }
            update();
        }

        void TimelineViewport::_frameCallback(const io::VideoFrame& frame)
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
            _render->begin(imaging::Info(size, imaging::PixelType::RGBA_U8));
            if (_frame.image)
            {
                _render->drawImage(_frame.image, timeline::fitWindow(_frame.image->getSize(), size));
            }
            _render->end();
        }
    }
}
