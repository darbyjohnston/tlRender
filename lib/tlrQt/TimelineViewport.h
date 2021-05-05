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
#include <QPointer>

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

            //! Set the timeline.
            void setTimeline(TimelineObject*);

        private Q_SLOTS:
            void _imageCallback(const std::shared_ptr<tlr::imaging::Image>&);

        protected:
            void initializeGL() override;
            void paintGL() override;

        private:
            std::shared_ptr<imaging::Image> _image;
            std::shared_ptr<imaging::Image> _imageTmp;
            QPointer<QOpenGLShaderProgram> _program;
            std::unique_ptr<QOpenGLTexture> _texture;
        };
    }
}
