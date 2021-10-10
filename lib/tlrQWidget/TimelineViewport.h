// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelinePlayer.h>

#include <tlrGL/Render.h>

#include <QOpenGLWidget>

namespace tlr
{
    namespace qwidget
    {
        //! Timeline viewport widget.
        class TimelineViewport : public QOpenGLWidget
        {
            Q_OBJECT

        public:
            TimelineViewport(
                const std::shared_ptr<core::Context>&,
                QWidget* parent = nullptr);
            
            ~TimelineViewport() override;

            //! Set the color configuration.
            void setColorConfig(const gl::ColorConfig&);

            //! Set the timeline player.
            void setTimelinePlayer(qt::TimelinePlayer*);

        private Q_SLOTS:
            void _videoCallback(const tlr::timeline::VideoData&);

        protected:
            void initializeGL() override;
            void paintGL() override;

        private:
            TLR_PRIVATE();
        };
    }
}
