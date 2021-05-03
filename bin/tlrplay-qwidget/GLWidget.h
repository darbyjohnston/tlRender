// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrRender/Image.h>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QPointer>

namespace tlr
{
    //! OpenGL window.
    class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
    {
    public:
        GLWidget(QWidget* parent = nullptr);

        //! Set the image to draw.
        void setImage(const std::shared_ptr<imaging::Image>&);

    protected:
        void initializeGL() override;
        void paintGL() override;

    private:
        std::shared_ptr<imaging::Image> _image;
        std::shared_ptr<imaging::Image> _image2;
        QPointer<QOpenGLShaderProgram> _program;
        std::unique_ptr<QOpenGLTexture> _texture;
    };
}
