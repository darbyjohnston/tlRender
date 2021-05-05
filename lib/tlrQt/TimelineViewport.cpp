// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineViewport.h>

#include <tlrCore/BBox.h>
#include <tlrCore/Matrix.h>

#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLVertexArrayObject>

namespace tlr
{
    namespace qt
    {
        TimelineViewport::TimelineViewport(QWidget* parent) :
            QOpenGLWidget(parent)
        {}

        void TimelineViewport::setTimeline(TimelineObject* timeline)
        {
            if (timeline)
            {
                connect(
                    timeline,
                    SIGNAL(currentImageChanged(const std::shared_ptr<tlr::imaging::Image>&)),
                    SLOT(_imageCallback(const std::shared_ptr<tlr::imaging::Image>&)));
            }
            else
            {
                disconnect(
                    timeline,
                    SIGNAL(currentImageChanged(const std::shared_ptr<tlr::imaging::Image>&)));
            }
        }

        void TimelineViewport::_imageCallback(const std::shared_ptr<imaging::Image>& image)
        {
            _imageTmp = image;
            update();
        }

        void TimelineViewport::initializeGL()
        {
            initializeOpenGLFunctions();

            QOpenGLShader vertexShader(QOpenGLShader::Vertex);
            vertexShader.compileSourceCode(
                "attribute vec3 aPos;\n"
                "attribute vec2 aTexture;\n"
                "\n"
                "varying vec2 Texture;\n"
                "\n"
                "uniform mat4 mvp;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = mvp * vec4(aPos, 1.0);\n"
                "    Texture = aTexture;\n"
                "}\n");
            QOpenGLShader fragmentShader(QOpenGLShader::Fragment);
            fragmentShader.compileSourceCode(
                "varying vec2 Texture;\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 t = texture2D(textureSampler, Texture);\n"
                "    gl_FragColor = t;\n"
                "}\n");

            _program = new QOpenGLShaderProgram;
            _program->addShader(&vertexShader);
            _program->addShader(&fragmentShader);
            _program->bindAttributeLocation("aPos", 0);
            _program->bindAttributeLocation("aTexture", 1);
            _program->link();

            _texture.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
            _texture->create();
        }

        namespace
        {
            struct VBOVertex
            {
                float    vx;
                float    vy;
                uint16_t tx;
                uint16_t ty;
            };

            QOpenGLTexture::TextureFormat getTextureFormat(imaging::PixelType value)
            {
                QOpenGLTexture::TextureFormat out = QOpenGLTexture::NoFormat;
                switch (value)
                {
                case imaging::PixelType::L_U8: out = QOpenGLTexture::R8_UNorm; break;
                case imaging::PixelType::RGB_U8: out = QOpenGLTexture::RGB8_UNorm; break;
                case imaging::PixelType::RGBA_U8: out = QOpenGLTexture::RGBA8_UNorm; break;
                default: break;
                }
                return out;
            }

            QOpenGLTexture::PixelFormat getPixelFormat(imaging::PixelType value)
            {
                QOpenGLTexture::PixelFormat out = QOpenGLTexture::PixelFormat::NoSourceFormat;
                switch (value)
                {
                case imaging::PixelType::L_U8: out = QOpenGLTexture::Red; break;
                case imaging::PixelType::RGB_U8: out = QOpenGLTexture::RGB; break;
                case imaging::PixelType::RGBA_U8: out = QOpenGLTexture::RGBA; break;
                default: break;
                }
                return out;
            }

            QOpenGLTexture::PixelType getPixelType(imaging::PixelType value)
            {
                QOpenGLTexture::PixelType out = QOpenGLTexture::NoPixelType;
                switch (value)
                {
                case imaging::PixelType::L_U8: out = QOpenGLTexture::UInt8; break;
                case imaging::PixelType::RGB_U8: out = QOpenGLTexture::UInt8; break;
                case imaging::PixelType::RGBA_U8: out = QOpenGLTexture::UInt8; break;
                default: break;
                }
                return out;
            }
        }

        void TimelineViewport::paintGL()
        {
            const int w = width();
            const int h = height();
            glViewport(0, 0, w, h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            const int tw = _texture->width();
            const int th = _texture->height();
            if (_imageTmp != _image)
            {
                _image = _imageTmp;
                if (_image)
                {
                    const imaging::Info& info = _image->getInfo();
                    if (tw != info.size.w || th != info.size.h)
                    {
                        _texture->destroy();
                        _texture->setFormat(getTextureFormat(info.pixelType));
                        _texture->setSize(info.size.w, info.size.h);
                        _texture->allocateStorage();
                    }
                    _texture->bind();
                    _texture->setData(getPixelFormat(info.pixelType), getPixelType(info.pixelType), _image->getData());
                }
            }

            if (_image)
            {
                _texture->bind();

                _program->bind();
                QMatrix4x4 m;
                m.ortho(0.F, static_cast<float>(w), static_cast<float>(h), 0.F, -1.F, 1.F);
                _program->setUniformValue("mvp", m);
                _program->setUniformValue("textureSampler", 0);

                const math::BBox2f bbox = timeline::fitWindow(imaging::Size(tw, th), imaging::Size(w, h));
                VBOVertex vboData[4];
                vboData[0].vx = bbox.min.x;
                vboData[0].vy = bbox.min.y;
                vboData[0].tx = 0;
                vboData[0].ty = 0;
                vboData[1].vx = bbox.max.x;
                vboData[1].vy = bbox.min.y;
                vboData[1].tx = 65535;
                vboData[1].ty = 0;
                vboData[2].vx = bbox.min.x;
                vboData[2].vy = bbox.max.y;
                vboData[2].tx = 0;
                vboData[2].ty = 65535;
                vboData[3].vx = bbox.max.x;
                vboData[3].vy = bbox.max.y;
                vboData[3].tx = 65535;
                vboData[3].ty = 65535;
                QOpenGLBuffer vbo;
                vbo.create();
                vbo.bind();
                vbo.allocate(vboData, 4 * sizeof(VBOVertex));

                QOpenGLVertexArrayObject vao;
                vao.create();
                vao.bind();
                _program->enableAttributeArray(0);
                _program->enableAttributeArray(1);
                _program->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(VBOVertex));
                _program->setAttributeBuffer(1, GL_UNSIGNED_SHORT, 8, 2, sizeof(VBOVertex));

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
    }
}
