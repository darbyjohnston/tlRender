// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelinePlayer.h>

#include <tlrGL/Render.h>

#include <QOpenGLWidget>

namespace tlr
{
    namespace qt
    {
        //! Timeline viewport widget.
        class TimelineViewport : public QOpenGLWidget
        {
            Q_OBJECT

        public:
            TimelineViewport(QWidget* parent = nullptr);

            //! Set the timeline player.
            void setTimelinePlayer(TimelinePlayer*);

        private Q_SLOTS:
            void _frameCallback(const tlr::io::VideoFrame&);

        protected:
            void initializeGL() override;
            void paintGL() override;

        private:
            TimelinePlayer* _timelinePlayer = nullptr;
            io::VideoFrame _frame;
            std::shared_ptr<gl::Render> _render;
        };
    }
}
