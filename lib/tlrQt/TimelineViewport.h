// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimelineObject.h>

#include <tlrCore/Image.h>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>

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

            //! Set the timeline object.
            void setTimeline(TimelineObject*);

        private Q_SLOTS:
            void _frameCallback(const tlr::io::VideoFrame&);

        protected:
            void initializeGL() override;
            void paintGL() override;

        private:
            TimelineObject* _timeline = nullptr;
            io::VideoFrame _frame;
            io::VideoFrame _frameTmp;
            QOpenGLShaderProgram* _program = nullptr;
            std::unique_ptr<QOpenGLTexture> _texture;
        };
    }
}
