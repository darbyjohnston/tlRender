// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelinePlayer.h>

#include <tlrCore/Image.h>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>

#include <array>

namespace tlr
{
    namespace qt
    {
        //! Timeline viewport widget.
        class TimelineViewport : public QOpenGLWidget, protected QOpenGLFunctions
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
            io::VideoFrame _frameTmp;
            QOpenGLShaderProgram* _program = nullptr;
            std::array<std::unique_ptr<QOpenGLTexture>, 3> _textures;
        };
    }
}
